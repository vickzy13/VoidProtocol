#include "VPSource/Characters/VPInfiltrator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "EnhancedInputComponent.h"

AVPInfiltrator::AVPInfiltrator()
{
    // Infiltrator stats — fast, tanky, loud
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    InfiltratorComponent = CreateDefaultSubobject<UVPInfiltratorComponent>(
        TEXT("InfiltratorComponent"));
    // Role assigned here as default — GameMode confirms it
    VPRole = EVPRole::Infiltrator;
}

void AVPInfiltrator::SetupAbilityInputBindings(UEnhancedInputComponent* EIC)
{
    if (TakedownAction)
        EIC->BindAction(TakedownAction, ETriggerEvent::Started,
            this, &AVPInfiltrator::OnTakedownPressed);
}

void AVPInfiltrator::OnTakedownPressed()
{
    if (!InfiltratorComponent) return;

    // Play takedown montage on Infiltrator
    if (TakedownMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
            AnimInstance->Montage_Play(TakedownMontage, 1.0f);
    }

    InfiltratorComponent->TryTakedown();
}

void AVPInfiltrator::ServerRequestRevive_Implementation(AVPCharacter* PlayerToRevive)
{
    if (!PlayerToRevive || !PlayerToRevive->IsDowned()) return;

    PlayerToRevive->SetDowned(false);

    if (UVPHealthComponent* HC = PlayerToRevive->GetHealthComponent())
        HC->Revive(50.f);

    UE_LOG(LogTemp, Warning, TEXT("Infiltrator revived: %s"), *PlayerToRevive->GetName());
}