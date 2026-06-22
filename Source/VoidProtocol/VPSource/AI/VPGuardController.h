#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "VPGuardController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class VOIDPROTOCOL_API AVPGuardController : public AAIController
{
    GENERATED_BODY()
   
private:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    FTimerHandle TargetUpdateTimer;
    AActor* CurrentTarget = nullptr;

    TMap<AActor*, float> DetectionMeters;
    TArray<AActor*> VisibleActors;

    // VPGuardController.h — add:
private:
    float DetectionFillRateNear = 80.f;  // close range, fast
    float DetectionFillRateFar = 25.f;   // far range, slow
    float NearDistance = 300.f;
    float FarDistance = 1000.f;
    float DetectionFillRate = 50.f;   // % per second while visible
    float DetectionDecayRate = 20.f;  // % per second while not visible
    float SuspiciousThreshold = 40.f;
    float AlertedThreshold = 100.f;

    FTimerHandle DetectionTickTimer;

    void TickDetection();

    void UpdateTargetLocation();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* GuardBehaviorTree;

    // Blackboard keys
    static const FName BBKey_TargetActor;
    static const FName BBKey_PatrolIndex;
    static const FName BBKey_AlertState;
    static const FName BBKey_TargetLocation;
    static const FName BBKey_HasLastKnownLocation;
    static const FName BBKey_PatrolPoint;

    //=========================================================
    // PUBLIC — overrides
    //=========================================================
public:
    AVPGuardController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Called by perception system when something is sensed
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Accessors for Behavior Tree tasks
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ClearTargetActor();
};