#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLobbyOnAggregateReadyChanged);

UCLASS()
class ECHOESOFTIME_API ALobbyGameState : public AGameState
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