// Fill out your copyright notice in the Description page of Project Settings.


#include "VPSource/VPMainMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AVPMainMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("MainMenuGameMode BeginPlay fired"));

    if (!MainMenuWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MainMenuWidgetClass is null — assign WBP_MainMenu in BP_VPMainMenuGameMode"));
        return;
    }

    MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
    if (MainMenuWidget)
    {
        MainMenuWidget->AddToViewport();
        UE_LOG(LogTemp, Warning, TEXT("Main menu added to viewport"));

        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->SetShowMouseCursor(true);
            PC->SetInputMode(FInputModeUIOnly());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CreateWidget failed"));
    }
}