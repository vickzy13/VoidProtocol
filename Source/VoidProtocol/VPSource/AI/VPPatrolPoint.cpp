#include "VPSource/AI/VPPatrolPoint.h"
#include "Components/BillboardComponent.h"

AVPPatrolPoint::AVPPatrolPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    RootComponent = BillboardComponent;
}