#include "VPSource/Actors/VPLockedDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Net/UnrealNetwork.h"

AVPLockedDoor::AVPLockedDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    RootComponent = DoorFrame;

    LeftPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPanel"));
    LeftPanel->SetupAttachment(DoorFrame);

    RightPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPanel"));
    RightPanel->SetupAttachment(DoorFrame);

    StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
    StatusLight->SetupAttachment(DoorFrame);
    StatusLight->SetLightColor(FLinearColor::Red);
    StatusLight->Intensity = 500.f;
}

void AVPLockedDoor::BeginPlay()
{
    Super::BeginPlay();
}

void AVPLockedDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsOpening) return;

    FVector NewLeft = FMath::VInterpTo(
        LeftPanel->GetRelativeLocation(),
        LeftPanelTargetLoc, DeltaTime, OpenSpeed);

    FVector NewRight = FMath::VInterpTo(
        RightPanel->GetRelativeLocation(),
        RightPanelTargetLoc, DeltaTime, OpenSpeed);

    LeftPanel->SetRelativeLocation(NewLeft);
    RightPanel->SetRelativeLocation(NewRight);

    // Stop ticking when panels reach target
    if (FVector::Dist(NewLeft, LeftPanelTargetLoc) < 1.f &&
        FVector::Dist(NewRight, RightPanelTargetLoc) < 1.f)
    {
        bIsOpening = false;
    }
}

void AVPLockedDoor::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPLockedDoor, bIsUnlocked);
}

void AVPLockedDoor::OnRep_IsUnlocked()
{
    if (bIsUnlocked)
        OpenDoor();
}

void AVPLockedDoor::OpenDoor()
{
    // Panels slide outward on Y axis
    LeftPanelTargetLoc = LeftPanel->GetRelativeLocation() +
        FVector(OpenDistance, 0.f , 0.f);
    RightPanelTargetLoc = RightPanel->GetRelativeLocation() +
        FVector(-OpenDistance, 0.f, 0.f);

    bIsOpening = true;

    // Light goes green
    StatusLight->SetLightColor(FLinearColor::Green);

    UE_LOG(LogTemp, Warning, TEXT("Door %s opening"), *GetName());
}

void AVPLockedDoor::OnHackCompleted(AActor* Hacker)
{
    if (!HasAuthority()) return;

    bIsUnlocked = true;
    OnRep_IsUnlocked(); // manual call on server

    UE_LOG(LogTemp, Warning, TEXT("Door %s unlocked"), *GetName());
}

FVector AVPLockedDoor::GetHackWidgetLocation() const
{
    return GetActorLocation() + FVector(0.f, 0.f, 100.f);
}