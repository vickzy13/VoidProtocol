// VPStealthComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VPStealthComponent.generated.h"

UCLASS(ClassGroup = (VoidProtocol), meta = (BlueprintSpawnableComponent))
class VOIDPROTOCOL_API UVPStealthComponent : public UActorComponent
{
    GENERATED_BODY()

    //=========================================================
    // PROTECTED — tunable per character in Blueprint
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Stealth")
    float WalkNoiseRadius = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Stealth")
    float CrouchNoiseRadius = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Stealth")
    float SprintNoiseRadius = 700.f;

    //=========================================================
    // PUBLIC
    //=========================================================
public:
    UVPStealthComponent();

    // Called from animation notify on footstep
    UFUNCTION(BlueprintCallable, Category = "Stealth")
    void OnFootstep(bool bIsCrouching, bool bIsSprinting);
};