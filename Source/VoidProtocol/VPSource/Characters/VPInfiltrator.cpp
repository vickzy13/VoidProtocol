#include "VPSource/Characters/VPInfiltrator.h"
#include "GameFramework/CharacterMovementComponent.h"

AVPInfiltrator::AVPInfiltrator()
{
    // Infiltrator stats — fast, tanky, loud
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 200.f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    // Role assigned here as default — GameMode confirms it
    VPRole = EVPRole::Infiltrator;
}

