// VPGuard.cpp
#include "VPSource/Characters/VPGuard.h"
#include "VPSource/AI/VPGuardController.h"
#include "GameFramework/CharacterMovementComponent.h"

AVPGuard::AVPGuard()
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AVPGuardController::StaticClass();

    // Replicate to all clients so they see guard moving
    bReplicates = true;
    GetCharacterMovement()->SetIsReplicated(true);
}

void AVPGuard::BeginPlay()
{
    Super::BeginPlay();
}

AActor* AVPGuard::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0) return nullptr;

    AActor* Point = PatrolPoints[CurrentPatrolIndex];
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return Point;
}