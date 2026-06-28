#include "VPSource/Widgets/VPDownedWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UVPDownedWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (DarkOverlay)
        DarkOverlay->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.7f));

    if (DownedText)
        DownedText->SetText(FText::FromString("OPERATOR DOWN\nAwait partner revival"));

    ReviveTimer = 30.f;
}

void UVPDownedWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ReviveTimer = FMath::Max(0.f, ReviveTimer - InDeltaTime);

    if (ReviveCountdown)
        ReviveCountdown->SetText(FText::FromString(
            FString::Printf(TEXT("%.0f"), ReviveTimer)));

    // Flash red as timer runs out
    if (ReviveTimer < 10.f && DownedText)
    {
        float Flash = FMath::Sin(GetWorld()->GetTimeSeconds() * 5.f) * 0.5f + 0.5f;
        DownedText->SetColorAndOpacity(FSlateColor(
            FLinearColor(1.f, Flash * 0.3f, Flash * 0.3f, 1.f)));
    }
}