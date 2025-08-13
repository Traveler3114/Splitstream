#include "GameStates/LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// FLobbyPlayerRepData Implementation
void FLobbyPlayerRepData::PostReplicatedAdd(const FLobbyPlayerRepDataArray& InArraySerializer)
{
    // Notify that a player was added
    if (ALobbyGameState* GameState = InArraySerializer.GetOwnerGameState())
    {
        GameState->OnPlayerJoined.Broadcast(*this);
    }
}

void FLobbyPlayerRepData::PostReplicatedChange(const FLobbyPlayerRepDataArray& InArraySerializer)
{
    // Notify that a player was updated
    if (ALobbyGameState* GameState = InArraySerializer.GetOwnerGameState())
    {
        GameState->OnPlayerUpdated.Broadcast(*this);
    }
}

void FLobbyPlayerRepData::PreReplicatedRemove(const FLobbyPlayerRepDataArray& InArraySerializer)
{
    // Notify that a player will be removed
    if (ALobbyGameState* GameState = InArraySerializer.GetOwnerGameState())
    {
        GameState->OnPlayerLeft.Broadcast(*this);
    }
}

// FLobbyPlayerRepDataArray Implementation
void FLobbyPlayerRepDataArray::AddPlayer(const FLobbyPlayerRepData& PlayerData)
{
    // Check if player already exists
    if (FindPlayer(PlayerData.PlayerId))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player with ID %d already exists in roster"), PlayerData.PlayerId);
        return;
    }

    // Add new player
    FLobbyPlayerRepData& NewPlayer = Players.AddDefaulted_GetRef();
    NewPlayer = PlayerData;
    MarkItemDirty(NewPlayer);
}

void FLobbyPlayerRepDataArray::RemovePlayer(int32 PlayerId)
{
    for (int32 i = Players.Num() - 1; i >= 0; --i)
    {
        if (Players[i].PlayerId == PlayerId)
        {
            Players.RemoveAt(i);
            MarkArrayDirty();
            break;
        }
    }
}

void FLobbyPlayerRepDataArray::UpdatePlayer(const FLobbyPlayerRepData& PlayerData)
{
    if (FLobbyPlayerRepData* ExistingPlayer = FindPlayer(PlayerData.PlayerId))
    {
        *ExistingPlayer = PlayerData;
        MarkItemDirty(*ExistingPlayer);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to update non-existent player with ID %d"), PlayerData.PlayerId);
    }
}

FLobbyPlayerRepData* FLobbyPlayerRepDataArray::FindPlayer(int32 PlayerId)
{
    for (FLobbyPlayerRepData& Player : Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            return &Player;
        }
    }
    return nullptr;
}

const FLobbyPlayerRepData* FLobbyPlayerRepDataArray::FindPlayer(int32 PlayerId) const
{
    for (const FLobbyPlayerRepData& Player : Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            return &Player;
        }
    }
    return nullptr;
}

ALobbyGameState* FLobbyPlayerRepDataArray::GetOwnerGameState() const
{
    return Cast<ALobbyGameState>(GetOuter());
}

// ALobbyGameState Implementation
ALobbyGameState::ALobbyGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    LobbyPhase = ELobbyPhase::Gathering;
    CountdownTime = 0.0f;
    bCountdownActive = false;
    CountdownDuration = 0.0f;
    MaxPlayers = 8; // Default max players

    // Set up the fast array owner
    PlayerRoster.SetOwner(this);
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ALobbyGameState, LobbyPhase);
    DOREPLIFETIME(ALobbyGameState, PlayerRoster);
    DOREPLIFETIME(ALobbyGameState, CountdownTime);
}

void ALobbyGameState::BeginPlay()
{
    Super::BeginPlay();
}

void ALobbyGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (HasAuthority() && bCountdownActive)
    {
        UpdateCountdown(DeltaTime);
    }
}

void ALobbyGameState::SetLobbyPhase(ELobbyPhase NewPhase)
{
    if (!HasAuthority())
    {
        return;
    }

    if (LobbyPhase != NewPhase)
    {
        LobbyPhase = NewPhase;
        OnLobbyPhaseChanged.Broadcast(NewPhase);
        
        UE_LOG(LogTemp, Log, TEXT("Lobby phase changed to %d"), (int32)NewPhase);
    }
}

void ALobbyGameState::StartCountdown(float Duration)
{
    if (!HasAuthority())
    {
        return;
    }

    CountdownDuration = Duration;
    CountdownTime = Duration;
    bCountdownActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Countdown started with duration %.2f seconds"), Duration);
}

void ALobbyGameState::StopCountdown()
{
    if (!HasAuthority())
    {
        return;
    }

    bCountdownActive = false;
    CountdownTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Countdown stopped"));
}

void ALobbyGameState::AddPlayerToRoster(int32 PlayerId, const FString& PlayerName, int32 PlatformIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    FLobbyPlayerRepData NewPlayer(PlayerId, PlayerName);
    NewPlayer.PlatformIndex = PlatformIndex;
    
    // First player becomes host
    if (PlayerRoster.Players.Num() == 0)
    {
        NewPlayer.bIsHost = true;
    }

    PlayerRoster.AddPlayer(NewPlayer);
    
    UE_LOG(LogTemp, Log, TEXT("Added player %s (ID: %d) to roster"), *PlayerName, PlayerId);
}

void ALobbyGameState::RemovePlayerFromRoster(int32 PlayerId)
{
    if (!HasAuthority())
    {
        return;
    }

    // Check if removing host
    const FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId);
    bool bWasHost = PlayerData && PlayerData->bIsHost;

    PlayerRoster.RemovePlayer(PlayerId);
    
    // If host was removed and there are still players, assign new host
    if (bWasHost && PlayerRoster.Players.Num() > 0)
    {
        PlayerRoster.Players[0].bIsHost = true;
        PlayerRoster.MarkItemDirty(PlayerRoster.Players[0]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Removed player (ID: %d) from roster"), PlayerId);
}

void ALobbyGameState::UpdatePlayerReady(int32 PlayerId, bool bIsReady)
{
    if (!HasAuthority())
    {
        return;
    }

    if (FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId))
    {
        PlayerData->bReady = bIsReady;
        PlayerRoster.MarkItemDirty(*PlayerData);
        
        UE_LOG(LogTemp, Log, TEXT("Player %d ready state changed to %s"), PlayerId, bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
    }
}

void ALobbyGameState::UpdatePlayerTeam(int32 PlayerId, FGameplayTag TeamTag)
{
    if (!HasAuthority())
    {
        return;
    }

    if (FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId))
    {
        PlayerData->TeamTag = TeamTag;
        PlayerRoster.MarkItemDirty(*PlayerData);
        
        UE_LOG(LogTemp, Log, TEXT("Player %d team changed to %s"), PlayerId, *TeamTag.ToString());
    }
}

void ALobbyGameState::UpdatePlayerHost(int32 PlayerId, bool bIsHost)
{
    if (!HasAuthority())
    {
        return;
    }

    if (FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId))
    {
        // If setting as host, remove host from others
        if (bIsHost)
        {
            for (FLobbyPlayerRepData& Player : PlayerRoster.Players)
            {
                if (Player.bIsHost && Player.PlayerId != PlayerId)
                {
                    Player.bIsHost = false;
                    PlayerRoster.MarkItemDirty(Player);
                }
            }
        }

        PlayerData->bIsHost = bIsHost;
        PlayerRoster.MarkItemDirty(*PlayerData);
        
        UE_LOG(LogTemp, Log, TEXT("Player %d host state changed to %s"), PlayerId, bIsHost ? TEXT("Host") : TEXT("Not Host"));
    }
}

void ALobbyGameState::UpdatePlayerAvatar(int32 PlayerId, FName AvatarId)
{
    if (!HasAuthority())
    {
        return;
    }

    if (FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId))
    {
        PlayerData->AvatarId = AvatarId;
        PlayerRoster.MarkItemDirty(*PlayerData);
        
        UE_LOG(LogTemp, Log, TEXT("Player %d avatar changed to %s"), PlayerId, *AvatarId.ToString());
    }
}

bool ALobbyGameState::AreAllPlayersReady() const
{
    if (PlayerRoster.Players.Num() == 0)
    {
        return false;
    }

    for (const FLobbyPlayerRepData& Player : PlayerRoster.Players)
    {
        if (!Player.bReady)
        {
            return false;
        }
    }
    return true;
}

bool ALobbyGameState::IsLobbyFull() const
{
    return PlayerRoster.Players.Num() >= MaxPlayers;
}

int32 ALobbyGameState::GetPlayerCount() const
{
    return PlayerRoster.Players.Num();
}

FLobbyPlayerRepData ALobbyGameState::GetPlayerData(int32 PlayerId) const
{
    if (const FLobbyPlayerRepData* PlayerData = PlayerRoster.FindPlayer(PlayerId))
    {
        return *PlayerData;
    }
    return FLobbyPlayerRepData();
}

void ALobbyGameState::OnRep_LobbyPhase()
{
    OnLobbyPhaseChanged.Broadcast(LobbyPhase);
}

void ALobbyGameState::OnRep_CountdownTime()
{
    OnCountdownUpdated.Broadcast(CountdownTime);
}

void ALobbyGameState::UpdateCountdown(float DeltaTime)
{
    if (!bCountdownActive)
    {
        return;
    }

    CountdownTime -= DeltaTime;
    OnCountdownUpdated.Broadcast(CountdownTime);

    if (CountdownTime <= 0.0f)
    {
        CountdownTime = 0.0f;
        bCountdownActive = false;
        
        // Handle countdown completion based on current phase
        switch (LobbyPhase)
        {
            case ELobbyPhase::Countdown:
                SetLobbyPhase(ELobbyPhase::Traveling);
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Countdown completed"));
    }
}

