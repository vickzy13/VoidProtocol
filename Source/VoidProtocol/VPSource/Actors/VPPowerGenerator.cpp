#include "VPSource/Actors/VPPowerGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Light.h"
#include "Kismet/GameplayStatics.h"
#include "VPSource/VPGameState.h"

AVPPowerGenerator::AVPPowerGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
    RootComponent = GeneratorMesh;

    StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
    StatusLight->SetupAttachment(GeneratorMesh);
    StatusLight->SetLightColor(FLinearColor::Green);
    StatusLight->Intensity = 800.f;
}

void AVPPowerGenerator::BeginPlay()
{
    Super::BeginPlay();
}

void AVPPowerGenerator::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPPowerGenerator, bIsPowerDown);
}

void AVPPowerGenerator::OnRep_IsPowerDown()
{
    if (bIsPowerDown)
        CutPower();
    else
        RestorePower();
}

void AVPPowerGenerator::CutPower()
{
    // Generator light goes red/dim
    StatusLight->SetLightColor(FLinearColor::Red);
    StatusLight->Intensity = 200.f;

    // Full light cutoff implementation on Day 12
    // For now just log the event
    UE_LOG(LogTemp, Warning, TEXT("POWER CUT — darkness for %.0fs"), PowerDownDuration);
}

void AVPPowerGenerator::RestorePower()
{
    StatusLight->SetLightColor(FLinearColor::Green);
    StatusLight->Intensity = 800.f;

    UE_LOG(LogTemp, Warning, TEXT("POWER RESTORED"));
}

void AVPPowerGenerator::OnHackCompleted(AActor* Hacker)
{
    if (!HasAuthority()) return;

    bIsPowerDown = true;
    OnRep_IsPowerDown();

    // Complete disable_reactor objective
    if (AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>())
        GS->CompleteObjective("disable_reactor");

    GetWorldTimerManager().SetTimer(PowerRestoreTimer, [this]()
        {
            bIsPowerDown = false;
            OnRep_IsPowerDown();
        }, PowerDownDuration, false);
}

FVector AVPPowerGenerator::GetHackWidgetLocation() const
{
    return GetActorLocation() + FVector(0.f, 0.f, 100.f);
}
