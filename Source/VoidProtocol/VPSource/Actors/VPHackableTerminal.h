// VPHackableTerminal.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPSource/Interfaces/VPHackable.h"
#include "VPHackableTerminal.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class AVPHackableTerminal;

UCLASS()
class VOIDPROTOCOL_API AVPHackableTerminal : public AActor, public IVPHackable
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — components + state
    //=========================================================
private:
    UPROPERTY(VisibleAnywhere, Category="Components")
    UStaticMeshComponent* TerminalMesh;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UPointLightComponent* ScreenLight;

    UPROPERTY(ReplicatedUsing=OnRep_IsHacked)
    bool bIsHacked = false;

    FTimerHandle HackResetTimer;

    UFUNCTION()
    void OnRep_IsHacked();

    //=========================================================
    // PROTECTED — configurable in Blueprint
    //=========================================================
protected:
    // How long guard positions are revealed after hack
    UPROPERTY(EditDefaultsOnly, Category="Terminal")
    float HackDuration = 30.f;

    // Adjacent terminals for Watchdogs-style network jumping
    UPROPERTY(EditInstanceOnly, Category="Terminal")
    TArray<AVPHackableTerminal*> AdjacentTerminals;

    //=========================================================
    // PUBLIC — IVPHackable + overrides
    //=========================================================
public:
    AVPHackableTerminal();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IVPHackable
    virtual bool CanBeHacked() const override { return !bIsHacked; }
    virtual void OnHackStarted(AActor* Hacker) override {}
    virtual void OnHackCompleted(AActor* Hacker) override;
    virtual void OnHackCancelled() override {}
    virtual FVector GetHackWidgetLocation() const override;
    virtual TArray<AActor*> GetAdjacentHackables() const override;

    bool IsHacked() const { return bIsHacked; }
};
