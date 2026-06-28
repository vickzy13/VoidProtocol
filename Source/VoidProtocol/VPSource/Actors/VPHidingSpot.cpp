#include "VPSource/Actors/VPHidingSpot.h"
#include "Components/BoxComponent.h"
#include "VPSource/Characters/VPGuard.h"
#include "VPSource/Components/VPInfiltratorComponent.h"
#include "VPSource/Characters/VPInfiltrator.h"

AVPHidingSpot::AVPHidingSpot()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(100.f, 100.f, 100.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AVPHidingSpot::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(
        this, &AVPHidingSpot::OnOverlapBegin);
}

void AVPHidingSpot::OnOverlapBegin(UPrimitiveComponent* OverlappingComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AVPInfiltrator* Infiltrator = Cast<AVPInfiltrator>(OtherActor);
    if (!Infiltrator) return;

    UVPInfiltratorComponent* IC =
        Infiltrator->FindComponentByClass<UVPInfiltratorComponent>();
    if (!IC || !IC->IsCarrying()) return;

    UE_LOG(LogTemp, Warning, TEXT("Hiding body in %s"), *GetName());
    IC->ServerHideBody(this); // ← call this directly, not DropBody
}