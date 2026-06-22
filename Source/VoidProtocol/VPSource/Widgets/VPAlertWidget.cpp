#include "VPSource/Widgets/VPAlertWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "VPSource/VPGameState.h"
#include "Kismet/GameplayStatics.h"

void UVPAlertWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind to GameState delegate
    if (AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>())
    {
        GS->OnAlertLevelChanged.AddDynamic(this, &UVPAlertWidget::OnAlertLevelChanged);
        // Set initial state
        OnAlertLevelChanged(GS->GetAlertLevel());
    }
}

void UVPAlertWidget::OnAlertLevelChanged(EVPAlertLevel NewLevel)
{
    if (!AlertText) return;

    switch (NewLevel)
    {
    case EVPAlertLevel::Unaware:
        AlertText->SetText(FText::FromString(""));
        if (AlertBackground)
            AlertBackground->SetVisibility(ESlateVisibility::Hidden);
        break;

    case EVPAlertLevel::Suspicious:
        AlertText->SetText(FText::FromString("! SUSPICIOUS"));
        if (AlertBackground)
        {
            AlertBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
            AlertBackground->SetColorAndOpacity(FLinearColor(1.f, 0.85f, 0.f, 1.f)); // yellow
        }
        break;

    case EVPAlertLevel::Alerted:
        AlertText->SetText(FText::FromString("!! ALERTED"));
        if (AlertBackground)
        {
            AlertBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
            AlertBackground->SetColorAndOpacity(FLinearColor::Red);
        }
        break;
    }
}

void UVPAlertWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!AlertBackground) return;

    AVPGameState* GS = GetWorld()->GetGameState<AVPGameState>();
    if (!GS) return;

    EVPAlertLevel Level = GS->GetAlertLevel();

    // Pulse effect for Suspicious and Alerted states
    if (Level != EVPAlertLevel::Unaware)
    {
        PulseTimer += InDeltaTime * (Level == EVPAlertLevel::Alerted ? 3.f : 1.5f);
        float PulseAlpha = (FMath::Sin(PulseTimer * PI) + 1.f) * 0.5f;
        float Opacity = FMath::Lerp(0.5f, 1.f, PulseAlpha);

        FLinearColor Color = AlertBackground->GetColorAndOpacity();
        Color.A = Opacity;
        AlertBackground->SetColorAndOpacity(Color);
    }
}