#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VPHackable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UVPHackable : public UInterface
{
    GENERATED_BODY()
};

class VOIDPROTOCOL_API IVPHackable
{
    GENERATED_BODY()

public:
    // Can this object be hacked right now?
    virtual bool CanBeHacked() const = 0;

    // Called when hack starts — show progress bar
    virtual void OnHackStarted(AActor* Hacker) = 0;

    // Called when hack completes
    virtual void OnHackCompleted(AActor* Hacker) = 0;

    // Called if hack is interrupted
    virtual void OnHackCancelled() = 0;

    // Get location for progress bar widget
    virtual FVector GetHackWidgetLocation() const = 0;

    // For camera jumping — returns adjacent hackable cameras
    virtual TArray<AActor*> GetAdjacentHackables() const
    {
        return TArray<AActor*>();
    }
};