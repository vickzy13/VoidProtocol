// Fill out your copyright notice in the Description page of Project Settings.


#include "VPSource/VPGameMode.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Characters/VPHacker.h"
#include "VPSource/Characters/VPInfiltrator.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EngineUtils.h"

AVPGameMode::AVPGameMode()
{
    // Default pawn set to none — we spawn manually in PostLogin
    DefaultPawnClass = nullptr;
}

void AVPGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Guard — skip if already registered
    if (PlayerRoles.Contains(NewPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("PostLogin called again for existing player — skipping"));
        return;
    }

    PlayerCount++;

    EVPRole AssignedRole = (PlayerCount == 1)
        ? EVPRole::Infiltrator
        : EVPRole::Hacker;

    TSubclassOf<APawn> PawnClass = (AssignedRole == EVPRole::Infiltrator)
        ? InfiltratorClass
        : HackerClass;

    if (!PawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGameMode: PawnClass not assigned!"));
        return;
    }

    if (APawn* OldPawn = NewPlayer->GetPawn())
        OldPawn->Destroy();

    AActor* StartSpot = FindPlayerStart(NewPlayer);
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGameMode: No PlayerStart found!"));
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
        PawnClass,
        StartSpot->GetActorLocation(),
        StartSpot->GetActorRotation(),
        Params
    );

    if (!NewPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGameMode: Failed to spawn pawn!"));
        return;
    }

    NewPlayer->Possess(NewPawn);

    if (AVPCharacter* VPChar = Cast<AVPCharacter>(NewPawn))
    {
        VPChar->SetRole_Server(AssignedRole);

        // ← THIS is the missing line — store role against controller
        PlayerRoles.Add(NewPlayer, AssignedRole);

        UE_LOG(LogTemp, Warning, TEXT("Player %d spawned as: %s"),
            PlayerCount,
            *UEnum::GetValueAsString(AssignedRole));
    }
}

void AVPGameMode::RequestRespawn(AController* DeadController)
{
    if (!DeadController) return;

    // Destroy the dead pawn
    if (APawn* OldPawn = DeadController->GetPawn())
    {
        OldPawn->Destroy();
    }

    // Respawn after delay — lambda captures controller safely
    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindLambda([this, DeadController]()
        {
            if (!DeadController) return;

            // Re-run PostLogin role assignment logic
            if (APlayerController* PC = Cast<APlayerController>(DeadController))
            {
                // Determine role from PlayerCount position
                // Player 1 controller always gets Infiltrator back
                TSubclassOf<APawn> PawnClass = nullptr;

                if (AVPCharacter* OldChar = Cast<AVPCharacter>(PC->GetPawn()))
                {
                    PawnClass = (OldChar->GetRole() == EVPRole::Infiltrator)
                        ? InfiltratorClass
                        : HackerClass;
                }
                else
                {
                    // Fallback — just restart normally
                    RestartPlayer(DeadController);
                    return;
                }

                AActor* StartSpot = FindPlayerStart(PC);
                if (!StartSpot) return;

                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
                    PawnClass,
                    StartSpot->GetActorLocation(),
                    StartSpot->GetActorRotation(),
                    Params
                );

                if (NewPawn)
                {
                    PC->Possess(NewPawn);

                    if (AVPCharacter* VPChar = Cast<AVPCharacter>(NewPawn))
                    {
                        // Restore same role on respawn
                        EVPRole RestoredRole = (PawnClass == InfiltratorClass)
                            ? EVPRole::Infiltrator
                            : EVPRole::Hacker;
                        VPChar->SetRole_Server(RestoredRole);

                        UE_LOG(LogTemp, Warning, TEXT("Player respawned as: %s"),
                            *UEnum::GetValueAsString(RestoredRole));
                    }
                }
            }
        });

    GetWorldTimerManager().SetTimer(
        RespawnTimer,
        RespawnDelegate,
        RespawnDelay,
        false
    );
}

