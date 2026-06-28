#pragma once
#include "VPSource/Characters/VPPlayableCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VPHacker.generated.h"

class UVPHackerComponent;

UCLASS()
class VOIDPROTOCOL_API AVPHacker : public AVPPlayableCharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UVPHackerComponent* HackerComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* HackAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpCameraAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* ExitCameraAction;

     UFUNCTION(Server, Reliable)
    void ServerRequestRevive(AVPCharacter* PlayerToRevive);

    void OnHackPressed();
    void OnHackReleased();
    void OnJumpCamera();
    void OnExitCamera();
    bool TryRevivePartner();

protected:
    virtual void SetupAbilityInputBindings(UEnhancedInputComponent* EIC) override;

public:
    AVPHacker();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hacker")
    float HackRange = 1500.f;
};