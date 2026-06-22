// VPDetectionWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPDetectionWidget.generated.h"

class UImage;
class AVPCharacter;

UCLASS()
class VOIDPROTOCOL_API UVPDetectionWidget : public UUserWidget
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — bound widget, name must match WBP exactly
    //=========================================================
private:
    UPROPERTY(meta = (BindWidget))
    UImage* DetectionArc;

    AVPCharacter* OwningCharacter = nullptr;

    void UpdateDetectionVisual();

    //=========================================================
    // PROTECTED — overrides
    //=========================================================
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};