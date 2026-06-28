// Fill out your copyright notice in the Description page of Project Settings.


#include "VPSource/Widgets/VPMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "VPSource/VPGameInstance.h"

void UVPMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button clicks — all logic stays here in C++
    if (HostButton)
        HostButton->OnClicked.AddDynamic(this, &UVPMainMenuWidget::OnHostClicked);

    if (JoinButton)
        JoinButton->OnClicked.AddDynamic(this, &UVPMainMenuWidget::OnJoinClicked);

    // Listen to GameInstance session status updates
    if (UVPGameInstance* GI = GetVPGameInstance())
        GI->OnSessionStatusChanged.AddDynamic(this, &UVPMainMenuWidget::SetStatusText);

    SetStatusText("Ready");
}

UVPGameInstance* UVPMainMenuWidget::GetVPGameInstance() const
{
    return GetGameInstance<UVPGameInstance>();
}

void UVPMainMenuWidget::OnHostClicked()
{
    SetStatusText("Creating session...");

    if (UVPGameInstance* GI = GetVPGameInstance())
        GI->HostSession();
}

void UVPMainMenuWidget::OnJoinClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Join button clicked"));
    SetStatusText("Connecting...");

    if (UVPGameInstance* GI = GetVPGameInstance())
        GI->FindAndJoinSession();
    else
        UE_LOG(LogTemp, Error, TEXT("OnJoinClicked: GameInstance is null"));
}

void UVPMainMenuWidget::SetStatusText(const FString& Text)
{
    if (StatusText)
        StatusText->SetText(FText::FromString(Text));
}