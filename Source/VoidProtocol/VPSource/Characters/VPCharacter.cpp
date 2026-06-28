// VPCharacter.cpp — lean version
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "VPSource/VPGameState.h"

AVPCharacter::AVPCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    GetCharacterMovement()->SetIsReplicated(true);

    HealthComponent = CreateDefaultSubobject<UVPHealthComponent>(TEXT("HealthComponent"));
}

void AVPCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPCharacter, VPRole);
    DOREPLIFETIME_CONDITION(AVPCharacter, DetectionLevel, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AVPCharacter, ThreatLocation, COND_OwnerOnly);
    DOREPLIFETIME(AVPCharacter, bIsDowned);
}

void AVPCharacter::OnRep_Role()
{
    UE_LOG(LogTemp, Warning, TEXT("%s role set on client: %s"),
        *GetName(), *UEnum::GetValueAsString(VPRole));
}

void AVPCharacter::OnRep_DetectionLevel() {}

float AVPCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    HealthComponent->HandleTakeDamage(DamageAmount);
    return DamageAmount;
}

void AVPCharacter::SetRole_Server(EVPRole NewRole)
{
    if (!HasAuthority()) return;
    VPRole = NewRole;
    OnRep_Role();
}

void AVPCharacter::SetDowned(bool bNewDowned)
{
    if (!HasAuthority()) return;

    bIsDowned = bNewDowned;
    OnRep_bIsDowned();

    if (bNewDowned)
    {
        // Disable movement — player can still look around
        GetCharacterMovement()->DisableMovement();

        // Start 30s revive window
        GetWorldTimerManager().SetTimer(ReviveWindowTimer, [this]()
            {
                // Revive window expired
                if (AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>())
                {
                    UE_LOG(LogTemp, Warning, TEXT("%s revive window expired — checking mission fail"),
                        *GetName());
                    GS->CheckAllPlayersDowned();
                }
            }, 30.f, false);

        UE_LOG(LogTemp, Warning, TEXT("%s is DOWNED — 30s revive window"), *GetName());
    }
    else
    {
        // Revived — re-enable movement
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetWorldTimerManager().ClearTimer(ReviveWindowTimer);
    }
}

void AVPCharacter::OnRep_bIsDowned()
{
    if (bIsDowned)
    {
        GetCharacterMovement()->DisableMovement();
        UE_LOG(LogTemp, Warning, TEXT("%s DOWNED on client"), *GetName());
    }
    else
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        UE_LOG(LogTemp, Warning, TEXT("%s REVIVED on client"), *GetName());
    }
}

void AVPCharacter::ServerRevive_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("ServerRevive called on %s | bIsDowned: %d"),
        *GetName(), bIsDowned);

    if (!bIsDowned) return;

    SetDowned(false);

    if (HealthComponent)
        HealthComponent->Revive(50.f);

    UE_LOG(LogTemp, Warning, TEXT("%s successfully revived"), *GetName());
}