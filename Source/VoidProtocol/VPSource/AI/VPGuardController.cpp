// VPGuardController.cpp
#include "VPSource/AI/VPGuardController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionSystem.h"
#include "VPSource/Characters/VPCharacter.h"

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
    SightConfig->SightRadius = 800.f;
    SightConfig->LoseSightRadius = 1000.f;
    SightConfig->PeripheralVisionAngleDegrees = 60.f;
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

    UE_LOG(LogTemp, Warning, TEXT("VPGuardController: OnPossess called on %s | NetMode: %d"),
        *InPawn->GetName(),
        (int32)GetWorld()->GetNetMode());

    if (GetWorld()->GetNetMode() == NM_Client)
    {
        UE_LOG(LogTemp, Warning, TEXT("VPGuardController: Skipping — client only"));
        return;
    }

    if (!GuardBehaviorTree)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGuardController: GuardBehaviorTree is NULL — assign BT_VPGuard in BP_VPGuardController"));
        return;
    }

    RunBehaviorTree(GuardBehaviorTree);
    UE_LOG(LogTemp, Warning, TEXT("VPGuardController: BT started on %s"), *InPawn->GetName());
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

    // Only react to players — ignore other guards and actors
    if (!Cast<AVPCharacter>(Actor)) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard spotted player: %s"), *Actor->GetName());
        SetTargetActor(Actor);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Guard lost player: %s"), *Actor->GetName());
        ClearTargetActor();
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