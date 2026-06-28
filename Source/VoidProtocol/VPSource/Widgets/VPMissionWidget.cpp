#include "VPSource/Widgets/VPMissionWidget.h"
#include "VPSource/VPGameState.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UVPMissionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Hide mission result elements immediately on construct
    if (MissionStatusBackground)
    {
        MissionStatusBackground->SetVisibility(ESlateVisibility::Hidden);
        MissionStatusBackground->SetColorAndOpacity(FLinearColor::Transparent);
    }

    if (MissionStatusText)
    {
        MissionStatusText->SetText(FText::GetEmpty());
        MissionStatusText->SetVisibility(ESlateVisibility::Hidden);
    }

    GameStateRef = GetWorld()->GetGameState<AVPGameState>();
    if (!GameStateRef)
    {
        // Retry after short delay — GameState might not be ready
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, [this]()
            {
                GameStateRef = GetWorld()->GetGameState<AVPGameState>();
                if (GameStateRef)
                {
                    GameStateRef->OnObjectiveCompleted.AddDynamic(
                        this, &UVPMissionWidget::OnObjectiveCompleted);
                    GameStateRef->OnMissionComplete.AddDynamic(
                        this, &UVPMissionWidget::OnMissionComplete);
                    GameStateRef->OnMissionFailed.AddDynamic(
                        this, &UVPMissionWidget::OnMissionFailed);
                    RefreshObjectiveList();
                }
            }, 0.5f, false);
        return;
    }

    GameStateRef->OnObjectiveCompleted.AddDynamic(
        this, &UVPMissionWidget::OnObjectiveCompleted);
    GameStateRef->OnMissionComplete.AddDynamic(
        this, &UVPMissionWidget::OnMissionComplete);
    GameStateRef->OnMissionFailed.AddDynamic(
        this, &UVPMissionWidget::OnMissionFailed);

    if (MissionStatusBackground)
        MissionStatusBackground->SetVisibility(ESlateVisibility::Hidden);

    RefreshObjectiveList();
}

void UVPMissionWidget::RefreshObjectiveList()
{
    if (!ObjectiveList || !GameStateRef) return;

    ObjectiveList->ClearChildren();

    for (const FVPObjective& Obj : GameStateRef->GetObjectives())
    {
        UTextBlock* ObjText = NewObject<UTextBlock>(this);
        if (!ObjText) continue;

        FString Label = Obj.bCompleted
            ? FString::Printf(TEXT("✓ %s"), *Obj.Description)
            : FString::Printf(TEXT("○ %s"), *Obj.Description);

        ObjText->SetText(FText::FromString(Label));
        ObjText->SetColorAndOpacity(FSlateColor(
            Obj.bCompleted
            ? FLinearColor(0.f, 1.f, 0.f, 1.f)   // green when done
            : FLinearColor(1.f, 1.f, 1.f, 0.7f)   // white when pending
        ));

        ObjectiveList->AddChildToVerticalBox(ObjText);
    }
}

void UVPMissionWidget::OnObjectiveCompleted(const FString& ObjectiveID)
{
    RefreshObjectiveList();
    UE_LOG(LogTemp, Warning, TEXT("MissionWidget: Objective %s completed"), *ObjectiveID);
}

void UVPMissionWidget::OnMissionComplete()
{
    ShowMissionResult("MISSION COMPLETE", FLinearColor(0.f, 1.f, 0.f, 1.f));
}

void UVPMissionWidget::OnMissionFailed()
{
    ShowMissionResult("MISSION FAILED", FLinearColor(1.f, 0.f, 0.f, 1.f));
}

void UVPMissionWidget::ShowMissionResult(const FString& Message, FLinearColor Color)
{
    if (MissionStatusText)
    {

        MissionStatusText->SetText(FText::FromString(Message));
        MissionStatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
        MissionStatusText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }

    if (MissionStatusBackground)
    {
        MissionStatusBackground->SetColorAndOpacity(Color);
        MissionStatusBackground->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}