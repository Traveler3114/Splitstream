#pragma once

#include "CoreMinimal.h"
#include "BaseGameMode.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlatformActor;
class ADefaultPlayerState;
class ALobbyPlayerController;
class ALobbyGameState;
class APlayerState;

UCLASS()
class SPLITSTREAM_API ALobbyGameMode : public ABaseGameMode
{
    GENERATED_BODY()

public:
    ALobbyGameMode();
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Seating")
    bool bAutoSeatOnLogin = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Rules")
    bool bRequireAllReadyToStart = true;

    // Map to travel to when host starts the game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Travel")
    FString MatchMapPath = TEXT("/Game/Maps/TestMap"); // Must exist & be in packaging list

    // Map to travel to when leaving the lobby (main menu)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby|Travel")
    FString MainMenuMapPath = TEXT("/Game/Maps/MainMenuMap"); // Must exist & be in packaging list

    bool TryAssignSeatToPlayer(APlayerState* PlayerState);

protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    ALobbyPlatformActor* FindFirstFreePlatform() const;
    ALobbyPlatformActor* FindPlatformForPlayer(APlayerState* PS) const;

    void BindReadyDelegateIfNeeded(APlayerState* PS);
    UFUNCTION()
    void HandlePlayerReadyStateChanged(ADefaultPlayerState* ChangedPS);

    void RecalculateAggregates();
    bool ComputeAllPlayersReady(int32& OutTotal, int32& OutReady, bool& OutAllReady) const;
    void UpdateHostStartButton(bool bAllReady);
    ALobbyPlayerController* GetHostLobbyController() const;

public:
    // Called by PlayerController server RPC to start the game
    void StartGameIfAllowed(ALobbyPlayerController* RequestingPC);
    // Kick a specific player (by PlayerState)
    void KickPlayerByPlayerState(APlayerState* TargetPS);

private:
    // Match travel scheduling
    void DoServerTravelToMatch();
    FTimerHandle TravelTimerHandle;
    bool bTravelScheduled = false;

    // Leave flow (menu) + session destroy
    void DoServerTravelToMenu();
    void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    FTimerHandle LeaveTimerHandle;
    bool bLeaveTravelScheduled = false;

    // Online session delegate handle + state
    struct FDelegateHandleWrapper { FDelegateHandle Handle; bool bBound = false; } DestroySessionCompleteHandle;
    bool bDestroyingSession = false;

    // Store URL we intend to travel to (menu)
    FString PendingMenuURL;
};