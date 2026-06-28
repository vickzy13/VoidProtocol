// VPGuardController.h
#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "VPGuardController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UCLASS()
class VOIDPROTOCOL_API AVPGuardController : public AAIController
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — components + detection state
    //=========================================================
private:
    UPROPERTY(VisibleAnywhere, Category="AI")
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, Category="AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, Category="AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Current chase target
    AActor* CurrentTarget = nullptr;

    // Timer to continuously update target location while chasing
    FTimerHandle TargetUpdateTimer;

    // Per-actor detection meters (Hitman-style gradual detection)
    TMap<AActor*, float> DetectionMeters;
    TArray<AActor*> VisibleActors;

    // Detection tuning
    float DetectionFillRateNear = 80.f;   // fast fill when close
    float DetectionFillRateFar  = 25.f;   // slow fill when far
    float NearDistance          = 300.f;
    float FarDistance           = 1000.f;
    float DetectionDecayRate    = 12.f;   // decay when out of sight
    float SuspiciousThreshold   = 40.f;
    float AlertedThreshold      = 100.f;

    // Detection tick timer
    FTimerHandle DetectionTickTimer;

    // Internal helpers
    void TickDetection();
    void UpdateTargetLocation();

    //=========================================================
    // PROTECTED — configurable in BP_VPGuardController
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category="AI")
    UBehaviorTree* GuardBehaviorTree;

    // Random patrol radius around spawn point
    UPROPERTY(EditDefaultsOnly, Category="AI|Patrol")
    float PatrolRadius = 1500.f;

    // Guard's spawn location — used as center of patrol area
    FVector SpawnLocation = FVector::ZeroVector;

    // Blackboard keys — must match BB_VPGuard asset exactly
    static const FName BBKey_TargetActor;
    static const FName BBKey_PatrolIndex;
    static const FName BBKey_AlertState;
    static const FName BBKey_TargetLocation;
    static const FName BBKey_HasLastKnownLocation;
    static const FName BBKey_PatrolPoint;

    //=========================================================
    // PUBLIC — overrides + external interface
    //=========================================================
public:
    AVPGuardController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Perception callback
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Called by BT to get next random patrol point
    UFUNCTION(BlueprintCallable, Category="AI|Patrol")
    void PickNextPatrolPoint();

    // Chase target management
    UFUNCTION(BlueprintCallable, Category="AI")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category="AI")
    void ClearTargetActor();

    // Clear all detection — called when guard goes unconscious
    void ClearAllDetection();

    // Accessor for BT restart on recovery
    UFUNCTION(BlueprintCallable, Category="AI")
    UBehaviorTree* GetGuardBehaviorTree() const { return GuardBehaviorTree; }
};
