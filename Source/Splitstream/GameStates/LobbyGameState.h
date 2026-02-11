#pragma once

#include "CoreMinimal.h"
#include "BaseGameState.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLobbyOnAggregateReadyChanged);

/**
 * Lobby game state that tracks aggregate player readiness.
 * Replicated to all clients for UI display of ready/total player counts.
 */
UCLASS()
class SPLITSTREAM_API ALobbyGameState : public ABaseGameState
{
	GENERATED_BODY()
public:
	ALobbyGameState();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyAggregates, Category = "Lobby|Ready")
	int32 TotalPlayers = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyAggregates, Category = "Lobby|Ready")
	int32 ReadyPlayers = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyAggregates, Category = "Lobby|Ready")
	bool bAllPlayersReady = false;

	// Called by GameMode on the server
	void SetAggregates(int32 InTotal, int32 InReady, bool bInAllReady);

	UPROPERTY(BlueprintAssignable, Category = "Lobby|Ready")
	FLobbyOnAggregateReadyChanged OnAggregateReadyChanged;

protected:
	UFUNCTION()
	void OnRep_ReadyAggregates();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};