// VPGuardController.cpp
#include "VPSource/AI/VPGuardController.h"
#include "VPSource/VPGameMode.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Characters/VPGuard.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

// Blackboard key definitions — must match BB_VPGuard asset exactly
const FName AVPGuardController::BBKey_TargetActor           = FName("TargetActor");
const FName AVPGuardController::BBKey_PatrolIndex           = FName("PatrolIndex");
const FName AVPGuardController::BBKey_AlertState            = FName("AlertState");
const FName AVPGuardController::BBKey_TargetLocation        = FName("TargetLocation");
const FName AVPGuardController::BBKey_HasLastKnownLocation  = FName("HasLastKnownLocation");
const FName AVPGuardController::BBKey_PatrolPoint           = FName("PatrolPoint");

//=============================================================
// CONSTRUCTOR
//=============================================================

AVPGuardController::AVPGuardController()
{
    // Perception component
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    SetPerceptionComponent(*PerceptionComp);

    // Sight config
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                          = 1200.f;
    SightConfig->LoseSightRadius                      = 1400.f;
    SightConfig->PeripheralVisionAngleDegrees         = 90.f;
    SightConfig->SetMaxAge(5.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    // Hearing config
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange                           = 800.f;
    HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComp->ConfigureSense(*HearingConfig);

    // Bind perception callback
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &AVPGuardController::OnPerceptionUpdated);
}

//=============================================================
// POSSESSION
//=============================================================

void AVPGuardController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // AI only runs on server
    if (GetWorld()->GetNetMode() == NM_Client) return;

    // Read patrol radius from the guard pawn
    if (AVPGuard* Guard = Cast<AVPGuard>(InPawn))
        PatrolRadius = Guard->PatrolRadius;

    if (!GuardBehaviorTree)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGuardController: No BehaviorTree assigned on %s!"),
            *InPawn->GetName());
        return;
    }

    // Store spawn location as patrol center
    SpawnLocation = InPawn->GetActorLocation();

    RunBehaviorTree(GuardBehaviorTree);

    // Pick first patrol point immediately
    PickNextPatrolPoint();

    // Start gradual detection ticking every 0.1s
    GetWorldTimerManager().SetTimer(DetectionTickTimer, this,
        &AVPGuardController::TickDetection, 0.1f, true);

   /* UE_LOG(LogTemp, Warning, TEXT("VPGuardController: BT started on %s | PatrolRadius: %.0f"),
        *InPawn->GetName(), PatrolRadius);*/
}

void AVPGuardController::OnUnPossess()
{
    Super::OnUnPossess();
    if (PerceptionComp)
        PerceptionComp->OnTargetPerceptionUpdated.RemoveAll(this);
}

//=============================================================
// RANDOM NAVMESH PATROL
//=============================================================

void AVPGuardController::PickNextPatrolPoint()
{
    if (GetWorld()->GetNetMode() == NM_Client) return;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys || !GetPawn()) return;

    FNavLocation RandomPoint;

    // Try to find a random reachable point within PatrolRadius of spawn
    bool bFound = NavSys->GetRandomReachablePointInRadius(
        SpawnLocation,
        PatrolRadius,
        RandomPoint);   

    if (bFound)
    {
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(BBKey_PatrolPoint, RandomPoint.Location);
            UE_LOG(LogTemp, Verbose, TEXT("Guard %s patrol → (%.0f, %.0f, %.0f)"),
                *GetPawn()->GetName(),
                RandomPoint.Location.X,
                RandomPoint.Location.Y,
                RandomPoint.Location.Z);
        }
    }
    else
    {
        // Fallback — use spawn location if no random point found
        if (UBlackboardComponent* BB = GetBlackboardComponent())
            BB->SetValueAsVector(BBKey_PatrolPoint, SpawnLocation);

        UE_LOG(LogTemp, Warning, TEXT("VPGuardController: No random patrol point found — using spawn"));
    }
}

//=============================================================
// PERCEPTION
//=============================================================

void AVPGuardController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    // Only react to VPCharacter subclasses (players) — ignore other guards
    if (!Cast<AVPCharacter>(Actor)) return;

    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    {
        if (Stimulus.WasSuccessfullySensed())
            VisibleActors.AddUnique(Actor);
        else
            VisibleActors.Remove(Actor);
    }
    else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
    {
        // Only investigate noise if not already chasing
        if (CurrentTarget) return;

        UE_LOG(LogTemp, Warning, TEXT("Guard HEARD noise from: %s"), *Actor->GetName());

        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsVector(BBKey_TargetLocation, Stimulus.StimulusLocation);
            BB->SetValueAsBool(BBKey_HasLastKnownLocation, true);
        }
    }
}

//=============================================================
// DETECTION TICK — Hitman-style gradual detection
//=============================================================

void AVPGuardController::TickDetection()
{
    if (GetWorld()->GetNetMode() == NM_Client) return;

    AVPGuard* Guard = Cast<AVPGuard>(GetPawn());
    if (Guard && Guard->IsUnconscious()) return;

    //─── Decay meters for actors no longer visible ──────────
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

    //─── Fill meters for currently visible actors ───────────
    for (AActor* Visible : VisibleActors)
    {
        float& Meter = DetectionMeters.FindOrAdd(Visible);

        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Visible->GetActorLocation());
        float DistAlpha = FMath::Clamp(
            (Dist - NearDistance) / (FarDistance - NearDistance), 0.f, 1.f);
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

            VPChar->SetDetectionLevel(Meter);
            if (Meter > 0.f && GetPawn())
                VPChar->SetThreatLocation(GetPawn()->GetActorLocation());
        }

        // Guarantee fill always beats decay while visible
        FillRate = FMath::Max(FillRate, DetectionDecayRate + 5.f);
        Meter = FMath::Clamp(Meter + FillRate * 0.1f, 0.f, 100.f);
    }

    if (CurrentTarget)
    {
        float& ChaseTargetMeter = DetectionMeters.FindOrAdd(CurrentTarget);
        ChaseTargetMeter = 100.f;
    }

    //─── Find highest detected target ───────────────────────
    AActor* BestTarget = nullptr;
    float BestMeter = 0.f;
    for (auto& Pair : DetectionMeters)
    {
        if (Pair.Value > BestMeter)
        {
            BestTarget = Pair.Key;
            BestMeter = Pair.Value;
        }
    }

    //─── Report to GameMode ──────────────────────────────────
    if (AVPGameMode* GM = Cast<AVPGameMode>(GetWorld()->GetAuthGameMode()))
    {
        if (CurrentTarget)
            GM->ReportAlerted();
        else if (BestMeter >= SuspiciousThreshold)
            GM->ReportSuspicious();
    }

    //─── Commit to one target — don't switch while chasing ──
    if (!CurrentTarget)
    {
        // Only pick new target if not already chasing
        if (BestMeter >= AlertedThreshold && BestTarget)
        {
            UE_LOG(LogTemp, Warning, TEXT("Guard ALERTED: chasing %s"),
                *BestTarget->GetName());
            SetTargetActor(BestTarget);
        }
    }
    else
    {
        // Already chasing — only lose target if meter fully decayed
        float* CurrentMeter = DetectionMeters.Find(CurrentTarget);
        if (!CurrentMeter || *CurrentMeter <= 0.f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Guard lost target — investigating"));
            ClearTargetActor();
        }
        // Don't switch to another target while chasing current one
    }
}

//=============================================================
// TARGET MANAGEMENT
//=============================================================

void AVPGuardController::SetTargetActor(AActor* Target)
{
    if (!Target) return;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        CurrentTarget = Target;
        BB->SetValueAsObject(BBKey_TargetActor, Target);
        BB->SetValueAsVector(BBKey_TargetLocation, Target->GetActorLocation());
        BB->SetValueAsBool(BBKey_HasLastKnownLocation, false);

        // Update target location every 0.1s while chasing
        GetWorldTimerManager().SetTimer(TargetUpdateTimer, this,
            &AVPGuardController::UpdateTargetLocation, 0.1f, true);
    }
}

void AVPGuardController::ClearTargetActor()
{
    // Don't clear if target is still visible
    if (CurrentTarget && VisibleActors.Contains(CurrentTarget))
    {
        UE_LOG(LogTemp, Warning, TEXT("ClearTargetActor blocked — target still visible"));
        return;
    }

    GetWorldTimerManager().ClearTimer(TargetUpdateTimer);

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        CurrentTarget = nullptr;
        BB->ClearValue(BBKey_TargetActor);
        BB->SetValueAsBool(BBKey_HasLastKnownLocation, true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Guard lost target — investigating last position"));
}

void AVPGuardController::ClearAllDetection()
{
    // Push 0 to all tracked players BEFORE clearing the map
    for (auto& Pair : DetectionMeters)
    {
        if (AVPCharacter* VPChar = Cast<AVPCharacter>(Pair.Key))
        {
            VPChar->SetDetectionLevel(0.f);
            VPChar->SetThreatLocation(FVector::ZeroVector);
        }
    }

    // Clear all detection state
    DetectionMeters.Empty();
    VisibleActors.Empty();
    CurrentTarget = nullptr;
    GetWorldTimerManager().ClearTimer(TargetUpdateTimer);

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->ClearValue(BBKey_TargetActor);
        BB->ClearValue(BBKey_HasLastKnownLocation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Guard detection fully cleared (unconscious)"));
}

void AVPGuardController::UpdateTargetLocation()
{
    if (!CurrentTarget) return;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
        BB->SetValueAsVector(BBKey_TargetLocation, CurrentTarget->GetActorLocation());
}
