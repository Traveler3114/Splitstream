#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "LobbyGameState.generated.h"

// Enum for different lobby phases
UENUM(BlueprintType)
enum class ELobbyPhase : uint8
{
    Gathering      UMETA(DisplayName = "Gathering"),
    Countdown      UMETA(DisplayName = "Countdown"),
    Locked         UMETA(DisplayName = "Locked"),
    Traveling      UMETA(DisplayName = "Traveling"),
    InMatch        UMETA(DisplayName = "In Match")
};

// Fast array item for individual player data
USTRUCT(BlueprintType)
struct ECHOESOFTIME_API FLobbyPlayerRepData : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FLobbyPlayerRepData()
        : PlayerId(0)
        , PlayerName(TEXT(""))
        , TeamTag(FGameplayTag())
        , bReady(false)
        , bIsHost(false)
        , AvatarId(NAME_None)
        , PlatformIndex(-1)
    {
    }

    FLobbyPlayerRepData(int32 InPlayerId, const FString& InPlayerName)
        : PlayerId(InPlayerId)
        , PlayerName(InPlayerName)
        , TeamTag(FGameplayTag())
        , bReady(false)
        , bIsHost(false)
        , AvatarId(NAME_None)
        , PlatformIndex(-1)
    {
    }

    // Player identifier
    UPROPERTY(BlueprintReadOnly)
    int32 PlayerId;

    // Player display name
    UPROPERTY(BlueprintReadOnly)
    FString PlayerName;

    // Team assignment
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag TeamTag;

    // Ready state
    UPROPERTY(BlueprintReadOnly)
    bool bReady;

    // Host status
    UPROPERTY(BlueprintReadOnly)
    bool bIsHost;

    // Avatar identifier
    UPROPERTY(BlueprintReadOnly)
    FName AvatarId;

    // Platform/slot index in lobby
    UPROPERTY(BlueprintReadOnly)
    int32 PlatformIndex;

    // FFastArraySerializerItem interface
    void PostReplicatedAdd(const struct FLobbyPlayerRepDataArray& InArraySerializer);
    void PostReplicatedChange(const struct FLobbyPlayerRepDataArray& InArraySerializer);
    void PreReplicatedRemove(const struct FLobbyPlayerRepDataArray& InArraySerializer);

    // Comparison operator for finding items
    bool operator==(const FLobbyPlayerRepData& Other) const
    {
        return PlayerId == Other.PlayerId;
    }
};

// Fast array serializer for player roster
USTRUCT(BlueprintType)
struct ECHOESOFTIME_API FLobbyPlayerRepDataArray : public FFastArraySerializer
{
    GENERATED_BODY()

    FLobbyPlayerRepDataArray()
    {
    }

    // Array of players in lobby
    UPROPERTY(BlueprintReadOnly)
    TArray<FLobbyPlayerRepData> Players;

    // FFastArraySerializer interface
    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FLobbyPlayerRepData, FLobbyPlayerRepDataArray>(Players, DeltaParms, *this);
    }



    // Helper functions for managing players
    void AddPlayer(const FLobbyPlayerRepData& PlayerData);
    void RemovePlayer(int32 PlayerId);
    void UpdatePlayer(const FLobbyPlayerRepData& PlayerData);
    FLobbyPlayerRepData* FindPlayer(int32 PlayerId);
    const FLobbyPlayerRepData* FindPlayer(int32 PlayerId) const;

    // Get owning game state for callbacks
    class ALobbyGameState* GetOwnerGameState() const;
};

// Template specialization for net delta serialization
template<>
struct TStructOpsTypeTraits<FLobbyPlayerRepDataArray> : public TStructOpsTypeTraitsBase2<FLobbyPlayerRepDataArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};

UCLASS(BlueprintType, Blueprintable)
class ECHOESOFTIME_API ALobbyGameState : public AGameState
{
    GENERATED_BODY()

public:
    ALobbyGameState();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Current lobby phase
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LobbyPhase)
    ELobbyPhase LobbyPhase;

    // Player roster using fast array
    UPROPERTY(BlueprintReadOnly, Replicated)
    FLobbyPlayerRepDataArray PlayerRoster;

    // Countdown timer for transitions
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CountdownTime)
    float CountdownTime;

    // Phase transition functions
    UFUNCTION(BlueprintCallable, CallInEditor = true, Category = "Lobby")
    void SetLobbyPhase(ELobbyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void StartCountdown(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void StopCountdown();

    // Player management
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void AddPlayerToRoster(int32 PlayerId, const FString& PlayerName, int32 PlatformIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void RemovePlayerFromRoster(int32 PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void UpdatePlayerReady(int32 PlayerId, bool bIsReady);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void UpdatePlayerTeam(int32 PlayerId, FGameplayTag TeamTag);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void UpdatePlayerHost(int32 PlayerId, bool bIsHost);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void UpdatePlayerAvatar(int32 PlayerId, FName AvatarId);

    // Validation functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby")
    bool AreAllPlayersReady() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby")
    bool IsLobbyFull() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby")
    int32 GetPlayerCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby")
    FLobbyPlayerRepData GetPlayerData(int32 PlayerId) const;

    // Blueprint events for player roster changes
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FLobbyPlayerRepData&, PlayerData);
    UPROPERTY(BlueprintAssignable, Category = "Lobby Events")
    FOnPlayerJoined OnPlayerJoined;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FLobbyPlayerRepData&, PlayerData);
    UPROPERTY(BlueprintAssignable, Category = "Lobby Events")
    FOnPlayerLeft OnPlayerLeft;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUpdated, const FLobbyPlayerRepData&, PlayerData);
    UPROPERTY(BlueprintAssignable, Category = "Lobby Events")
    FOnPlayerUpdated OnPlayerUpdated;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPhaseChanged, ELobbyPhase, NewPhase);
    UPROPERTY(BlueprintAssignable, Category = "Lobby Events")
    FOnLobbyPhaseChanged OnLobbyPhaseChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownUpdated, float, RemainingTime);
    UPROPERTY(BlueprintAssignable, Category = "Lobby Events")
    FOnCountdownUpdated OnCountdownUpdated;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Replication callbacks
    UFUNCTION()
    void OnRep_LobbyPhase();

    UFUNCTION()
    void OnRep_CountdownTime();

    // Internal countdown management
    UPROPERTY()
    bool bCountdownActive;

    UPROPERTY()
    float CountdownDuration;

    void UpdateCountdown(float DeltaTime);

    // Maximum players allowed in lobby
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby Settings")
    int32 MaxPlayers;

private:
    // Maximum players allowed in lobby
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby Settings")
    int32 MaxPlayers;
};