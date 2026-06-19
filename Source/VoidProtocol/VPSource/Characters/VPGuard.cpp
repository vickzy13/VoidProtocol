// VPGuard.cpp
#include "VPSource/Characters/VPGuard.h"
#include "VPSource/AI/VPGuardController.h"
#include "GameFramework/CharacterMovementComponent.h"

AVPGuard::AVPGuard()
{
    PrimaryActorTick.bCanEverTick = true; // ← confirm this is true
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AVPGuardController::StaticClass();
    bReplicates = true;
    GetCharacterMovement()->SetIsReplicated(true);
}

void AVPGuard::BeginPlay()
{
    Super::BeginPlay();
}

void AVPGuard::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AnimGroundVelocity = GetVelocity();
}

AActor* AVPGuard::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return nullptr;

    AActor* Point = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return Point;
}