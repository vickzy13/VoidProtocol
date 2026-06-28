// VPPowerGenerator.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPSource/Interfaces/VPHackable.h"
#include "VPPowerGenerator.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class VOIDPROTOCOL_API AVPPowerGenerator : public AActor, public IVPHackable
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — components + state
    //=========================================================
private:
    UPROPERTY(VisibleAnywhere, Category="Components")
    UStaticMeshComponent* GeneratorMesh;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UPointLightComponent* StatusLight;

    UPROPERTY(ReplicatedUsing=OnRep_IsPowerDown)
    bool bIsPowerDown = false;

    FTimerHandle PowerRestoreTimer;

    UFUNCTION()
    void OnRep_IsPowerDown();

    void CutPower();
    void RestorePower();

    //=========================================================
    // PROTECTED — configurable in Blueprint
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category="Generator")
    float PowerDownDuration = 30.f;

    //=========================================================
    // PUBLIC — IVPHackable + overrides
    //=========================================================
public:
    AVPPowerGenerator();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IVPHackable
    virtual bool CanBeHacked() const override { return !bIsPowerDown; }
    virtual void OnHackStarted(AActor* Hacker) override {}
    virtual void OnHackCompleted(AActor* Hacker) override;
    virtual void OnHackCancelled() override {}
    virtual FVector GetHackWidgetLocation() const override;

    bool IsPowerDown() const { return bIsPowerDown; }
};
