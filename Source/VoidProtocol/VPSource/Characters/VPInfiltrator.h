#pragma once
#include "VPSource/Characters/VPPlayableCharacter.h"
#include "VPSource/Components/VPInfiltratorComponent.h"
#include "VPInfiltrator.generated.h"

class UCharacterMovementComponent;
UCLASS()
class VOIDPROTOCOL_API AVPInfiltrator : public AVPPlayableCharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UVPInfiltratorComponent* InfiltratorComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* TakedownAction;

    void OnTakedownPressed();

protected:
    virtual void SetupAbilityInputBindings(UEnhancedInputComponent* EIC) override;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* TakedownMontage;

public:
    AVPInfiltrator();
    UFUNCTION(Server, Reliable)
    void ServerRequestRevive(AVPCharacter* PlayerToRevive);
};