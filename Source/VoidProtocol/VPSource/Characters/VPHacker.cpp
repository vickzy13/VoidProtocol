// Fill out your copyright notice in the Description page of Project Settings.


#include "VPSource/Characters/VPHacker.h"
#include "GameFramework/CharacterMovementComponent.h"

AVPHacker::AVPHacker()
{
    // Hacker stats — slower, fragile, quiet
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 150.f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    VPRole = EVPRole::Hacker;
}