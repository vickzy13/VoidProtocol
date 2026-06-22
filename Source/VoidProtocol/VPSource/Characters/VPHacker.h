#pragma once
#include "VPSource/Characters/VPPlayableCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VPHacker.generated.h"

UCLASS()
class VOIDPROTOCOL_API AVPHacker : public AVPPlayableCharacter
{
    GENERATED_BODY()

public:
    AVPHacker();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hacker")
    float HackRange = 1500.f;
};