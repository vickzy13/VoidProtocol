// VPCharacter.cpp — lean version
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "Net/UnrealNetwork.h"

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