#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPObjectiveTrigger.generated.h"

class UBoxComponent;
class UBillboardComponent;

UCLASS()
class VOIDPROTOCOL_API AVPObjectiveTrigger : public AActor
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBillboardComponent* Billboard;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappingComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

protected:
    // Must match objective ID in AVPGameState::InitializeObjectives
    UPROPERTY(EditInstanceOnly, Category = "Objective")
    FString ObjectiveID;

    // Visual label shown in editor
    UPROPERTY(EditInstanceOnly, Category = "Objective")
    FString ObjectiveLabel = "Objective";

    // Can this be triggered by any player or only Infiltrator/Hacker?
    UPROPERTY(EditDefaultsOnly, Category = "Objective")
    bool bAnyPlayerCanTrigger = true;

public:
    AVPObjectiveTrigger();
    virtual void BeginPlay() override;
};