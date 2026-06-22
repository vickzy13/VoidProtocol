#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "VPGameState.generated.h"

UENUM(BlueprintType)
enum class EVPAlertLevel : uint8
{
    Unaware    UMETA(DisplayName = "Unaware"),
    Suspicious UMETA(DisplayName = "Suspicious"),
    Alerted    UMETA(DisplayName = "Alerted")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, EVPAlertLevel, NewLevel);

UCLASS()
class VOIDPROTOCOL_API AVPGameState : public AGameStateBase
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — internal state
    //=========================================================
private:
    UPROPERTY(ReplicatedUsing = OnRep_AlertLevel)
    EVPAlertLevel AlertLevel = EVPAlertLevel::Unaware;

    // Timer to return to Unaware if no threats detected
    FTimerHandle AlertDecayTimer;

    UFUNCTION()
    void OnRep_AlertLevel();

    //=========================================================
    // PUBLIC — accessible by guards, HUD, GameMode
    //=========================================================
public:
    AVPGameState();

    // Broadcast when alert level changes — HUD binds to this
    UPROPERTY(BlueprintAssignable, Category = "Alert")
    FOnAlertLevelChanged OnAlertLevelChanged;

    // Called by GameMode (server only)
    void SetAlertLevel(EVPAlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Alert")
    EVPAlertLevel GetAlertLevel() const { return AlertLevel; }

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};