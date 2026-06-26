#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VPInfiltratorComponent.generated.h"

class AVPGuard;

UCLASS(ClassGroup = (VoidProtocol), meta = (BlueprintSpawnableComponent))
class VOIDPROTOCOL_API UVPInfiltratorComponent : public UActorComponent
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — state
    //=========================================================
private:
    // Guard being carried
    UPROPERTY(Replicated)
    AVPGuard* CarriedGuard = nullptr;

    bool bIsCarrying = false;

    FTimerHandle TakedownScanTimer;

    // Scans for guards behind player
    void ScanForTakedown();

    // Currently targeted guard for takedown
    AVPGuard* TakedownTarget = nullptr;

    //=========================================================
    // PROTECTED — configurable in Blueprint
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Takedown")
    float TakedownRange = 150.f;

    UPROPERTY(EditDefaultsOnly, Category = "Takedown")
    float TakedownAngle = 180.f; // degrees behind player

    UPROPERTY(EditDefaultsOnly, Category = "Takedown")
    float CarryOffset = 100.f; // distance in front of player

    //=========================================================
    // PUBLIC
    //=========================================================
public:
    UVPInfiltratorComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Called by AVPInfiltrator input
    UFUNCTION(BlueprintCallable, Category = "Takedown")
    void TryTakedown();

    UFUNCTION(BlueprintCallable, Category = "Takedown")
    void DropBody();

    UFUNCTION(BlueprintCallable, Category = "Takedown")
    bool IsCarrying() const { return bIsCarrying; }

    UFUNCTION(BlueprintCallable, Category = "Takedown")
    AVPGuard* GetTakedownTarget() const { return TakedownTarget; }

    // Server RPCs
    UFUNCTION(Server, Reliable)
    void ServerTakedown(AVPGuard* Guard);

    // Multicast — play effects on all clients
    UFUNCTION(NetMulticast, Reliable)
    void MulticastTakedownFX(AVPGuard* Guard);
    UFUNCTION(Server, Reliable)
    void ServerHideBody(AActor* HidingSpot);
};