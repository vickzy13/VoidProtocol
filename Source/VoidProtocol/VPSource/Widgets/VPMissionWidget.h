#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPSource/VPGameState.h"
#include "VPMissionWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UImage;

UCLASS()
class VOIDPROTOCOL_API UVPMissionWidget : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* ObjectiveList;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MissionStatusText;

    UPROPERTY(meta = (BindWidget))
    UImage* MissionStatusBackground;

    AVPGameState* GameStateRef = nullptr;

    UFUNCTION()
    void OnObjectiveCompleted(const FString& ObjectiveID);

    UFUNCTION()
    void OnMissionComplete();

    UFUNCTION()
    void OnMissionFailed();

    void RefreshObjectiveList();
    void ShowMissionResult(const FString& Message, FLinearColor Color);

protected:
    virtual void NativeConstruct() override;
};