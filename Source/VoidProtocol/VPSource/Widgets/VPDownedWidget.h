#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPDownedWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class VOIDPROTOCOL_API UVPDownedWidget : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UImage* DarkOverlay;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DownedText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ReviveCountdown;

    float ReviveTimer = 30.f;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};