// VPHealthComponent.cpp
#include "VPHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "VPSource/Characters/VPCharacter.h"

UVPHealthComponent::UVPHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UVPHealthComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UVPHealthComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UVPHealthComponent, Health);
}

void UVPHealthComponent::HandleTakeDamage(float Amount)
{
    if (GetOwner()->HasAuthority())
        ServerApplyDamage_Implementation(Amount);
    else
        ServerApplyDamage(Amount);
}

void UVPHealthComponent::Revive(float ReviveHealth)
{
    if (!GetOwner()->HasAuthority()) return;

    bIsDead = false;
    Health = FMath::Clamp(ReviveHealth, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health);

    UE_LOG(LogTemp, Warning, TEXT("Health revived to %.0f"), Health);
}

void UVPHealthComponent::ServerApplyDamage_Implementation(float DamageAmount)
{
    if (bIsDead) return;

    Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health);

    UE_LOG(LogTemp, Warning, TEXT("Health: %.0f"), Health);

    if (Health <= 0.f)
    {
        bIsDead = true;
        MulticastOnDeath();

        // Set downed on server — replicates to all clients via OnRep_bIsDowned
        if (AVPCharacter* VPChar = Cast<AVPCharacter>(GetOwner()))
            VPChar->SetDowned(true); // ← server sets it, replication handles clients
    }
}

void UVPHealthComponent::OnRep_Health()
{
    OnHealthChanged.Broadcast(Health);
}

void UVPHealthComponent::MulticastOnDeath_Implementation()
{
    bIsDead = true;

    // Disable movement only — keep input so player can look around
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
        OwnerChar->GetCharacterMovement()->DisableMovement();

    OnDeath.Broadcast();
}