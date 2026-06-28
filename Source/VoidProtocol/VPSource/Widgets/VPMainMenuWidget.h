// VPMainMenuWidget.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VPMainMenuWidget.generated.h"

// Forward declarations
class UButton;
class UTextBlock;
class UVPGameInstance;

UCLASS()
class VOIDPROTOCOL_API UVPMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — bound widgets, must match names in WBP exactly
    // meta=(BindWidget) links C++ var to Blueprint widget
    //=========================================================
private:
    UPROPERTY(meta = (BindWidget))
    UButton* HostButton;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatusText;

    //=========================================================
    // PRIVATE — internal helpers
    //=========================================================
private:
    UVPGameInstance* GetVPGameInstance() const;

    UFUNCTION()
    void OnHostClicked();

    UFUNCTION()
    void OnJoinClicked();

    //=========================================================
    // PROTECTED — UUserWidget overrides
    //=========================================================
protected:
    virtual void NativeConstruct() override;

    //=========================================================
    // PUBLIC — callable from Blueprint if needed
    //=========================================================
public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetStatusText(const FString& Text);
};