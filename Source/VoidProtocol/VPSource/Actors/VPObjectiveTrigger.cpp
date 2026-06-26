// VPObjectiveTrigger.cpp
#include "VPSource/Actors/VPObjectiveTrigger.h"
#include "VPSource/VPGameState.h"
#include "VPSource/Characters/VPCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

AVPObjectiveTrigger::AVPObjectiveTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));

    Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    Billboard->SetupAttachment(RootComponent);
}

void AVPObjectiveTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(
        this, &AVPObjectiveTrigger::OnOverlapBegin);
}

void AVPObjectiveTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappingComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (ObjectiveID.IsEmpty()) return;

    // Must be a player character
    if (!Cast<AVPCharacter>(OtherActor)) return;

    AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>();
    if (!GS) return;

    // Don't complete twice
    if (GS->IsObjectiveComplete(ObjectiveID)) return;

    GS->CompleteObjective(ObjectiveID);

    UE_LOG(LogTemp, Warning, TEXT("Objective triggered: %s by %s"),
        *ObjectiveID, *OtherActor->GetName());
}