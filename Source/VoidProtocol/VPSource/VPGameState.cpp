// VPGameState.cpp
#include "VPSource/VPGameState.h"
#include "Net/UnrealNetwork.h"

AVPGameState::AVPGameState()
{
}

void AVPGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPGameState, AlertLevel);
}

void AVPGameState::SetAlertLevel(EVPAlertLevel NewLevel)
{
    if (!HasAuthority()) return;
    if (AlertLevel == NewLevel) return; // no change — don't spam

    AlertLevel = NewLevel;
    OnRep_AlertLevel(); // manually call on server since RepNotify only fires on clients

    UE_LOG(LogTemp, Warning, TEXT("VPGameState: Alert level → %s"),
        *UEnum::GetValueAsString(NewLevel));

    // Clear existing decay timer
    GetWorldTimerManager().ClearTimer(AlertDecayTimer);

    // Auto decay back to Unaware after timeout
    if (NewLevel == EVPAlertLevel::Suspicious)
    {
        // Suspicious decays to Unaware after 10 seconds if no further detections
        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Unaware);
            }, 10.f, false);
    }
    else if (NewLevel == EVPAlertLevel::Alerted)
    {
        // Alerted decays to Suspicious after 30 seconds
        GetWorldTimerManager().SetTimer(AlertDecayTimer, [this]()
            {
                SetAlertLevel(EVPAlertLevel::Suspicious);
            }, 30.f, false);
    }
}

void AVPGameState::OnRep_AlertLevel()
{
    // Broadcast to all listeners — HUD widget binds here
    OnAlertLevelChanged.Broadcast(AlertLevel);
}