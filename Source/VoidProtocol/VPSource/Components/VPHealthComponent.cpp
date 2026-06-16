// VPHealthComponent.cpp
#include "VPHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

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

void UVPHealthComponent::ServerApplyDamage_Implementation(float Amount)
{
    if (bIsDead) return;

    Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(Health);

    if (Health <= 0.f)
    {
        bIsDead = true;
        MulticastOnDeath();
    }
}

void UVPHealthComponent::OnRep_Health()
{
    OnHealthChanged.Broadcast(Health);
}

void UVPHealthComponent::MulticastOnDeath_Implementation()
{
    bIsDead = true;

    // Disable movement and input on the owning character
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        OwnerChar->GetCharacterMovement()->DisableMovement();
        OwnerChar->GetCharacterMovement()->StopMovementImmediately();
        OwnerChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
        {
            OwnerChar->DisableInput(PC);
        }
    }

    OnDeath.Broadcast();
}