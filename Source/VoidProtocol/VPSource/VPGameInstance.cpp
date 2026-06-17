// VPGameInstance.cpp
#include "VPSource/VPGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"

UVPGameInstance::UVPGameInstance()
{
    OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
        this, &UVPGameInstance::OnCreateSessionComplete);

    OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(
        this, &UVPGameInstance::OnFindSessionsComplete);

    OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
        this, &UVPGameInstance::OnJoinSessionComplete);
}

//=============================================================
// ENGINE OVERRIDES
//=============================================================

void UVPGameInstance::Init()
{
    Super::Init();

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS)
    {
        UE_LOG(LogTemp, Error, TEXT("VPGameInstance: No Online Subsystem found!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("VPGameInstance: OSS = %s"),
        *OSS->GetSubsystemName().ToString());

    SessionInterface = OSS->GetSessionInterface();
}

void UVPGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    Super::OnWorldChanged(OldWorld, NewWorld);

    if (!NewWorld) return;

    // Refresh session interface after level travel
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS)
    {
        SessionInterface = OSS->GetSessionInterface();
        UE_LOG(LogTemp, Warning, TEXT("VPGameInstance: World changed - session interface refreshed"));
    }
}

//=============================================================
// HOST
//=============================================================

void UVPGameInstance::HostSession()
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("HostSession: SessionInterface not valid"));
        return;
    }

    // Destroy any existing session first
    if (SessionInterface->GetNamedSession(NAME_GameSession))
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }

    bSessionActive = false; // reset travel guard

    CreateSessionHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        OnCreateSessionCompleteDelegate);

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = bIsLAN;            // LAN — swap to false for EOS/Steam
    SessionSettings.NumPublicConnections = MaxPlayers;
    SessionSettings.bShouldAdvertise = true;       // visible to FindSessions
    SessionSettings.bAllowJoinInProgress = true;   // allow client to join after host travels
    SessionSettings.bAllowJoinViaPresence = false; // presence = EOS/Steam only
    SessionSettings.bUsesPresence = !bIsLAN;         // presence = EOS/Steam only
    SessionSettings.Set(FName("PORT"), 7777,
        EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

    const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
    SessionInterface->CreateSession(
        *LocalPlayer->GetPreferredUniqueNetId(),
        NAME_GameSession,
        SessionSettings
    );

    UE_LOG(LogTemp, Warning, TEXT("VPGameInstance: Creating session..."));
    OnSessionStatusChanged.Broadcast("Creating session...");
}

void UVPGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);

    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("OnCreateSessionComplete: Failed"));
        OnSessionStatusChanged.Broadcast("Failed to create session");
        return;
    }

    // Confirm session actually exists
    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (Session)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session state: %s"),
            EOnlineSessionState::ToString(Session->SessionState));
    }

    // Travel to gameplay level — session persists across travel for NULL subsystem
    FString LevelPath = GameplayLevel.GetAssetPathString();
    int32 DotIndex;
    if (LevelPath.FindLastChar('.', DotIndex))
        LevelPath = LevelPath.Left(DotIndex);

    if (LevelPath.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("GameplayLevel not set in BP_VPGameInstance!"));
        OnSessionStatusChanged.Broadcast("Error: Gameplay level not set");
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Host travelling to: %s"), *LevelPath);
    OnSessionStatusChanged.Broadcast("Loading mission...");
    GetWorld()->ServerTravel(LevelPath + "?listen");
}

//=============================================================
// JOIN — with retry logic
//=============================================================

void UVPGameInstance::FindAndJoinSession()
{
    UE_LOG(LogTemp, Warning, TEXT("FindAndJoinSession called"));

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface not valid"));
        return;
    }

    if (SessionInterface->GetNamedSession(NAME_GameSession))
    {
        UE_LOG(LogTemp, Warning, TEXT("Existing session found - destroying first"));

        FDelegateHandle DestroyHandle;
        DestroyHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateLambda(
                [this, DestroyHandle](FName SessionName, bool bWasSuccessful) mutable
                {
                    UE_LOG(LogTemp, Warning, TEXT("Session destroyed: %s - starting search"),
                        bWasSuccessful ? TEXT("YES") : TEXT("NO"));
                    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);

                    FTimerHandle Delay;
                    GetTimerManager().SetTimer(Delay, [this]()
                        {
                            DoFindSessions();
                        }, 1.f, false);
                }));

        SessionInterface->DestroySession(NAME_GameSession);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No existing session - searching immediately"));
        SearchRetryCount = 0;

        FTimerHandle InitialDelay;
        GetTimerManager().SetTimer(InitialDelay, [this]()
            {
                DoFindSessions();
            }, SearchRetryDelay, false);
    }
}

void UVPGameInstance::DoFindSessions()
{
    if (!SessionInterface.IsValid()) return;

    UE_LOG(LogTemp, Warning, TEXT("Search attempt %d / %d"),
        SearchRetryCount + 1, MaxSearchRetries);

    FindSessionsHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        OnFindSessionsCompleteDelegate);

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = bIsLAN;       // LAN — swap to false for EOS/Steam
    SessionSearch->MaxSearchResults = 10;
    // No QuerySettings needed for NULL subsystem

    const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
    SessionInterface->FindSessions(
        *LocalPlayer->GetPreferredUniqueNetId(),
        SessionSearch.ToSharedRef()
    );
}

void UVPGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    // Always clear the delegate first
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);

    int32 ResultCount = SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : -1;

    UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete: Success=%s, Results=%d"),
        bWasSuccessful ? TEXT("YES") : TEXT("NO"),
        ResultCount);

    if (bWasSuccessful && ResultCount > 0)
    {
        // Session found — join it
        UE_LOG(LogTemp, Warning, TEXT("Found session! Joining..."));
        OnSessionStatusChanged.Broadcast("Found session! Joining...");

        JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
            OnJoinSessionCompleteDelegate);

        const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
        SessionInterface->JoinSession(
            *LocalPlayer->GetPreferredUniqueNetId(),
            NAME_GameSession,
            SessionSearch->SearchResults[0]
        );
    }
    else
    {
        // Not found — retry up to MaxSearchRetries
        SearchRetryCount++;

        if (SearchRetryCount < MaxSearchRetries)
        {
            UE_LOG(LogTemp, Warning, TEXT("No session found - retry %d/%d in %.0fs"),
                SearchRetryCount, MaxSearchRetries, SearchRetryDelay);

            OnSessionStatusChanged.Broadcast(FString::Printf(
                TEXT("Searching... (%d/%d)"), SearchRetryCount, MaxSearchRetries));

            FTimerHandle RetryHandle;
            GetTimerManager().SetTimer(RetryHandle, [this]()
            {
                DoFindSessions();
            }, SearchRetryDelay, false);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Max retries reached - no session found"));
            OnSessionStatusChanged.Broadcast("No session found. Is the host running?");
            SearchRetryCount = 0;
        }
    }
}
FString UVPGameInstance::ResolveConnectionURL(const FString& RawURL)
{
    ISocketSubsystem* SocketSub = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSub) return RawURL;

    TArray<TSharedPtr<FInternetAddr>> LocalAddresses;
    SocketSub->GetLocalAdapterAddresses(LocalAddresses);

    for (const TSharedPtr<FInternetAddr>& Addr : LocalAddresses)
    {
        if (!Addr.IsValid()) continue;

        FString LocalIP = Addr->ToString(false);
        if (RawURL.Contains(LocalIP))
        {
            FString LoopbackURL = RawURL.Replace(*LocalIP, TEXT("127.0.0.1"));
            UE_LOG(LogTemp, Warning, TEXT("Same machine detected — %s → %s"),
                *RawURL, *LoopbackURL);
            return LoopbackURL;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Different machine — using URL as-is: %s"), *RawURL);
    return RawURL;
}

void UVPGameInstance::OnJoinSessionComplete(
    FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join - Result: %d"), (int32)Result);
        OnSessionStatusChanged.Broadcast("Failed to join session");
        return;
    }

    FString TravelURL;
    if (SessionInterface->GetResolvedConnectString(NAME_GameSession, TravelURL))
    {
        UE_LOG(LogTemp, Warning, TEXT("Raw resolved URL: %s"), *TravelURL);

        // Fix same machine IP
        TravelURL = ResolveConnectionURL(TravelURL);

        // Fix port if it resolved to 0
        if (TravelURL.EndsWith(TEXT(":0")))
        {
            TravelURL = TravelURL.Replace(TEXT(":0"), TEXT(":7777"));
            UE_LOG(LogTemp, Warning, TEXT("Fixed port 0 → 7777: %s"), *TravelURL);
        }

        UE_LOG(LogTemp, Warning, TEXT("Final ClientTravel URL: %s"), *TravelURL);
        OnSessionStatusChanged.Broadcast("Connecting...");

        if (APlayerController* PC = GetFirstLocalPlayerController())
            PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
    }
}
//=============================================================
// CALLED BY GAMEMODE
//=============================================================

void UVPGameInstance::OnAllPlayersJoined()
{
    // This is now unused since host travels in OnCreateSessionComplete
    // Kept for future use (e.g. lobby system)
    UE_LOG(LogTemp, Warning, TEXT("OnAllPlayersJoined: All %d players in gameplay level"), MaxPlayers);
}
