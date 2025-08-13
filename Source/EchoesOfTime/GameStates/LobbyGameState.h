#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameplayTagContainer.h"
#include "Engine/NetSerialization.h"
#include "LobbyGameState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLobby, Log, All);

// Lobby phases for clear state management
UENUM(BlueprintType)
enum class ELobbyPhase : uint8
{
    Gathering,   // Players joining, can change teams/ready state freely
    Countdown,   // All players ready, countdown active, limited changes allowed
    Locked,      // Brief phase before travel, no changes allowed
    Traveling,   // Server travel in progress
    InMatch      // Match started (not used in lobby but for completeness)
};

// Forward declarations
class ADefaultPlayerState;

// Replicated player data for efficient roster updates
USTRUCT()
struct FLobbyPlayerRepData : public FFastArraySerializerItem
{
    GENERATED_BODY()

    UPROPERTY()
    FString PlayerId;

    UPROPERTY()
    FText DisplayName;

    UPROPERTY()
    FGameplayTag TeamTag;

    UPROPERTY()
    FGameplayTag RoleTag; // Placeholder for future role selection

    UPROPERTY()
    bool bReady = false;

    UPROPERTY()
    bool bIsHost = false;

    UPROPERTY()
    int32 AvatarId = 0; // Placeholder for future avatar system

    UPROPERTY()
    int32 PlatformIndex = -1; // Index into GameMode's platform array

    FLobbyPlayerRepData()
    {
        PlayerId = TEXT("");
        DisplayName = FText::GetEmpty();
        TeamTag = FGameplayTag::EmptyTag;
        RoleTag = FGameplayTag::EmptyTag;
    }

    FLobbyPlayerRepData(const FString& InPlayerId, const FText& InDisplayName, int32 InPlatformIndex)
        : PlayerId(InPlayerId), DisplayName(InDisplayName), PlatformIndex(InPlatformIndex)
    {
        TeamTag = FGameplayTag::EmptyTag;
        RoleTag = FGameplayTag::EmptyTag;
    }
};

// Fast array for efficient player roster replication
USTRUCT()
struct FLobbyPlayerRepDataArray : public FFastArraySerializer
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FLobbyPlayerRepData> Items;

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FLobbyPlayerRepData, FLobbyPlayerRepDataArray>(Items, DeltaParms, *this);
    }
};

template<>
struct TStructOpsTypeTraits<FLobbyPlayerRepDataArray> : public TStructOpsTypeTraitsBase2<FLobbyPlayerRepDataArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};

// View data structure for batch UI updates
USTRUCT(BlueprintType)
struct FLobbyPlayerViewData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag TeamTag;

    UPROPERTY(BlueprintReadOnly)
    bool bReady = false;

    UPROPERTY(BlueprintReadOnly)
    bool bCanKick = false;

    UPROPERTY(BlueprintReadOnly)
    int32 AvatarId = 0;

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag RoleTag;

    FLobbyPlayerViewData()
    {
        DisplayName = FText::GetEmpty();
        TeamTag = FGameplayTag::EmptyTag;
        RoleTag = FGameplayTag::EmptyTag;
    }
};

UCLASS()
class ECHOESOFTIME_API ALobbyGameState : public AGameState
{
    GENERATED_BODY()

public:
    ALobbyGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Current lobby phase
    UPROPERTY(Replicated, ReplicatedUsing = OnRep_LobbyPhase, BlueprintReadOnly, Category = "Lobby")
    ELobbyPhase LobbyPhase = ELobbyPhase::Gathering;

    UFUNCTION()
    void OnRep_LobbyPhase();

    // Countdown timer (in seconds, only relevant during Countdown phase)
    UPROPERTY(Replicated, ReplicatedUsing = OnRep_CountdownRemaining, BlueprintReadOnly, Category = "Lobby")
    float CountdownRemaining = 0.0f;

    UFUNCTION()
    void OnRep_CountdownRemaining();

    // Aggregate readiness state for easy access
    UPROPERTY(Replicated, ReplicatedUsing = OnRep_AllPlayersReady, BlueprintReadOnly, Category = "Lobby")
    bool bAllPlayersReady = false;

    UFUNCTION()
    void OnRep_AllPlayersReady();

    // Minimum players required to start
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    int32 MinPlayersToStart = 2;

    // Countdown duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    float CountdownDuration = 5.0f;

    // Fast array for player roster
    UPROPERTY(Replicated)
    FLobbyPlayerRepDataArray PlayerRoster;

    // Server-side functions for managing player roster
    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void AddPlayerToRoster(ADefaultPlayerState* PlayerState, int32 PlatformIndex);

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void RemovePlayerFromRoster(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void UpdatePlayerRosterEntry(const FString& PlayerId, const FGameplayTag& TeamTag, bool bReady);

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void UpdatePlayerPlatformIndex(const FString& PlayerId, int32 NewPlatformIndex);

    // Readiness evaluation and phase management
    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void EvaluateReadinessAndPhase();

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void SetLobbyPhase(ELobbyPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void StartCountdown();

    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void CancelCountdown();

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Lobby")
    FLobbyPlayerRepData* FindPlayerInRoster(const FString& PlayerId);

    UFUNCTION(BlueprintPure, Category = "Lobby")
    TArray<FLobbyPlayerViewData> GetPlayerViewDataArray() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool CanPlayersChangeTeams() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    bool CanPlayersToggleReady() const;

    // Blueprint events for UI updates
    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void OnLobbyPhaseChanged(ELobbyPhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void OnCountdownUpdate(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void OnPlayerRosterChanged();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Internal countdown management
    bool bCountdownActive = false;
    float CountdownTimer = 0.0f;

    // Helper to compute aggregate readiness
    bool ComputeAllPlayersReady() const;

    // Helper to count ready players
    int32 CountReadyPlayers() const;

    // Helper to find host player
    FLobbyPlayerRepData* FindHostPlayer();
};