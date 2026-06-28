// VPPatrolPoint.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VPPatrolPoint.generated.h"

class UBillboardComponent;

UCLASS()
class VOIDPROTOCOL_API AVPPatrolPoint : public AActor
{
    GENERATED_BODY()

private:
    // Visual sphere so you can see it in editor — hidden in game
    UPROPERTY(VisibleAnywhere, Category = "Patrol")
    UBillboardComponent* BillboardComponent;

public:
    AVPPatrolPoint();
};