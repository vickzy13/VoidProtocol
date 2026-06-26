#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPHidingSpot.generated.h"

class UBoxComponent;
class AVPGuard;

UCLASS()
class VOIDPROTOCOL_API AVPHidingSpot : public AActor
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* TriggerVolume;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappingComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
    AVPHidingSpot();
    virtual void BeginPlay() override;
};