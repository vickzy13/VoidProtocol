// VPGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPGameMode.generated.h"

UCLASS()
class VOIDPROTOCOL_API AVPGameMode : public AGameModeBase
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — player tracking, not needed outside GameMode
    //=========================================================
private:
    int32 PlayerCount = 0;
    TMap<AController*, EVPRole> PlayerRoles;

    //=========================================================
    // PROTECTED — configurable in BP_VPGameMode details panel
    //=========================================================
protected:
    UPROPERTY(EditDefaultsOnly, Category="Roles")
    TSubclassOf<APawn> InfiltratorClass;

    UPROPERTY(EditDefaultsOnly, Category="Roles")
    TSubclassOf<APawn> HackerClass;

    UPROPERTY(EditDefaultsOnly, Category="Respawn")
    float RespawnDelay = 5.f;

    //=========================================================
    // PUBLIC — engine overrides + called by other systems
    //=========================================================
public:
    AVPGameMode();

    // Engine calls this when a player joins
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // Called by UVPHealthComponent OnDeath delegate (server only)
    void RequestRespawn(AController* DeadController);

    virtual AActor* FindPlayerStart_Implementation(
        AController* Player, const FString& IncomingName = TEXT("")) override;

    void ReportSuspicious();
    void ReportAlerted();
    void ReportAllClear();
};
