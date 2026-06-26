#include "VPSource/Widgets/VPCameraViewWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "VPSource/Components/VPHackerComponent.h"

void UVPCameraViewWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Scanline overlay — dark green tint
    if (ScanlineOverlay)
        ScanlineOverlay->SetColorAndOpacity(FLinearColor(0.f, 0.8f, 0.2f, 0.15f));

    if (ExitPrompt)
        ExitPrompt->SetText(FText::FromString("[ESC] EXIT CAMERA"));

    if (NextCameraPrompt)
        NextCameraPrompt->SetText(FText::FromString("[F] NEXT CAMERA"));

    if (CameraLabel)
        CameraLabel->SetText(FText::FromString("CAMERA FEED"));
}

void UVPCameraViewWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Subtle scanline flicker
    ScanlineTimer += InDeltaTime * 2.f;
    if (ScanlineOverlay)
    {
        float Alpha = 0.1f + FMath::Sin(ScanlineTimer) * 0.05f;
        FLinearColor Color = ScanlineOverlay->GetColorAndOpacity();
        Color.A = Alpha;
        ScanlineOverlay->SetColorAndOpacity(Color);
    }
}

void UVPCameraViewWidget::SetHackerComponent(UVPHackerComponent* Component)
{
    HackerComponent = Component;
}

void UVPCameraViewWidget::UpdateCameraInfo(const FString& CameraName, int32 AdjacentCount)
{
    if (CameraLabel)
        CameraLabel->SetText(FText::FromString(
            FString::Printf(TEXT("CAMERA: %s"), *CameraName)));

    if (NextCameraPrompt)
    {
        FString Prompt = AdjacentCount > 0
            ? FString::Printf(TEXT("[F] NEXT CAMERA (%d available)"), AdjacentCount)
            : TEXT("[F] NO ADJACENT CAMERAS");
        NextCameraPrompt->SetText(FText::FromString(Prompt));
    }
}