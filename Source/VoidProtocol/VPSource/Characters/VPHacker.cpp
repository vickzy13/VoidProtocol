#include "VPSource/Characters/VPHacker.h"
#include "VPSource/Components/VPHackerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"

AVPHacker::AVPHacker()
{
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    VPRole = EVPRole::Hacker;

    HackerComponent = CreateDefaultSubobject<UVPHackerComponent>(TEXT("HackerComponent"));
}

void AVPHacker::SetupAbilityInputBindings(UEnhancedInputComponent* EIC)
{
    if (HackAction)
    {
        EIC->BindAction(HackAction, ETriggerEvent::Started,
            this, &AVPHacker::OnHackPressed);
        EIC->BindAction(HackAction, ETriggerEvent::Completed,
            this, &AVPHacker::OnHackReleased);
    }
    if (JumpCameraAction)
        EIC->BindAction(JumpCameraAction, ETriggerEvent::Started,
            this, &AVPHacker::OnJumpCamera);
    if (ExitCameraAction)
        EIC->BindAction(ExitCameraAction, ETriggerEvent::Started,
            this, &AVPHacker::OnExitCamera);
}

void AVPHacker::OnHackPressed() { HackerComponent->StartHack(); }
void AVPHacker::OnHackReleased() { HackerComponent->StopHack(); }
void AVPHacker::OnJumpCamera() 
{
    UE_LOG(LogTemp, Warning, TEXT("OnJumpCamera pressed"));
    HackerComponent->JumpToNextNode(); 
}
void AVPHacker::OnExitCamera() { HackerComponent->ExitCamera(); }