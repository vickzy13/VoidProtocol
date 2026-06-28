// VPGameState.cpp
#include "VPSource/VPGameState.h"
#include "Net/UnrealNetwork.h"
#include "VPSource/Characters/VPCharacter.h"
#include "GameFramework/PlayerState.h"

AVPGameState::AVPGameState()
{
}

void AVPGameState::BeginPlay()
{
    Super::BeginPlay();

    // Only server initializes objectives
    if (HasAuthority())
        InitializeObjectives();
}

void AVPGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPGameState, AlertLevel);
    DOREPLIFETIME(AVPGameState, Objectives);
    DOREPLIFETIME(AVPGameState, bMissionComplete);
    DOREPLIFETIME(AVPGameState, bMissionFailed);
    DOREPLIFETIME(AVPGameState, LastKnownThreatLocation);
    DOREPLIFETIME(AVPGameState, AlertCountdown);
}

//=============================================================
// OBJECTIVES
//=============================================================

void AVPGameState::InitializeObjectives()
{
    Objectives.Empty();
    Objectives.Add(FVPObjective("steal_data", "Steal the Data Core"));
    Objectives.Add(FVPObjective("disable_reactor", "Disable the Reactor"));
    Objectives.Add(FVPObjective("reach_extraction", "Reach the Extraction Point"));

    UE_LOG(LogTemp, Warning, TEXT("VPGameState: %d objectives initialized"),
        Objectives.Num());
}

void AVPGameState::CheckAllPlayersDowned()
{
    if (!HasAuthority()) return;
    if (bMissionComplete || bMissionFailed) return;

    // Check if all players are downed
    bool bAllDowned = true;
    for (APlayerState* PS : PlayerArray)
    {
        APlayerController* PC = Cast<APlayerController>(PS->GetPlayerController());
        if (!PC) continue;

        AVPCharacter* VPChar = Cast<AVPCharacter>(PC->GetPawn());
        if (VPChar && !VPChar->IsDowned())
        {
            bAllDowned = false;
            break;
        }
    }

    if (bAllDowned)
    {
        UE_LOG(LogTemp, Warning, TEXT("All players downed — Mission Failed"));
        TriggerMissionFail();
    }
}

void AVPGameState::CompleteObjective(const FString& ObjectiveID)
{
    if (!HasAuthority()) return;

    for (FVPObjective& Obj : Objectives)
    {
        if (Obj.ID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.bCompleted = true;
            OnObjectiveCompleted.Broadcast(ObjectiveID);
            OnRep_Objectives(); // manual call on server

            UE_LOG(LogTemp, Warning, TEXT("Objective complete: %s"), *ObjectiveID);

            CheckMissionComplete();
            return;
        }
    }
}

bool AVPGameState::IsObjectiveComplete(const FString& ObjectiveID) const
{
    for (const FVPObjective& Obj : Objectives)
    {
        if (Obj.ID == ObjectiveID)
            return Obj.bCompleted;
    }
    return false;
}

void AVPGameState::CheckMissionComplete()
{
    if (bMissionComplete || bMissionFailed) return;

    for (const FVPObjective& Obj : Objectives)
    {
        if (!Obj.bCompleted) return; // not all done yet
    }

    // All objectives complete
    bMissionComplete = true;
    OnRep_bMissionComplete();

    UE_LOG(LogTemp, Warning, TEXT("MISSION COMPLETE!"));
}

//=============================================================
// ALERT SYSTEM
//=============================================================

void AVPGameState::SetAlertLevel(EVPAlertLevel NewLevel)
{
    if (!HasAuthority()) return;
    if (AlertLevel == NewLevel) return;

    AlertLevel = NewLevel;
    OnRep_AlertLevel(); // manual call on server

    // Clear all existing timers first
    GetWorldTimerManager().ClearTimer(AlertDecayTimer);
    GetWorldTimerManager().ClearTimer(MissionFailTimer);
    GetWorldTimerManager().ClearTimer(CountdownTickTimer);
    AlertCountdown = 0.f;

    UE_LOG(LogTemp, Warning, TEXT("VPGameState: Alert → %s"),
        *UEnum::GetValueAsString(NewLevel));

    switch (NewLevel)
    {
    case EVPAlertLevel::Unaware:
        // Nothing extra — timers already cleared above
        break;

    case EVPAlertLevel::Suspicious:
        // Decay back to Unaware after 10s if no further detections
        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Unaware);
            }, 10.f, false);
        break;

    case EVPAlertLevel::Alerted:
        // Start countdown
        AlertCountdown = 120.f;
        GetWorldTimerManager().SetTimer(CountdownTickTimer, this,
            &AVPGameState::TickAlertCountdown, 1.f, true);

        // Mission fails after 120s
        GetWorldTimerManager().SetTimer(MissionFailTimer, [this]()
            {
                if (!bMissionComplete)
                    TriggerMissionFail();
            }, 120.f, false);

        // Decay to Suspicious after 30s
        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Suspicious);
            }, 30.f, false);
        break;
    }
}

void AVPGameState::SetLastKnownThreatLocation(FVector Location)
{
    if (!HasAuthority()) return;
    LastKnownThreatLocation = Location;
}

//=============================================================
// REP NOTIFIES
//=============================================================

void AVPGameState::OnRep_AlertLevel()
{
    OnAlertLevelChanged.Broadcast(AlertLevel);
}

void AVPGameState::OnRep_Objectives()
{
    // Find which objective was just completed and broadcast
    for (const FVPObjective& Obj : Objectives)
    {
        if (Obj.bCompleted)
            OnObjectiveCompleted.Broadcast(Obj.ID);
    }
}

void AVPGameState::OnRep_bMissionComplete()
{
    OnMissionComplete.Broadcast();
}

void AVPGameState::OnRep_bMissionFailed()
{
    OnMissionFailed.Broadcast();
}

void AVPGameState::TriggerMissionFail()
{
    if (!HasAuthority()) return;
    if (bMissionFailed || bMissionComplete) return;

    bMissionFailed = true;
    OnRep_bMissionFailed();

    // Clear all timers
    GetWorldTimerManager().ClearTimer(AlertDecayTimer);
    GetWorldTimerManager().ClearTimer(MissionFailTimer);
    GetWorldTimerManager().ClearTimer(CountdownTickTimer);

    AlertCountdown = 0.f;

    UE_LOG(LogTemp, Warning, TEXT("MISSION FAILED"));
}

void AVPGameState::TickAlertCountdown()
{
    AlertCountdown = FMath::Max(0.f, AlertCountdown - 1.f);
}