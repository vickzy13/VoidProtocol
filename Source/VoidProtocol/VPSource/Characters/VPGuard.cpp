#include "VPSource/Characters/VPGuard.h"
#include "Net/UnrealNetwork.h"
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
    OnRep_IsUnconscious(); // handles collision + animation

    if (bUnconscious)
    {
        // Stop AI
        if (AVPGuardController* GC = Cast<AVPGuardController>(GetController()))
        {
            GC->StopMovement();
            GC->ClearAllDetection();
        }

        // Don't touch collision here — OnRep handles it
        GetCharacterMovement()->DisableMovement();

        GetWorldTimerManager().SetTimer(RecoverTimer, [this]()
            {
                SetUnconscious(false);
            }, 60.f, false);
    }
    else
    {
        // Recovery — OnRep handles collision restore
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        if (AVPGuardController* GC = Cast<AVPGuardController>(GetController()))
            GC->RunBehaviorTree(GC->GetGuardBehaviorTree());
    }
}

void AVPGuard::OnRep_IsUnconscious()
{
    if (bIsUnconscious)
    {
        // Fix rotation before playing knockout
        // Ensure guard mesh is correctly oriented
        FRotator CurrentRot = GetActorRotation();
        SetActorRotation(FRotator(0.f, CurrentRot.Yaw, 0.f)); // zero out pitch/roll

        if (KnockoutMontage)
        {
            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
            if (AnimInstance)
                AnimInstance->Montage_Play(KnockoutMontage, 1.0f);
        }

        GetCapsuleComponent()->SetCollisionResponseToChannel(
            ECC_Pawn, ECR_Ignore);
        GetCapsuleComponent()->SetGenerateOverlapEvents(true);
        GetCharacterMovement()->DisableMovement();
    }
    else
    {
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics);
        GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }
}