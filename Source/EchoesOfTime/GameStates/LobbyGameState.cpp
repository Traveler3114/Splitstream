#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GameFramework/GameMode.h"
#include "DefaultPlayerState.h"

DEFINE_LOG_CATEGORY(LogLobby);

ALobbyGameState::ALobbyGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALobbyGameState, LobbyPhase);
    DOREPLIFETIME(ALobbyGameState, CountdownRemaining);
    DOREPLIFETIME(ALobbyGameState, bAllPlayersReady);
    DOREPLIFETIME(ALobbyGameState, PlayerRoster);
}

void ALobbyGameState::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        UE_LOG(LogLobby, Log, TEXT("LobbyGameState initialized - Phase: Gathering"));
        SetLobbyPhase(ELobbyPhase::Gathering);
    }
}

void ALobbyGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle countdown on server
    if (HasAuthority() && bCountdownActive && LobbyPhase == ELobbyPhase::Countdown)
    {
        CountdownTimer -= DeltaTime;
        CountdownRemaining = FMath::Max(0.0f, CountdownTimer);

        if (CountdownRemaining <= 0.0f)
        {
            // Countdown finished - transition to Locked then start game
            bCountdownActive = false;
            SetLobbyPhase(ELobbyPhase::Locked);
            
            UE_LOG(LogLobby, Log, TEXT("Countdown finished - Starting game"));
            
            // Trigger game start in GameMode
            if (AGameMode* GameMode = GetWorld()->GetAuthGameMode())
            {
                // Call StartGame on GameMode (will be implemented there)
                if (GameMode->GetClass()->FindFunctionByName(TEXT("StartGame")))
                {
                    GameMode->ProcessEvent(GameMode->GetClass()->FindFunctionByName(TEXT("StartGame")), nullptr);
                }
            }
        }
    }
}

void ALobbyGameState::OnRep_LobbyPhase()
{
    UE_LOG(LogLobby, Log, TEXT("Lobby phase changed to: %d"), (int32)LobbyPhase);
    OnLobbyPhaseChanged(LobbyPhase);
}

void ALobbyGameState::OnRep_CountdownRemaining()
{
    OnCountdownUpdate(CountdownRemaining);
}

void ALobbyGameState::OnRep_AllPlayersReady()
{
    UE_LOG(LogLobby, Log, TEXT("All players ready state changed: %s"), bAllPlayersReady ? TEXT("True") : TEXT("False"));
}

void ALobbyGameState::AddPlayerToRoster(ADefaultPlayerState* PlayerState, int32 PlatformIndex)
{
    if (!HasAuthority() || !PlayerState)
    {
        return;
    }

    const FString PlayerId = PlayerState->GetUniqueId().ToString();
    
    // Check if player already exists
    if (FindPlayerInRoster(PlayerId))
    {
        UE_LOG(LogLobby, Warning, TEXT("Player %s already in roster"), *PlayerId);
        return;
    }

    FLobbyPlayerRepData NewPlayerData;
    NewPlayerData.PlayerId = PlayerId;
    NewPlayerData.DisplayName = FText::FromString(PlayerState->GetPlayerName());
    NewPlayerData.PlatformIndex = PlatformIndex;
    NewPlayerData.bReady = false;
    NewPlayerData.TeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Future")); // Default team
    NewPlayerData.RoleTag = FGameplayTag::EmptyTag; // Placeholder
    NewPlayerData.AvatarId = 0; // Placeholder
    
    // First player becomes host
    NewPlayerData.bIsHost = (PlayerRoster.Items.Num() == 0);

    PlayerRoster.Items.Add(NewPlayerData);
    PlayerRoster.MarkItemDirty(PlayerRoster.Items.Last());

    UE_LOG(LogLobby, Log, TEXT("Added player %s to roster at platform %d (Host: %s)"), 
           *PlayerState->GetPlayerName(), PlatformIndex, NewPlayerData.bIsHost ? TEXT("Yes") : TEXT("No"));

    EvaluateReadinessAndPhase();
    OnPlayerRosterChanged();
}

void ALobbyGameState::RemovePlayerFromRoster(const FString& PlayerId)
{
    if (!HasAuthority())
    {
        return;
    }

    for (int32 i = PlayerRoster.Items.Num() - 1; i >= 0; --i)
    {
        if (PlayerRoster.Items[i].PlayerId == PlayerId)
        {
            int32 PlatformIndex = PlayerRoster.Items[i].PlatformIndex;
            bool bWasHost = PlayerRoster.Items[i].bIsHost;
            
            PlayerRoster.Items.RemoveAt(i);
            PlayerRoster.MarkArrayDirty();

            UE_LOG(LogLobby, Log, TEXT("Removed player %s from roster (Platform: %d)"), *PlayerId, PlatformIndex);

            // If host left and there are still players, promote first player to host
            if (bWasHost && PlayerRoster.Items.Num() > 0)
            {
                PlayerRoster.Items[0].bIsHost = true;
                PlayerRoster.MarkItemDirty(PlayerRoster.Items[0]);
                UE_LOG(LogLobby, Log, TEXT("Promoted player %s to host"), *PlayerRoster.Items[0].DisplayName.ToString());
            }

            EvaluateReadinessAndPhase();
            OnPlayerRosterChanged();
            break;
        }
    }
}

void ALobbyGameState::UpdatePlayerRosterEntry(const FString& PlayerId, const FGameplayTag& TeamTag, bool bReady)
{
    if (!HasAuthority())
    {
        return;
    }

    FLobbyPlayerRepData* PlayerData = FindPlayerInRoster(PlayerId);
    if (PlayerData)
    {
        bool bChanged = false;
        
        if (PlayerData->TeamTag != TeamTag)
        {
            PlayerData->TeamTag = TeamTag;
            bChanged = true;
            UE_LOG(LogLobby, Log, TEXT("Player %s changed team to %s"), *PlayerData->DisplayName.ToString(), *TeamTag.ToString());
        }
        
        if (PlayerData->bReady != bReady)
        {
            PlayerData->bReady = bReady;
            bChanged = true;
            UE_LOG(LogLobby, Log, TEXT("Player %s ready state: %s"), *PlayerData->DisplayName.ToString(), bReady ? TEXT("Ready") : TEXT("Not Ready"));
        }

        if (bChanged)
        {
            PlayerRoster.MarkItemDirty(*PlayerData);
            EvaluateReadinessAndPhase();
        }
    }
}

void ALobbyGameState::UpdatePlayerPlatformIndex(const FString& PlayerId, int32 NewPlatformIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    FLobbyPlayerRepData* PlayerData = FindPlayerInRoster(PlayerId);
    if (PlayerData && PlayerData->PlatformIndex != NewPlatformIndex)
    {
        int32 OldIndex = PlayerData->PlatformIndex;
        PlayerData->PlatformIndex = NewPlatformIndex;
        PlayerRoster.MarkItemDirty(*PlayerData);
        
        UE_LOG(LogLobby, Log, TEXT("Player %s moved from platform %d to %d"), 
               *PlayerData->DisplayName.ToString(), OldIndex, NewPlatformIndex);
    }
}

void ALobbyGameState::EvaluateReadinessAndPhase()
{
    if (!HasAuthority())
    {
        return;
    }

    bool bNewAllPlayersReady = ComputeAllPlayersReady();
    int32 ReadyCount = CountReadyPlayers();
    int32 TotalPlayers = PlayerRoster.Items.Num();

    // Update aggregate ready state
    if (bAllPlayersReady != bNewAllPlayersReady)
    {
        bAllPlayersReady = bNewAllPlayersReady;
        UE_LOG(LogLobby, Log, TEXT("Readiness changed - Ready: %d/%d, All Ready: %s"), 
               ReadyCount, TotalPlayers, bAllPlayersReady ? TEXT("Yes") : TEXT("No"));
    }

    // Phase transition logic
    if (LobbyPhase == ELobbyPhase::Gathering)
    {
        // Start countdown if all players ready and minimum met
        if (bAllPlayersReady && TotalPlayers >= MinPlayersToStart)
        {
            StartCountdown();
        }
    }
    else if (LobbyPhase == ELobbyPhase::Countdown)
    {
        // Cancel countdown if readiness condition no longer met
        if (!bAllPlayersReady || TotalPlayers < MinPlayersToStart)
        {
            CancelCountdown();
        }
    }
}

void ALobbyGameState::SetLobbyPhase(ELobbyPhase NewPhase)
{
    if (!HasAuthority() || LobbyPhase == NewPhase)
    {
        return;
    }

    ELobbyPhase OldPhase = LobbyPhase;
    LobbyPhase = NewPhase;

    UE_LOG(LogLobby, Log, TEXT("Lobby phase transition: %d -> %d"), (int32)OldPhase, (int32)NewPhase);

    // Handle phase-specific logic
    switch (NewPhase)
    {
        case ELobbyPhase::Gathering:
            bCountdownActive = false;
            CountdownRemaining = 0.0f;
            break;
            
        case ELobbyPhase::Countdown:
            // Countdown logic handled in StartCountdown()
            break;
            
        case ELobbyPhase::Locked:
            bCountdownActive = false;
            CountdownRemaining = 0.0f;
            break;
            
        case ELobbyPhase::Traveling:
            // Travel will be initiated by GameMode
            break;
            
        default:
            break;
    }

    OnRep_LobbyPhase(); // Call locally on server
}

void ALobbyGameState::StartCountdown()
{
    if (!HasAuthority())
    {
        return;
    }

    SetLobbyPhase(ELobbyPhase::Countdown);
    bCountdownActive = true;
    CountdownTimer = CountdownDuration;
    CountdownRemaining = CountdownTimer;

    UE_LOG(LogLobby, Log, TEXT("Starting countdown: %.1f seconds"), CountdownDuration);
}

void ALobbyGameState::CancelCountdown()
{
    if (!HasAuthority())
    {
        return;
    }

    if (LobbyPhase == ELobbyPhase::Countdown)
    {
        SetLobbyPhase(ELobbyPhase::Gathering);
        UE_LOG(LogLobby, Log, TEXT("Countdown cancelled - returning to Gathering phase"));
    }
}

FLobbyPlayerRepData* ALobbyGameState::FindPlayerInRoster(const FString& PlayerId)
{
    for (FLobbyPlayerRepData& PlayerData : PlayerRoster.Items)
    {
        if (PlayerData.PlayerId == PlayerId)
        {
            return &PlayerData;
        }
    }
    return nullptr;
}

TArray<FLobbyPlayerViewData> ALobbyGameState::GetPlayerViewDataArray() const
{
    TArray<FLobbyPlayerViewData> ViewDataArray;
    
    // Find host player for kick permission logic
    bool bLocalPlayerIsHost = false;
    // Note: In a real implementation, you'd check against the local player's ID
    // For now, we'll assume any player can see kick buttons for non-host players
    
    for (const FLobbyPlayerRepData& PlayerData : PlayerRoster.Items)
    {
        FLobbyPlayerViewData ViewData;
        ViewData.DisplayName = PlayerData.DisplayName;
        ViewData.TeamTag = PlayerData.TeamTag;
        ViewData.bReady = PlayerData.bReady;
        ViewData.AvatarId = PlayerData.AvatarId;
        ViewData.RoleTag = PlayerData.RoleTag;
        ViewData.bCanKick = !PlayerData.bIsHost; // Only non-host players can be kicked
        
        ViewDataArray.Add(ViewData);
    }
    
    return ViewDataArray;
}

bool ALobbyGameState::CanPlayersChangeTeams() const
{
    return LobbyPhase == ELobbyPhase::Gathering;
}

bool ALobbyGameState::CanPlayersToggleReady() const
{
    return LobbyPhase == ELobbyPhase::Gathering || LobbyPhase == ELobbyPhase::Countdown;
}

bool ALobbyGameState::ComputeAllPlayersReady() const
{
    if (PlayerRoster.Items.Num() == 0)
    {
        return false;
    }

    for (const FLobbyPlayerRepData& PlayerData : PlayerRoster.Items)
    {
        if (!PlayerData.bReady)
        {
            return false;
        }
    }

    return true;
}

int32 ALobbyGameState::CountReadyPlayers() const
{
    int32 Count = 0;
    for (const FLobbyPlayerRepData& PlayerData : PlayerRoster.Items)
    {
        if (PlayerData.bReady)
        {
            Count++;
        }
    }
    return Count;
}

FLobbyPlayerRepData* ALobbyGameState::FindHostPlayer()
{
    for (FLobbyPlayerRepData& PlayerData : PlayerRoster.Items)
    {
        if (PlayerData.bIsHost)
        {
            return &PlayerData;
        }
    }
    return nullptr;
}