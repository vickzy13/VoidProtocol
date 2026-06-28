#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPCameraViewWidget.generated.h"

class UImage;
class UTextBlock;
class UVPHackerComponent;

UCLASS()
class VOIDPROTOCOL_API UVPCameraViewWidget : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UImage* ScanlineOverlay;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CameraLabel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ExitPrompt;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* NextCameraPrompt;

    UVPHackerComponent* HackerComponent = nullptr;

    float ScanlineTimer = 0.f;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    void SetHackerComponent(UVPHackerComponent* Component);
    void UpdateCameraInfo(const FString& CameraName, int32 AdjacentCount);
};