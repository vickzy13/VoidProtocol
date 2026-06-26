// VPHackableTerminal.cpp
#include "VPSource/Actors/VPHackableTerminal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "VPSource/VPGameState.h"

AVPHackableTerminal::AVPHackableTerminal()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalMesh"));
    RootComponent = TerminalMesh;

    ScreenLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ScreenLight"));
    ScreenLight->SetupAttachment(TerminalMesh);
    ScreenLight->SetLightColor(FLinearColor::Blue);
    ScreenLight->Intensity = 300.f;
}

void AVPHackableTerminal::BeginPlay()
{
    Super::BeginPlay();
}

void AVPHackableTerminal::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPHackableTerminal, bIsHacked);
}

void AVPHackableTerminal::OnRep_IsHacked()
{
    // Blue = normal, Green = hacked (data extracted)
    ScreenLight->SetLightColor(bIsHacked ?
        FLinearColor::Green : FLinearColor::Blue);
}

void AVPHackableTerminal::OnHackCompleted(AActor* Hacker)
{
    if (!HasAuthority()) return;

    bIsHacked = true;
    OnRep_IsHacked();

    // Complete the steal_data objective when terminal is hacked
    if (AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>())
        GS->CompleteObjective("steal_data");

    GetWorldTimerManager().SetTimer(HackResetTimer, [this]()
        {
            bIsHacked = false;
            OnRep_IsHacked();
        }, HackDuration, false);
}

FVector AVPHackableTerminal::GetHackWidgetLocation() const
{
    return GetActorLocation() + FVector(0.f, 0.f, 80.f);
}

TArray<AActor*> AVPHackableTerminal::GetAdjacentHackables() const
{
    TArray<AActor*> Result;
    for (AVPHackableTerminal* Terminal : AdjacentTerminals)
        if (Terminal) Result.Add(Terminal);
    return Result;
}
