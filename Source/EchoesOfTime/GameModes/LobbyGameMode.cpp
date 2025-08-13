#include "LobbyGameMode.h"
#include "GameStates/LobbyGameState.h"
#include "Actors/LobbyPlatformActor.h"
#include "DefaultPlayerState.h"
#include "Controllers/LobbyPlayerController.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ALobbyGameMode::ALobbyGameMode()
{
	PlayerStateClass = ADefaultPlayerState::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer || !NewPlayer->PlayerState) return;

	BindReadyDelegateIfNeeded(NewPlayer->PlayerState);

	if (bAutoSeatOnLogin)
	{
		TryAssignSeatToPlayer(NewPlayer->PlayerState);
	}

	RecalculateAggregates();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (Exiting && Exiting->PlayerState)
	{
		if (ALobbyPlatformActor* Plat = FindPlatformForPlayer(Exiting->PlayerState))
		{
			Plat->ServerClearOccupant();
		}
	}
	Super::Logout(Exiting);
	RecalculateAggregates();
}

bool ALobbyGameMode::TryAssignSeatToPlayer(APlayerState* PlayerState)
{
	if (!PlayerState) return false;
	if (FindPlatformForPlayer(PlayerState)) return true;

	if (ALobbyPlatformActor* Free = FindFirstFreePlatform())
	{
		return Free->ServerAssignOccupant(PlayerState);
	}
	return false;
}

ALobbyPlatformActor* ALobbyGameMode::FindFirstFreePlatform() const
{
	for (TActorIterator<ALobbyPlatformActor> It(GetWorld()); It; ++It)
	{
		if (ALobbyPlatformActor* Plat = *It)
		{
			if (!Plat->IsOccupied()) return Plat;
		}
	}
	return nullptr;
}

ALobbyPlatformActor* ALobbyGameMode::FindPlatformForPlayer(APlayerState* PS) const
{
	if (!PS) return nullptr;
	for (TActorIterator<ALobbyPlatformActor> It(GetWorld()); It; ++It)
	{
		if (ALobbyPlatformActor* Plat = *It)
		{
			if (Plat->GetOccupant() == PS) return Plat;
		}
	}
	return nullptr;
}

void ALobbyGameMode::BindReadyDelegateIfNeeded(APlayerState* PS)
{
	if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS))
	{
		DPS->OnReadyChanged.RemoveAll(this);
		DPS->OnReadyChanged.AddDynamic(this, &ALobbyGameMode::HandlePlayerReadyStateChanged);
	}
}

void ALobbyGameMode::HandlePlayerReadyStateChanged(ADefaultPlayerState* /*ChangedPS*/)
{
	RecalculateAggregates();
}

bool ALobbyGameMode::ComputeAllPlayersReady(int32& OutTotal, int32& OutReady, bool& OutAllReady) const
{
	OutTotal = 0; OutReady = 0; OutAllReady = false;
	if (!GameState) return false;

	const TArray<APlayerState*>& Players = GameState->PlayerArray;
	OutTotal = Players.Num();
	if (OutTotal == 0) { OutAllReady = false; return true; }

	for (APlayerState* PS : Players)
	{
		if (ADefaultPlayerState* DPS = Cast<ADefaultPlayerState>(PS))
		{
			if (DPS->IsReady()) ++OutReady;
		}
	}
	OutAllReady = (OutReady == OutTotal && OutTotal > 0);
	return true;
}

void ALobbyGameMode::RecalculateAggregates()
{
	int32 Total = 0, Ready = 0;
	bool bAll = false;
	ComputeAllPlayersReady(Total, Ready, bAll);

	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		LGS->SetAggregates(Total, Ready, bAll);
	}

	UpdateHostStartButton(bAll);
}

void ALobbyGameMode::UpdateHostStartButton(bool bAllReady)
{
	if (!bRequireAllReadyToStart)
	{
		bAllReady = true;
	}
	if (ALobbyPlayerController* HostPC = GetHostLobbyController())
	{
		HostPC->ClientSetStartButtonEnabled(bAllReady);
	}
}

ALobbyPlayerController* ALobbyGameMode::GetHostLobbyController() const
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(It->Get()))
		{
			if (LPC->HasAuthority())
				return LPC;
		}
	}
	return nullptr;
}

void ALobbyGameMode::StartGameIfAllowed(ALobbyPlayerController* RequestingPC)
{
	if (!RequestingPC || !RequestingPC->HasAuthority()) return;

	int32 Total = 0, Ready = 0; bool bAll = false;
	ComputeAllPlayersReady(Total, Ready, bAll);

	if (!bRequireAllReadyToStart || bAll)
	{
		if (bTravelScheduled)
		{
			UE_LOG(LogTemp, Warning, TEXT("StartGameIfAllowed: Travel already scheduled."));
			return;
		}

		// 1. Tell every client to show loading screen
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(It->Get()))
			{
				LPC->ClientShowLoadingScreen();
			}
		}

		// 2. Schedule travel shortly after (allow RPC to reach clients first)
		bTravelScheduled = true;
		GetWorld()->GetTimerManager().SetTimer(
			TravelTimerHandle,
			this,
			&ALobbyGameMode::DoServerTravel,
			0.25f,
			false
		);

		UE_LOG(LogTemp, Log, TEXT("StartGameIfAllowed: All ready (%d/%d). Scheduling travel..."), Ready, Total);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StartGameIfAllowed: Not all players ready (%d/%d)."), Ready, Total);
	}
}

void ALobbyGameMode::DoServerTravel()
{
	// Build URL (relative, no bAbsolute=true)
	FString URL = MatchMapPath;
	// Add ?listen if not present
	if (!URL.Contains(TEXT("?")))
	{
		URL += TEXT("?listen");
	}
	else if (!URL.Contains(TEXT("listen")))
	{
		URL += TEXT("&listen");
	}

	UE_LOG(LogTemp, Log, TEXT("ServerTravel -> %s"), *URL);
	GetWorld()->ServerTravel(URL); // DO NOT pass 'true' here, keep relative content path
}