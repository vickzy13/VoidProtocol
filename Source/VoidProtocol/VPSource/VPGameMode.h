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
private:
    int32 PlayerCount = 0;
    TMap<APlayerController*, EVPRole> PlayerRoles;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Roles")
    TSubclassOf<APawn> InfiltratorClass;

    UPROPERTY(EditDefaultsOnly, Category = "Roles")
    TSubclassOf<APawn> HackerClass;

    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    float RespawnDelay = 5.f;

public:
    AVPGameMode();

    // Called automatically by engine when a player joins
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // Called by UVPHealthComponent::OnDeath delegate
    void RequestRespawn(AController* DeadController);
};