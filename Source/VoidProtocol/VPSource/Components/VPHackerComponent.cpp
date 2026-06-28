#include "VPSource/Components/VPHackerComponent.h"
#include "VPSource/Interfaces/VPHackable.h"
#include "VPSource/Actors/VPSecurityCamera.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "VPSource/Widgets/VPHackProgressWidget.h"
#include "VPSource/Widgets/VPCameraViewWidget.h"
#include "VPSource/Actors/VPHackableTerminal.h"

UVPHackerComponent::UVPHackerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UVPHackerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Scan for nearby hackables every 0.2s
    GetOwner()->GetWorldTimerManager().SetTimer(HackScanTimer,
        this, &UVPHackerComponent::ScanForHackable, 0.2f, true);
}

void UVPHackerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsHacking)
        UpdateHackProgress(DeltaTime);
}

void UVPHackerComponent::ScanForHackable()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner || !Owner->IsLocallyControlled()) return;
    if (bInCameraMode) return;

    FVector Start = Owner->GetActorLocation();
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(HackRange);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    GetWorld()->OverlapMultiByChannel(Overlaps, Start, FQuat::Identity,
        ECC_WorldStatic, Sphere, Params);

    AActor* BestHackable = nullptr;
    float BestDist = HackRange;

    for (auto& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!Actor) continue;

        IVPHackable* Hackable = Cast<IVPHackable>(Actor);
        if (!Hackable || !Hackable->CanBeHacked()) continue;

        float Dist = FVector::Dist(Start, Actor->GetActorLocation());
        //UE_LOG(LogTemp, Warning, TEXT("Found hackable: %s at dist %.0f"),
            //*Actor->GetName(), Dist);

        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestHackable = Actor;
        }
    }

    // Show widget only when target exists
    if (BestHackable != TargetHackable)
    {
        // Target changed
        if (HackProgressWidget)
        {
            HackProgressWidget->RemoveFromParent();
            HackProgressWidget = nullptr;
        }
        CancelHack();
    }

    TargetHackable = BestHackable;

    if (TargetHackable && !HackProgressWidget && HackProgressWidgetClass)
    {
        APlayerController* PC = Cast<APlayerController>(Owner->GetController());
        if (PC)
        {
            HackProgressWidget = CreateWidget<UVPHackProgressWidget>(
                PC, HackProgressWidgetClass);
            if (HackProgressWidget)
            {
                HackProgressWidget->SetHackerComponent(this);
                HackProgressWidget->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("Hack widget shown for: %s"),
                    *TargetHackable->GetName());
            }
        }
    }
    else if (!TargetHackable && HackProgressWidget)
    {
        HackProgressWidget->RemoveFromParent();
        HackProgressWidget = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Hack widget hidden — no target"));
    }
}

void UVPHackerComponent::StartHack()
{
    if (!TargetHackable || bIsHacking || bInCameraMode) return;

    bIsHacking = true;
    HackProgress = 0.f;

    ServerStartHack(TargetHackable);

    UE_LOG(LogTemp, Warning, TEXT("Hacker: Starting hack on %s"),
        *TargetHackable->GetName());
}

void UVPHackerComponent::StopHack()
{
    if (!bIsHacking) return;
    CancelHack();
}

void UVPHackerComponent::UpdateHackProgress(float DeltaTime)
{
    if (!TargetHackable)
    {
        CancelHack();
        return;
    }

    HackProgress += DeltaTime / HackDuration;

    if (HackProgress >= 1.f)
    {
        HackProgress = 1.f;
        CompleteHack();
    }
}

void UVPHackerComponent::CompleteHack()
{
    bIsHacking = false;
    if (!TargetHackable) return;

    ServerCompleteHack(TargetHackable);

    if (AVPSecurityCamera* Cam = Cast<AVPSecurityCamera>(TargetHackable))
    {
        // Camera — enter camera view
        EnterCameraMode(Cam);
    }
    else if (AVPHackableTerminal* Terminal = Cast<AVPHackableTerminal>(TargetHackable))
    {
        // Terminal — enter network mode
        bInCameraMode = true;
        CurrentHackNode = TargetHackable;
        CurrentCamera = nullptr;

        APawn* Owner = Cast<APawn>(GetOwner());
        APlayerController* PC = Cast<APlayerController>(
            Owner ? Owner->GetController() : nullptr);

        if (PC && CameraViewWidgetClass && !CameraViewWidget)
        {
            CameraViewWidget = CreateWidget<UVPCameraViewWidget>(PC, CameraViewWidgetClass);
            if (CameraViewWidget)
            {
                CameraViewWidget->AddToViewport();
                TArray<AActor*> Adjacent = Terminal->GetAdjacentHackables();
                CameraViewWidget->UpdateCameraInfo(
                    FString::Printf(TEXT("NETWORK: %s"), *Terminal->GetName()),
                    Adjacent.Num());
            }
        }
    }
    else
    {
        // Door, generator, other — just complete, no mode change
        // bInCameraMode stays FALSE — scan continues immediately
        UE_LOG(LogTemp, Warning, TEXT("Hack complete: %s"), *TargetHackable->GetName());
    }

    HackProgress = 0.f;
    TargetHackable = nullptr;

    if (HackProgressWidget)
    {
        HackProgressWidget->RemoveFromParent();
        HackProgressWidget = nullptr;
    }
}

void UVPHackerComponent::CancelHack()
{
    if (!bIsHacking) return;

    bIsHacking = false;
    HackProgress = 0.f;

    if (TargetHackable)
    {
        ServerCancelHack(TargetHackable);
        IVPHackable* Hackable = Cast<IVPHackable>(TargetHackable);
        if (Hackable) Hackable->OnHackCancelled();
    }
}

void UVPHackerComponent::EnterCameraMode(AVPSecurityCamera* Camera)
{
    if (!Camera) return;

    APawn* Owner = Cast<APawn>(GetOwner());
    APlayerController* PC = Cast<APlayerController>(
        Owner ? Owner->GetController() : nullptr);
    if (!PC) return;

    CurrentCamera = Camera;
    CurrentHackNode = Camera;
    bInCameraMode = true;

    PC->SetViewTarget(Camera);

    if (CameraViewWidgetClass && !CameraViewWidget)
    {
        CameraViewWidget = CreateWidget<UVPCameraViewWidget>(PC, CameraViewWidgetClass);
        if (CameraViewWidget)
        {
            CameraViewWidget->SetHackerComponent(this);
            CameraViewWidget->AddToViewport();

            // Update camera info display
            TArray<AActor*> Adjacent = Camera->GetAdjacentHackables();
            CameraViewWidget->UpdateCameraInfo(Camera->GetName(), Adjacent.Num());
        }
    }
    else if (CameraViewWidget)
    {
        // Already exists — just update info
        TArray<AActor*> Adjacent = Camera->GetAdjacentHackables();
        CameraViewWidget->UpdateCameraInfo(Camera->GetName(), Adjacent.Num());
    }
}

void UVPHackerComponent::ExitCamera()
{
    if (!bInCameraMode) return;

    APawn* Owner = Cast<APawn>(GetOwner());
    APlayerController* PC = Cast<APlayerController>(
        Owner ? Owner->GetController() : nullptr);
    if (!PC) return;

    bInCameraMode = false;
    CurrentCamera = nullptr;
    CurrentHackNode = nullptr; // ← add this

    PC->SetViewTarget(Owner);

    if (CameraViewWidget)
    {
        CameraViewWidget->RemoveFromParent();
        CameraViewWidget = nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("Exited hack node view"));
}

void UVPHackerComponent::JumpToNextNode()
{
    UE_LOG(LogTemp, Warning, TEXT("JumpToNextNode: bInCameraMode=%d | CurrentHackNode=%s | CurrentCamera=%s"),
        bInCameraMode,
        CurrentHackNode ? *CurrentHackNode->GetName() : TEXT("NULL"),
        CurrentCamera ? *CurrentCamera->GetName() : TEXT("NULL"));

    if (!bInCameraMode)
    {
        UE_LOG(LogTemp, Error, TEXT("JumpToNextNode: NOT in camera mode — returning"));
        return;
    }

    if (!CurrentHackNode)
    {
        UE_LOG(LogTemp, Error, TEXT("JumpToNextNode: CurrentHackNode is NULL — returning"));
        return;
    }

    IVPHackable* CurrentHackable = Cast<IVPHackable>(CurrentHackNode);
    if (!CurrentHackable)
    {
        UE_LOG(LogTemp, Error, TEXT("JumpToNextNode: Cast to IVPHackable FAILED"));
        return;
    }

    TArray<AActor*> Adjacent = CurrentHackable->GetAdjacentHackables();
    UE_LOG(LogTemp, Warning, TEXT("JumpToNextNode: Found %d adjacent nodes"), Adjacent.Num());

    if (Adjacent.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("JumpToNextNode: No adjacent nodes — did you set AdjacentTerminals in the level?"));
        return;
    }

    for (AActor* Actor : Adjacent)
    {
        UE_LOG(LogTemp, Warning, TEXT("JumpToNextNode: Trying to jump to %s"),
            Actor ? *Actor->GetName() : TEXT("NULL"));

        if (!Actor) continue;

        IVPHackable* Hackable = Cast<IVPHackable>(Actor);
        if (!Hackable)
        {
            UE_LOG(LogTemp, Error, TEXT("JumpToNextNode: Cast to IVPHackable failed for %s"),
                *Actor->GetName());
            continue;
        }

        ServerCompleteHack(Actor);
        CurrentHackNode = Actor;

        if (AVPSecurityCamera* NextCam = Cast<AVPSecurityCamera>(Actor))
        {
            APawn* Owner = Cast<APawn>(GetOwner());
            APlayerController* PC = Cast<APlayerController>(
                Owner ? Owner->GetController() : nullptr);
            if (PC) PC->SetViewTarget(NextCam);
            CurrentCamera = NextCam;

            if (CameraViewWidget)
            {
                TArray<AActor*> Next = NextCam->GetAdjacentHackables();
                CameraViewWidget->UpdateCameraInfo(NextCam->GetName(), Next.Num());
            }
            UE_LOG(LogTemp, Warning, TEXT("Jumped to camera: %s"), *NextCam->GetName());
        }
        else if (AVPHackableTerminal* NextTerminal = Cast<AVPHackableTerminal>(Actor))
        {
            if (CameraViewWidget)
            {
                TArray<AActor*> Next = NextTerminal->GetAdjacentHackables();
                CameraViewWidget->UpdateCameraInfo(
                    FString::Printf(TEXT("NETWORK: %s"), *NextTerminal->GetName()),
                    Next.Num());
            }
            UE_LOG(LogTemp, Warning, TEXT("Jumped to terminal: %s"), *NextTerminal->GetName());
        }

        return;
    }
}
// Server RPCs
void UVPHackerComponent::ServerStartHack_Implementation(AActor* Hackable)
{
    if (!Hackable) return;
    IVPHackable* H = Cast<IVPHackable>(Hackable);
    if (H && H->CanBeHacked())
        H->OnHackStarted(GetOwner());
}

void UVPHackerComponent::ServerCompleteHack_Implementation(AActor* Hackable)
{
    if (!Hackable) return;
    IVPHackable* H = Cast<IVPHackable>(Hackable);
    if (H) H->OnHackCompleted(GetOwner());
}

void UVPHackerComponent::ServerCancelHack_Implementation(AActor* Hackable)
{
    if (!Hackable) return;
    IVPHackable* H = Cast<IVPHackable>(Hackable);
    if (H) H->OnHackCancelled();
}