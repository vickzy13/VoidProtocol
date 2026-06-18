// VPGuard.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VPGuard.generated.h"

UCLASS()
class VOIDPROTOCOL_API AVPGuard : public ACharacter
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — patrol state
    //=========================================================
private:
    int32 CurrentPatrolIndex = 0;

    //=========================================================
    // PROTECTED — configurable in BP_VPGuard
    //=========================================================
protected:
    // Patrol waypoints — assign in level via BP_VPGuard details
    UPROPERTY(EditInstanceOnly, Category = "Patrol")
    TArray<AActor*> PatrolPoints;

    UPROPERTY(EditDefaultsOnly, Category = "Patrol")
    float PatrolWaitTime = 2.f;

    UPROPERTY(EditDefaultsOnly, Category = "Patrol")
    float PatrolAcceptanceRadius = 50.f;

    //=========================================================
    // PUBLIC — overrides + patrol interface
    //=========================================================
public:
    AVPGuard();

    virtual void BeginPlay() override;

    // Called by BT Task to get next patrol point
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    AActor* GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    float GetPatrolWaitTime() const { return PatrolWaitTime; }

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    bool HasPatrolPoints() const { return PatrolPoints.Num() > 0; }
};