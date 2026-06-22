// Fill out your copyright notice in the Description page of Project Settings.


#include "VPSource/Widgets/VPDetectionWidget.h"
#include "Components/Image.h"
#include "VPSource/Characters/VPCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"

void UVPDetectionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    OwningCharacter = Cast<AVPCharacter>(GetOwningPlayerPawn());

    if (DetectionArc)
        DetectionArc->SetVisibility(ESlateVisibility::Hidden);
}

void UVPDetectionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!OwningCharacter)
    {
        OwningCharacter = Cast<AVPCharacter>(GetOwningPlayerPawn());
        if (OwningCharacter)
            UE_LOG(LogTemp, Warning, TEXT("DetectionWidget: OwningCharacter acquired: %s"), *OwningCharacter->GetName());
        return;
    }

    UpdateDetectionVisual();
}

void UVPDetectionWidget::UpdateDetectionVisual()
{
    if (!DetectionArc || !OwningCharacter) return;

    float Level = OwningCharacter->GetDetectionLevel();

    // Hidden when no detection at all
    if (Level <= 0.f)
    {
        DetectionArc->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    DetectionArc->SetVisibility(ESlateVisibility::HitTestInvisible);

    // Color: yellow ramping up, red + full opacity at 100
    FLinearColor Color;
    float Opacity = FMath::Clamp(Level / 100.f, 0.f, 1.f);

    if (Level >= 100.f)
        Color = FLinearColor::Red;
    else
        Color = FLinearColor(1.f, 0.85f, 0.f, 1.f); // yellow

    Color.A = Opacity;
    DetectionArc->SetColorAndOpacity(Color);

    // Rotate arc to point toward threat, relative to camera facing
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    FVector PlayerLoc = OwningCharacter->GetActorLocation();
    FVector ThreatLoc = OwningCharacter->GetThreatLocation();

    FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, ThreatLoc);
    float WorldYawToThreat = LookAtRot.Yaw;

    FRotator CameraRot = PC->PlayerCameraManager
        ? PC->PlayerCameraManager->GetCameraRotation()
        : FRotator::ZeroRotator;
    float CameraYaw = CameraRot.Yaw;

    float RelativeAngle = WorldYawToThreat - CameraYaw;

    // Normalize to -180..180
    RelativeAngle = FMath::UnwindDegrees(RelativeAngle);

    DetectionArc->SetRenderTransformAngle(RelativeAngle);
}