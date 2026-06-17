// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPHacker.generated.h"

UCLASS()
class VOIDPROTOCOL_API AVPHacker : public AVPCharacter
{
    GENERATED_BODY()

public:
    AVPHacker();

    // Hack range — used on Day 9
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hacker")
    float HackRange = 1500.f;
};
