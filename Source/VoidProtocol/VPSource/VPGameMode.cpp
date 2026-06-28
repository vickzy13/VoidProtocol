// VPGameMode.cpp
#include "VPSource/VPGameMode.h"
#include "VPSource/VPGameInstance.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Characters/VPHacker.h"
#include "VPSource/Characters/VPInfiltrator.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "VPSource/VPGameState.h"
#include "VPSource/Components/VPHealthComponent.h"

class UVPHealthComponent;

AVPGameMode::AVPGameMode()
{
    // We spawn pawns manually in PostLogin — no default pawn
    DefaultPawnClass = nullptr;
}

void AVPGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Guard — don't process same player twice
    if (PlayerRoles.Contains(NewPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("PostLogin: Player already registered, skipping"));
        return;
    }

    PlayerCount++;

    // Player 1 = Infiltrator, Player 2 = Hacker
    EVPRole AssignedRole = (PlayerCount == 1)
        ? EVPRole::Infiltrator
        : EVPRole::Hacker;

    TSubclassOf<APawn> PawnClass = (AssignedRole == EVPRole::Infiltrator)
        ? InfiltratorClass
        : HackerClass;

    if (!PawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: PawnClass not set in BP_VPGameMode for role %s"),
            *UEnum::GetValueAsString(AssignedRole));
        return;
    }

    // Destroy any auto-spawned default pawn
    if (APawn* OldPawn = NewPlayer->GetPawn())
    {
        UE_LOG(LogTemp, Warning, TEXT("PostLogin: Destroying old pawn %s"), *OldPawn->GetName());
        OldPawn->Destroy();
    }

    // Find spawn point
    AActor* StartSpot = FindPlayerStart(NewPlayer);
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: No PlayerStart found in level!"));
        return;
    }

    // Spawn correct role pawn
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
        UE_LOG(LogTemp, Error, TEXT("PostLogin: SpawnActor failed for %s"),
            *UEnum::GetValueAsString(AssignedRole));
        return;
    }

    // Possess and assign role
    NewPlayer->Possess(NewPawn);

    if (AVPCharacter* VPChar = Cast<AVPCharacter>(NewPawn))
    {
        VPChar->SetRole_Server(AssignedRole);

        // Store in map — used by RequestRespawn to restore correct role
        PlayerRoles.Add(NewPlayer, AssignedRole);
        // Reset input mode — main menu sets UIOnly which persists across travel
        NewPlayer->SetShowMouseCursor(false);
        NewPlayer->SetInputMode(FInputModeGameOnly());

        UE_LOG(LogTemp, Warning, TEXT("PostLogin: Player %d assigned as %s"),
            PlayerCount, *UEnum::GetValueAsString(AssignedRole));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: Cast to AVPCharacter failed — check Blueprint parent class"));
    }
}

void AVPGameMode::RequestRespawn(AController* DeadController)
{
    if (!DeadController) return;

    // Destroy dead pawn
    if (APawn* OldPawn = DeadController->GetPawn())
        OldPawn->Destroy();

    // Lookup role from map — pawn is gone so we can't read it from the character
    EVPRole* StoredRole = PlayerRoles.Find(DeadController);
    if (!StoredRole)
    {
        UE_LOG(LogTemp, Error, TEXT("RequestRespawn: No stored role for controller — falling back to RestartPlayer"));
        RestartPlayer(DeadController);
        return;
    }

    EVPRole RoleToRestore = *StoredRole;
    TSubclassOf<APawn> PawnClass = (RoleToRestore == EVPRole::Infiltrator)
        ? InfiltratorClass
        : HackerClass;

    // Respawn after delay
    FTimerHandle RespawnTimer;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindLambda([this, DeadController, PawnClass, RoleToRestore]()
    {
        if (!DeadController) return;

        AActor* StartSpot = FindPlayerStart(DeadController);
        if (!StartSpot || !PawnClass) return;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
            PawnClass,
            StartSpot->GetActorLocation(),
            StartSpot->GetActorRotation(),
            Params
        );

        if (!NewPawn) return;

        if (APlayerController* PC = Cast<APlayerController>(DeadController))
            PC->Possess(NewPawn);

        if (AVPCharacter* VPChar = Cast<AVPCharacter>(NewPawn))
        {
            VPChar->SetRole_Server(RoleToRestore);
            UE_LOG(LogTemp, Warning, TEXT("RequestRespawn: Respawned as %s"),
                *UEnum::GetValueAsString(RoleToRestore));
        }
    });

    GetWorldTimerManager().SetTimer(RespawnTimer, RespawnDelegate, RespawnDelay, false);
}

AActor* AVPGameMode::FindPlayerStart_Implementation(
    AController* Player, const FString& IncomingName)
{
    // Collect all PlayerStarts in the level
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        APlayerStart::StaticClass(), PlayerStarts);

    if (PlayerStarts.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FindPlayerStart: No PlayerStart actors found!"));
        return Super::FindPlayerStart_Implementation(Player, IncomingName);
    }

    // Pick based on PlayerCount — player 1 gets first start, player 2 gets second
    int32 Index = FMath::Clamp(PlayerCount - 1, 0, PlayerStarts.Num() - 1);
    UE_LOG(LogTemp, Warning, TEXT("FindPlayerStart: Using PlayerStart index %d"), Index);
    return PlayerStarts[Index];
}

void AVPGameMode::ReportSuspicious()
{
    if (AVPGameState* GS = GetGameState<AVPGameState>())
    {
        // Only escalate — never downgrade via this call
        if (GS->GetAlertLevel() == EVPAlertLevel::Unaware)
            GS->SetAlertLevel(EVPAlertLevel::Suspicious);
    }
}

void AVPGameMode::ReportAlerted()
{
    if (AVPGameState* GS = GetGameState<AVPGameState>())
        GS->SetAlertLevel(EVPAlertLevel::Alerted);
}

void AVPGameMode::ReportAllClear()
{
    if (AVPGameState* GS = GetGameState<AVPGameState>())
        GS->SetAlertLevel(EVPAlertLevel::Unaware);
}