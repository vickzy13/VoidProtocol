#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPHackProgressWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UVPHackerComponent;

UCLASS()
class VOIDPROTOCOL_API UVPHackProgressWidget : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HackProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HackLabel;

    UVPHackerComponent* HackerComponent = nullptr;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    void SetHackerComponent(UVPHackerComponent* Component);
};