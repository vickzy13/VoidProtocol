#include "VPSource/Actors/VPSecurityCamera.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Net/UnrealNetwork.h"

AVPSecurityCamera::AVPSecurityCamera()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    CameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CameraMesh"));
    RootComponent = CameraMesh;

    CameraView = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraView"));
    CameraView->SetupAttachment(CameraMesh);
    CameraView->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));

    CameraLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("CameraLight"));
    CameraLight->SetupAttachment(CameraMesh);
    CameraLight->SetLightColor(FLinearColor::Red);
    CameraLight->Intensity = 1000.f;
    CameraLight->OuterConeAngle = 30.f;

    RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(
        TEXT("RotatingMovement"));
    RotatingMovement->RotationRate = FRotator(0.f, PatrolRotationRate, 0.f);
}

void AVPSecurityCamera::BeginPlay()
{
    Super::BeginPlay();
    UpdateVisuals();
}

void AVPSecurityCamera::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPSecurityCamera, bIsDisabled);
}

void AVPSecurityCamera::OnRep_IsDisabled()
{
    UpdateVisuals();
}

void AVPSecurityCamera::UpdateVisuals()
{
    if (bIsDisabled)
    {
        // Stop rotating
        RotatingMovement->RotationRate = FRotator::ZeroRotator;

        // Light off
        CameraLight->SetLightColor(FLinearColor(0.2f, 0.2f, 0.2f));
        CameraLight->Intensity = 200.f;

        // Dark material
        if (DisabledMaterial)
            CameraMesh->SetMaterial(0, DisabledMaterial);
    }
    else
    {
        // Resume rotating
        RotatingMovement->RotationRate = FRotator(0.f, PatrolRotationRate, 0.f);

        // Red light back on
        CameraLight->SetLightColor(FLinearColor::Red);
        CameraLight->Intensity = 1000.f;

        // Active material
        if (ActiveMaterial)
            CameraMesh->SetMaterial(0, ActiveMaterial);
    }
}

void AVPSecurityCamera::OnHackStarted(AActor* Hacker)
{
    // Progress bar widget spawned by HackerComponent — nothing needed here
}

void AVPSecurityCamera::OnHackCompleted(AActor* Hacker)
{
    if (!HasAuthority()) return;

    bIsDisabled = true;
    OnRep_IsDisabled(); // manual call on server

    // Re-enable after DisableDuration
    GetWorldTimerManager().SetTimer(ReEnableTimer, [this]()
        {
            bIsDisabled = false;
            OnRep_IsDisabled();
            UE_LOG(LogTemp, Warning, TEXT("Camera %s re-enabled"), *GetName());
        }, DisableDuration, false);

    UE_LOG(LogTemp, Warning, TEXT("Camera %s hacked for %.0fs"), *GetName(), DisableDuration);
}

void AVPSecurityCamera::OnHackCancelled()
{
    // Nothing to undo — hack wasn't completed
}

FVector AVPSecurityCamera::GetHackWidgetLocation() const
{
    return GetActorLocation() + FVector(0.f, 0.f, 50.f);
}

TArray<AActor*> AVPSecurityCamera::GetAdjacentHackables() const
{
    TArray<AActor*> Result;
    for (AVPSecurityCamera* Cam : AdjacentCameras)
        if (Cam) Result.Add(Cam);
    return Result;
}