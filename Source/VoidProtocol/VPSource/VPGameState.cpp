// VPGameState.cpp
#include "VPSource/VPGameState.h"
#include "Net/UnrealNetwork.h"

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
    OnRep_AlertLevel();

    GetWorldTimerManager().ClearTimer(AlertDecayTimer);
    GetWorldTimerManager().ClearTimer(MissionFailTimer);

    if (NewLevel == EVPAlertLevel::Suspicious)
    {
        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Unaware);
            }, 10.f, false);
    }
    else if (NewLevel == EVPAlertLevel::Alerted)
    {
        // Mission fails if Alerted for 120 seconds
        GetWorldTimerManager().SetTimer(MissionFailTimer, [this]()
            {
                if (!bMissionComplete)
                {
                    bMissionFailed = true;
                    OnRep_bMissionFailed();
                    UE_LOG(LogTemp, Warning, TEXT("MISSION FAILED — stayed alerted too long"));
                }
            }, 120.f, false);

        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Suspicious);
            }, 30.f, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("VPGameState: Alert → %s"),
        *UEnum::GetValueAsString(NewLevel));
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