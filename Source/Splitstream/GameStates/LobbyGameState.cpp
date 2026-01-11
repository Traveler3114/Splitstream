#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"

ALobbyGameState::ALobbyGameState() {}

void ALobbyGameState::SetAggregates(int32 InTotal, int32 InReady, bool bInAllReady)
{
	if (!HasAuthority()) return;

	const bool bChanged = (TotalPlayers != InTotal) || (ReadyPlayers != InReady) || (bAllPlayersReady != bInAllReady);

	TotalPlayers = InTotal;
	ReadyPlayers = InReady;
	bAllPlayersReady = bInAllReady;

	if (bChanged)
	{
		// Fire locally on server too
		OnRep_ReadyAggregates();
	}
}

void ALobbyGameState::OnRep_ReadyAggregates()
{
	OnAggregateReadyChanged.Broadcast();
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyGameState, TotalPlayers);
	DOREPLIFETIME(ALobbyGameState, ReadyPlayers);
	DOREPLIFETIME(ALobbyGameState, bAllPlayersReady);
}