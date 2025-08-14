#include "LobbyGameMode.h"
#include "GameStates/LobbyGameState.h"
#include "Actors/LobbyPlatformActor.h"
#include "DefaultPlayerState.h"
#include "Controllers/LobbyPlayerController.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h" // for UNetConnection

// Online Subsystem
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

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

		// Show loading on all clients
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(It->Get()))
			{
				LPC->ClientShowLoadingScreen();
			}
		}

		// Travel after a short delay so RPCs are processed
		bTravelScheduled = true;
		GetWorld()->GetTimerManager().SetTimer(
			TravelTimerHandle,
			this,
			&ALobbyGameMode::DoServerTravelToMatch,
			0.25f,
			false
		);

		UE_LOG(LogTemp, Log, TEXT("All ready (%d/%d). Scheduling match travel..."), Ready, Total);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not all players ready (%d/%d)."), Ready, Total);
	}
}

void ALobbyGameMode::DoServerTravelToMatch()
{
	FString URL = MatchMapPath;
	if (!URL.Contains(TEXT("?"))) { URL += TEXT("?listen"); }
	else if (!URL.Contains(TEXT("listen"))) { URL += TEXT("&listen"); }

	UE_LOG(LogTemp, Log, TEXT("ServerTravel -> %s"), *URL);
	GetWorld()->ServerTravel(URL);
}

void ALobbyGameMode::HostLeaveLobby()
{
	if (bLeaveTravelScheduled)
	{
		UE_LOG(LogTemp, Warning, TEXT("HostLeaveLobby: Leave already scheduled."));
		return;
	}

	// Remember target URL
	PendingMenuURL = MainMenuMapPath;

	// 1) Tell all remote clients to show loading and leave to main menu (each will be on their own)
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(It->Get()))
		{
			if (!LPC->IsLocalController()) // remote client
			{
				LPC->ClientShowLoadingScreen();
				LPC->ClientTravel(PendingMenuURL, TRAVEL_Absolute);
			}
		}
	}

	// 2) Show loading for the host locally (nice UX)
	if (ALobbyPlayerController* HostPC = GetHostLobbyController())
	{
		HostPC->ClientShowLoadingScreen();
	}

	// 3) Destroy the online session (if any), then travel host after completion
	IOnlineSessionPtr Session;
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		Session = OSS->GetSessionInterface();
	}

	if (Session.IsValid())
	{
		if (!bDestroyingSession)
		{
			bDestroyingSession = true;

			// Bind completion delegate
			FOnDestroySessionCompleteDelegate Delegate = FOnDestroySessionCompleteDelegate::CreateUObject(
				this, &ALobbyGameMode::HandleDestroySessionComplete);
			DestroySessionCompleteHandle.Handle = Session->AddOnDestroySessionCompleteDelegate_Handle(Delegate);
			DestroySessionCompleteHandle.bBound = true;

			const bool bDestroyCalled = Session->DestroySession(NAME_GameSession);
			if (!bDestroyCalled)
			{
				// Fallback if destroy couldn't be initiated
				UE_LOG(LogTemp, Warning, TEXT("HostLeaveLobby: DestroySession call failed to start. Falling back to timed travel."));
				if (!bLeaveTravelScheduled)
				{
					bLeaveTravelScheduled = true;
					GetWorld()->GetTimerManager().SetTimer(
						LeaveTimerHandle, this, &ALobbyGameMode::DoServerTravelToMenu, 0.5f, false);
				}
			}
			else
			{
				// Safety fallback if delegate never fires
				if (!bLeaveTravelScheduled)
				{
					bLeaveTravelScheduled = true;
					GetWorld()->GetTimerManager().SetTimer(
						LeaveTimerHandle, this, &ALobbyGameMode::DoServerTravelToMenu, 5.0f, false);
				}
			}
		}
	}
	else
	{
		// No session system -> just travel after a short delay
		if (!bLeaveTravelScheduled)
		{
			bLeaveTravelScheduled = true;
			GetWorld()->GetTimerManager().SetTimer(
				LeaveTimerHandle, this, &ALobbyGameMode::DoServerTravelToMenu, 0.25f, false);
		}
	}
}

void ALobbyGameMode::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("HandleDestroySessionComplete: %s (Success=%d)"), *SessionName.ToString(), bWasSuccessful ? 1 : 0);

	IOnlineSessionPtr Session;
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
	{
		Session = OSS->GetSessionInterface();
	}

	if (Session.IsValid() && DestroySessionCompleteHandle.bBound)
	{
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle.Handle);
		DestroySessionCompleteHandle.bBound = false;
	}

	bDestroyingSession = false;

	// Cancel fallback timer and go now
	if (bLeaveTravelScheduled)
	{
		GetWorld()->GetTimerManager().ClearTimer(LeaveTimerHandle);
	}

	DoServerTravelToMenu();
}

void ALobbyGameMode::DoServerTravelToMenu()
{
	const FString URL = PendingMenuURL.IsEmpty() ? MainMenuMapPath : PendingMenuURL;
	UE_LOG(LogTemp, Log, TEXT("Host Leave -> ServerTravel %s"), *URL);
	GetWorld()->ServerTravel(URL);
}

void ALobbyGameMode::KickPlayerByPlayerState(APlayerState* TargetPS)
{
	if (!HasAuthority() || !TargetPS) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (PC->PlayerState == TargetPS)
			{
				// 1) Tell the client to show loading (optional UX)
				if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
				{
					LPC->ClientShowLoadingScreen();
				}

				// 2) Explicitly travel the client to MainMenu
				const FString MenuURL = MainMenuMapPath.IsEmpty()
					? TEXT("/Game/Maps/MainMenuMap")
					: MainMenuMapPath;

				PC->ClientTravel(MenuURL, TRAVEL_Absolute);

				// 3) Close the connection shortly after so the RPC has time to flush
				TWeakObjectPtr<APlayerController> WeakPC = PC;
				FTimerHandle TmpHandle;
				FTimerDelegate Del;
				Del.BindLambda([WeakPC]()
					{
						if (APlayerController* StrongPC = WeakPC.Get())
						{
							if (UNetConnection* Conn = StrongPC->GetNetConnection())
							{
								Conn->Close();
							}
						}
					});
				GetWorld()->GetTimerManager().SetTimer(TmpHandle, Del, 0.35f, false);

				UE_LOG(LogTemp, Log, TEXT("Kicked player -> travel to MainMenu: %s"), *TargetPS->GetPlayerName());
				break;
			}
		}
	}
}