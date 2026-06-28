// VPGameState.h
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

// Single objective struct
USTRUCT(BlueprintType)
struct FVPObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ID;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    bool bCompleted = false;

    FVPObjective() {}
    FVPObjective(const FString& InID, const FString& InDesc)
        : ID(InID), Description(InDesc), bCompleted(false) {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, EVPAlertLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, const FString&, ObjectiveID);

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

    UPROPERTY(ReplicatedUsing = OnRep_Objectives)
    TArray<FVPObjective> Objectives;

    UPROPERTY(ReplicatedUsing = OnRep_bMissionComplete)
    bool bMissionComplete = false;

    UPROPERTY(ReplicatedUsing = OnRep_bMissionFailed)
    bool bMissionFailed = false;

    UPROPERTY(ReplicatedUsing = OnRep_AlertCountdown)
    float AlertCountdown = 0.f;

    // Timer for alert decay and mission fail
    FTimerHandle AlertDecayTimer;
    FTimerHandle MissionFailTimer;
    FTimerHandle CountdownTickTimer;

    UFUNCTION()
    void OnRep_AlertLevel();

    UFUNCTION()
    void OnRep_Objectives();

    UFUNCTION()
    void OnRep_bMissionComplete();

    UFUNCTION()
    void OnRep_bMissionFailed();

    UFUNCTION()
    void OnRep_AlertCountdown() {}

    void TickAlertCountdown();

    void CheckMissionComplete();

    //=========================================================
    // PUBLIC — delegates + interface
    //=========================================================
public:
    AVPGameState();

    // Delegates — HUD binds to these
    UPROPERTY(BlueprintAssignable, Category = "Alert")
    FOnAlertLevelChanged OnAlertLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnMissionComplete OnMissionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnMissionFailed OnMissionFailed;

    UPROPERTY(BlueprintAssignable, Category = "Mission")
    FOnObjectiveCompleted OnObjectiveCompleted;

    // Last known threat location for guard coordination
    UPROPERTY(Replicated)
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    float GetAlertCountdown() const { return AlertCountdown; }

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Alert system
    void SetAlertLevel(EVPAlertLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Alert")
    EVPAlertLevel GetAlertLevel() const { return AlertLevel; }

    void SetLastKnownThreatLocation(FVector Location);

    // Objective system
    void InitializeObjectives();

    void CheckAllPlayersDowned();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsObjectiveComplete(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    TArray<FVPObjective> GetObjectives() const { return Objectives; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsMissionComplete() const { return bMissionComplete; }

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsMissionFailed() const { return bMissionFailed; }

    void TriggerMissionFail();
};