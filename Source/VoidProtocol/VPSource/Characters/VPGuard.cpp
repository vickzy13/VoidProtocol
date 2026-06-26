#include "Net/UnrealNetwork.h"
#include "VPSource/Characters/VPGuard.h"
#include "VPSource/AI/VPGuardController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AVPGuard::AVPGuard()
{
    PrimaryActorTick.bCanEverTick = true; // ← confirm this is true
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AVPGuardController::StaticClass();
    bReplicates = true;
    GetCharacterMovement()->SetIsReplicated(true);
}

void AVPGuard::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVPGuard, bIsUnconscious);
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

void AVPGuard::SetUnconscious(bool bUnconscious)
{
    if (!HasAuthority()) return;

    bIsUnconscious = bUnconscious;
    OnRep_IsUnconscious();

    if (bUnconscious)
    {
        // Disable AI
        if (AVPGuardController* GC = Cast<AVPGuardController>(GetController()))
            GC->StopMovement();

        // Disable collision
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->DisableMovement();

        // Wake up after 60 seconds
        GetWorldTimerManager().SetTimer(RecoverTimer, [this]()
            {
                SetUnconscious(false);
            }, 60.f, false);

        UE_LOG(LogTemp, Warning, TEXT("Guard %s unconscious for 60s"), *GetName());
    }
    else
    {
        // Re-enable AI
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        if (AVPGuardController* GC = Cast<AVPGuardController>(GetController()))
            GC->RunBehaviorTree(GC->GetGuardBehaviorTree());

        UE_LOG(LogTemp, Warning, TEXT("Guard %s recovered"), *GetName());
    }
}

void AVPGuard::OnRep_IsUnconscious()
{
    if (bIsUnconscious)
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->DisableMovement();
    }
    else
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
}