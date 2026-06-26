// VPHackerComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VPHackerComponent.generated.h"

class IVPHackable;
class AVPSecurityCamera;
class UUserWidget;
class UVPHackProgressWidget;
class UVPCameraViewWidget;

UCLASS(ClassGroup = (VoidProtocol), meta = (BlueprintSpawnableComponent))
class VOIDPROTOCOL_API UVPHackerComponent : public UActorComponent
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — hack state
    //=========================================================
private:
    // Currently targeted hackable
    AActor* TargetHackable = nullptr;

    // Current hack progress 0-1
    float HackProgress = 0.f;

    // Are we actively hacking?
    bool bIsHacking = false;

    // Are we in camera view mode?
    bool bInCameraMode = false;

    // Current camera we're viewing through
    AVPSecurityCamera* CurrentCamera = nullptr;

    // Progress bar widget (world space)
    UPROPERTY()
    UVPHackProgressWidget* HackProgressWidget = nullptr;

    // Camera overlay widget
    UPROPERTY()
    UVPCameraViewWidget* CameraViewWidget = nullptr;

    FTimerHandle HackScanTimer;

    AActor* CurrentHackNode = nullptr;

    void ScanForHackable();
    void UpdateHackProgress(float DeltaTime);
    void CompleteHack();
    void CancelHack();
    void EnterCameraMode(AVPSecurityCamera* Camera);

    //=========================================================
    // PROTECTED — configurable in Blueprint
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Hacking")
    float HackRange = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Hacking")
    float HackDuration = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UVPHackProgressWidget> HackProgressWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UVPCameraViewWidget> CameraViewWidgetClass;

    //=========================================================
    // PUBLIC
    //=========================================================
public:
    UVPHackerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // Called by AVPHacker input bindings
    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void StartHack();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void StopHack();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void JumpToNextNode(); // Watchdogs style

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void ExitCamera();

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    float GetHackProgress() const { return HackProgress; }

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    bool IsHacking() const { return bIsHacking; }

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    bool IsInCameraMode() const { return bInCameraMode; }

    // Server RPCs
    UFUNCTION(Server, Reliable)
    void ServerStartHack(AActor* Hackable);

    UFUNCTION(Server, Reliable)
    void ServerCompleteHack(AActor* Hackable);

    UFUNCTION(Server, Reliable)
    void ServerCancelHack(AActor* Hackable);
};