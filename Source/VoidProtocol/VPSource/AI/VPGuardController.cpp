// VPGuardController.cpp
#include "VPSource/AI/VPGuardController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionSystem.h"
#include "VPSource/Characters/VPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

// Blackboard key names — must match exactly what you create in the BB asset
// VPGuardController.cpp — must have ALL of these:
const FName AVPGuardController::BBKey_TargetActor = FName("TargetActor");
const FName AVPGuardController::BBKey_PatrolIndex = FName("PatrolIndex");
const FName AVPGuardController::BBKey_AlertState = FName("AlertState");
const FName AVPGuardController::BBKey_TargetLocation = FName("TargetLocation");
const FName AVPGuardController::BBKey_HasLastKnownLocation = FName("HasLastKnownLocation");

AVPGuardController::AVPGuardController()
{
    // Create perception component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("PerceptionComponent"));
    SetPerceptionComponent(*PerceptionComponent);

    // Configure sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1200.f;
    SightConfig->LoseSightRadius = 1400.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;
    SightConfig->SetMaxAge(5.f);           // forget after 5 seconds
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception callback
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
        this, &AVPGuardController::OnPerceptionUpdated);
}

void AVPGuardController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (GetWorld()->GetNetMode() == NM_Client) return;

    if (!GuardBehaviorTree)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGuardController: No BehaviorTree assigned!"));
        return;
    }

    RunBehaviorTree(GuardBehaviorTree);

    // Start gradual detection ticking
    GetWorldTimerManager().SetTimer(DetectionTickTimer, this,
        &AVPGuardController::TickDetection, 0.1f, true);

    UE_LOG(LogTemp, Warning, TEXT("VPGuardController: BT started on %s"),
        *InPawn->GetName());
}

void AVPGuardController::OnUnPossess()
{
    Super::OnUnPossess();
    if (PerceptionComponent)
        PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
}

void AVPGuardController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    if (!Cast<AVPCharacter>(Actor)) return;

    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            VisibleActors.AddUnique(Actor);
        }
        else
        {
            VisibleActors.Remove(Actor);
        }
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        if (CurrentTarget) return; // ignore noise while already chasing

        UE_LOG(LogTemp, Warning, TEXT("Guard HEARD noise near: %s"), *Actor->GetName());

        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(BBKey_TargetLocation, Stimulus.StimulusLocation);
            BB->SetValueAsBool(BBKey_HasLastKnownLocation, true);
        }
    }
}

void AVPGuardController::TickDetection()
{
    if (GetWorld()->GetNetMode() == NM_Client) return;

    TArray<AActor*> Keys;
    DetectionMeters.GetKeys(Keys);
    for (AActor* Key : Keys)
    {
        if (!VisibleActors.Contains(Key))
        {
            float& Meter = DetectionMeters[Key];
            Meter = FMath::Clamp(Meter - DetectionDecayRate * 0.1f, 0.f, 100.f);
        }
    }

    for (AActor* Visible : VisibleActors)
    {
        float& Meter = DetectionMeters.FindOrAdd(Visible);

        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Visible->GetActorLocation());
        float DistAlpha = FMath::Clamp((Dist - NearDistance) / (FarDistance - NearDistance), 0.f, 1.f);
        float FillRate = FMath::Lerp(DetectionFillRateNear, DetectionFillRateFar, DistAlpha);

        FVector ToTarget = (Visible->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        float Dot = FVector::DotProduct(GetPawn()->GetActorForwardVector(), ToTarget);
        FillRate *= FMath::Lerp(0.5f, 1.5f, FMath::Clamp(Dot, 0.f, 1.f));

        if (AVPCharacter* VPChar = Cast<AVPCharacter>(Visible))
        {
            if (VPChar->GetCharacterMovement()->IsCrouching())
                FillRate *= 0.4f;
            else if (VPChar->GetVelocity().Size() > 400.f)
                FillRate *= 1.5f;
        }

        Meter = FMath::Clamp(Meter + FillRate * 0.1f, 0.f, 100.f);
    }

    AActor* BestTarget = nullptr;
    float BestMeter = 0.f;
    for (auto& Pair : DetectionMeters)
    {
        if (Pair.Value >= AlertedThreshold && Pair.Value > BestMeter)
        {
            BestTarget = Pair.Key;
            BestMeter = Pair.Value;
        }
    }

    if (BestTarget && CurrentTarget != BestTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard ALERTED: %s"), *BestTarget->GetName());
        SetTargetActor(BestTarget);
    }

    if (CurrentTarget)
    {
        float* CurrentMeter = DetectionMeters.Find(CurrentTarget);
        if (!CurrentMeter || *CurrentMeter <= 0.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Guard LOST target due to decay"));
            ClearTargetActor();
        }
    }

    for (auto& Pair : DetectionMeters)
    {
        if (AVPCharacter* VPChar = Cast<AVPCharacter>(Pair.Key))
        {
            VPChar->SetDetectionLevel(Pair.Value);
            if (Pair.Value > 0.f && GetPawn())
                VPChar->SetThreatLocation(GetPawn()->GetActorLocation());
        }
    }
}

void AVPGuardController::SetTargetActor(AActor* Target)
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        CurrentTarget = Target;
        BB->SetValueAsObject(BBKey_TargetActor, Target);
        BB->SetValueAsVector(BBKey_TargetLocation, Target->GetActorLocation());
        BB->SetValueAsBool(BBKey_HasLastKnownLocation, false);

        GetWorldTimerManager().SetTimer(TargetUpdateTimer, this,
            &AVPGuardController::UpdateTargetLocation, 0.5f, true);
    }
}

void AVPGuardController::ClearTargetActor()
{
    GetWorldTimerManager().ClearTimer(TargetUpdateTimer);

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        CurrentTarget = nullptr;
        BB->ClearValue(BBKey_TargetActor);

        // Set investigate flag — keeps TargetLocation as last known position
        BB->SetValueAsBool(BBKey_HasLastKnownLocation, true);

        UE_LOG(LogTemp, Warning, TEXT("Guard lost target — investigating last position"));
    }
}

void AVPGuardController::UpdateTargetLocation()
{
    if (!CurrentTarget) return;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
        BB->SetValueAsVector(BBKey_TargetLocation, CurrentTarget->GetActorLocation());
}