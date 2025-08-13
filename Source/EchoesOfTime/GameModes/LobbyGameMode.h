#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

class ALobbyPlatformActor;
class ADefaultPlayerState;
class ALobbyPlayerController;
class ALobbyGameState;

UCLASS()
class ECHOESOFTIME_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Seating")
	bool bAutoSeatOnLogin = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Rules")
	bool bRequireAllReadyToStart = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Travel")
	FString MatchMapPath = TEXT("/Game/Maps/TestMap"); // Must exist & be in packaging list

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

private:
	void DoServerTravel();                // executes after short delay
	FTimerHandle TravelTimerHandle;
	bool bTravelScheduled = false;
};