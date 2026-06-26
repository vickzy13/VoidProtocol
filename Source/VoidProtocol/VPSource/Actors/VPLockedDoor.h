#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPSource/Interfaces/VPHackable.h"
#include "VPLockedDoor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class VOIDPROTOCOL_API AVPLockedDoor : public AActor, public IVPHackable
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* DoorFrame;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* LeftPanel;    // ← replace DoorMesh with these two

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* RightPanel;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UPointLightComponent* StatusLight;

    UPROPERTY(ReplicatedUsing = OnRep_IsUnlocked)
    bool bIsUnlocked = false;

    bool bIsOpening = false;
    FVector LeftPanelTargetLoc;
    FVector RightPanelTargetLoc;

    UFUNCTION()
    void OnRep_IsUnlocked();

    void OpenDoor();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Door")
    float OpenDistance = 150.f;

    UPROPERTY(EditDefaultsOnly, Category = "Door")
    float OpenSpeed = 3.f;

public:
    AVPLockedDoor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual bool CanBeHacked() const override { return !bIsUnlocked; }
    virtual void OnHackStarted(AActor* Hacker) override {}
    virtual void OnHackCompleted(AActor* Hacker) override;
    virtual void OnHackCancelled() override {}
    virtual FVector GetHackWidgetLocation() const override;
};