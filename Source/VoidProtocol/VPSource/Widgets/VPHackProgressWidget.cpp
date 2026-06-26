// VPHackProgressWidget.cpp
#include "VPSource/Widgets/VPHackProgressWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "VPSource/Components/VPHackerComponent.h"

void UVPHackProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (HackProgressBar)
    {
        HackProgressBar->SetFillColorAndOpacity(FLinearColor(0.f, 1.f, 1.f, 1.f)); // cyan
        HackProgressBar->SetPercent(0.f);
    }

    if (HackLabel)
        HackLabel->SetText(FText::FromString("HACKING..."));
}

void UVPHackProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!HackerComponent) return;

    if (HackProgressBar)
        HackProgressBar->SetPercent(HackerComponent->GetHackProgress());
}

void UVPHackProgressWidget::SetHackerComponent(UVPHackerComponent* Component)
{
    HackerComponent = Component;
}