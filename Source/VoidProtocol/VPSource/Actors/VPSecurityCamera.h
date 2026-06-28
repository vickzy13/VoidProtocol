#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPSource/Interfaces/VPHackable.h"
#include "VPSecurityCamera.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class USpotLightComponent;
class URotatingMovementComponent;

UCLASS()
class VOIDPROTOCOL_API AVPSecurityCamera : public AActor, public IVPHackable
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — components
    //=========================================================
private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* CameraMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UCameraComponent* CameraView;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USpotLightComponent* CameraLight;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    URotatingMovementComponent* RotatingMovement;

    // Replicated disabled state
    UPROPERTY(ReplicatedUsing = OnRep_IsDisabled)
    bool bIsDisabled = false;

    FTimerHandle ReEnableTimer;

    UFUNCTION()
    void OnRep_IsDisabled();

    void UpdateVisuals();

    //=========================================================
    // PROTECTED — configurable in Blueprint
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DisableDuration = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float PatrolRotationRate = 30.f; // degrees per second

    UPROPERTY(EditInstanceOnly, Category = "Camera")
    TArray<AVPSecurityCamera*> AdjacentCameras;

    // Materials
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    UMaterialInterface* ActiveMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    UMaterialInterface* DisabledMaterial;

    //=========================================================
    // PUBLIC — IVPHackable interface + accessors
    //=========================================================
public:
    AVPSecurityCamera();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IVPHackable
    virtual bool CanBeHacked() const override { return !bIsDisabled; }
    virtual void OnHackStarted(AActor* Hacker) override;
    virtual void OnHackCompleted(AActor* Hacker) override;
    virtual void OnHackCancelled() override;
    virtual FVector GetHackWidgetLocation() const override;
    virtual TArray<AActor*> GetAdjacentHackables() const override;

    // For Hacker to possess this camera's view
    UFUNCTION(BlueprintCallable, Category = "Camera")
    UCameraComponent* GetCameraView() const { return CameraView; }

    bool IsDisabled() const { return bIsDisabled; }
};