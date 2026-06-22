// VPAlertWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPSource/VPGameState.h"
#include "VPAlertWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class VOIDPROTOCOL_API UVPAlertWidget : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UImage* AlertBackground;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AlertText;

    float PulseTimer = 0.f;
    bool bPulsingUp = true;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION()
    void OnAlertLevelChanged(EVPAlertLevel NewLevel);

public:
    void UpdateAlertVisual(EVPAlertLevel Level, float DeltaTime);
};