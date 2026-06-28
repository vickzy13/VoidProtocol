// VPGameInstance.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SocketSubsystem.h"
#include "VPGameInstance.generated.h"

UCLASS()
class VOIDPROTOCOL_API UVPGameInstance : public UGameInstance
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — session interface, delegates, handles
    //=========================================================
private:
    IOnlineSessionPtr SessionInterface;

    FOnCreateSessionCompleteDelegate  OnCreateSessionCompleteDelegate;
    FOnFindSessionsCompleteDelegate   OnFindSessionsCompleteDelegate;
    FOnJoinSessionCompleteDelegate    OnJoinSessionCompleteDelegate;

    FDelegateHandle CreateSessionHandle;
    FDelegateHandle FindSessionsHandle;
    FDelegateHandle JoinSessionHandle;

    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    // Callbacks — private, only called by OSS internally
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    FString ResolveConnectionURL(const FString& RawURL);
    // Internal search helper with retry logic
    void DoFindSessions();

    // Session state
    bool bSessionActive = false;

    // Search retry config
    int32 SearchRetryCount = 0;

    UPROPERTY(EditDefaultsOnly, Category="Session", meta=(AllowPrivateAccess="true"))
    int32 MaxSearchRetries = 5;

    UPROPERTY(EditDefaultsOnly, Category="Session", meta=(AllowPrivateAccess="true"))
    float SearchRetryDelay = 2.f;

    UPROPERTY(EditDefaultsOnly, Category="Session", meta=(AllowPrivateAccess="true"))
    int32 MaxPlayers = 2;

    UPROPERTY(EditDefaultsOnly, Category = "Session", meta = (AllowPrivateAccess = "true"))
    bool bIsLAN = true; // true = NULL/LAN, false = EOS/Steam

    //=========================================================
    // PROTECTED — engine overrides
    //=========================================================
protected:
    virtual void Init() override;
    virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

    //=========================================================
    // PUBLIC — UI callable, GameMode callable, config
    //=========================================================
public:
    UVPGameInstance();

    // Session status delegate — UI binds to this
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStatusChanged, const FString&, Status);

    UPROPERTY(BlueprintAssignable, Category="Session")
    FOnSessionStatusChanged OnSessionStatusChanged;

    // Called by Host button in WBP_MainMenu
    UFUNCTION(BlueprintCallable, Category="Session")
    void HostSession();

    // Called by Join button in WBP_MainMenu
    UFUNCTION(BlueprintCallable, Category="Session")
    void FindAndJoinSession();

    // Called by VPGameMode::PostLogin when all players connected
    void OnAllPlayersJoined();

    // Called by VPGameMode to check how many players to wait for
    int32 GetMaxPlayers() const { return MaxPlayers; }

    // Gameplay level to travel to — set in BP_VPGameInstance details panel
    UPROPERTY(EditDefaultsOnly, Category="Session")
    FSoftObjectPath GameplayLevel;
};
