// VPMainMenuGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VPMainMenuGameMode.generated.h"

UCLASS()
class VOIDPROTOCOL_API AVPMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — the widget class to spawn, set in BP
    //=========================================================
private:
    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UUserWidget* MainMenuWidget = nullptr;

    //=========================================================
    // PROTECTED — override
    //=========================================================
protected:
    virtual void BeginPlay() override;
};