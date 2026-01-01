// DefaultGameMode.cpp

#include "DefaultGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Controllers/DefaultPlayerController.h"

void ADefaultGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
	{
		GS->OnRestartRequested.AddDynamic(this, &ADefaultGameMode::RestartLevel);
		GS->OnAlarmStarted.AddDynamic(this, &ADefaultGameMode::OnAlarmStarted);
		GS->OnAlarmCanceled.AddDynamic(this, &ADefaultGameMode::OnAlarmCanceled);

		GS->OnPreAlarmStarted.AddDynamic(this, &ADefaultGameMode::OnPreAlarmStarted);
		GS->OnPreAlarmCanceled.AddDynamic(this, &ADefaultGameMode::OnPreAlarmCanceled);
	}
}


void ADefaultGameMode::HostLeaveLobby()
{
	if (!HasAuthority())
		return;

	FString MenuURL = TEXT("/Game/Maps/MainMenuMap");
	if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
	{
		if (!GS->MainMenuMapPath.IsEmpty())
			MenuURL = GS->MainMenuMapPath;
	}
	PendingMenuURL = MenuURL;

	// Notify all remote clients
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && !PC->IsLocalController())
		{
			if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
			{
				MyPC->ClientShowLoadingScreen();
			}
			PC->ClientTravel(MenuURL, TRAVEL_Absolute);
		}
	}
	// Host loading screen
	APlayerController* HostPC = UGameplayStatics::GetPlayerController(this, 0);
	if (HostPC)
		if (ADefaultPlayerController* HostDefPC = Cast<ADefaultPlayerController>(HostPC))
			HostDefPC->ClientShowLoadingScreen();

	// --- Session destruction logic
	IOnlineSessionPtr Session;
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
		Session = OSS->GetSessionInterface();

	if (Session.IsValid())
	{
		if (!bDestroyingSession)
		{
			bDestroyingSession = true;
			FOnDestroySessionCompleteDelegate Delegate = FOnDestroySessionCompleteDelegate::CreateUObject(
				this, &ADefaultGameMode::HandleDestroySessionComplete);
			DestroySessionCompleteHandle.Handle = Session->AddOnDestroySessionCompleteDelegate_Handle(Delegate);
			DestroySessionCompleteHandle.bBound = true;

			const bool bDestroyCalled = Session->DestroySession(NAME_GameSession);
			if (!bDestroyCalled)
			{
				// fallback if session destroy could not be initiated
				GetWorldTimerManager().SetTimer(
					LeaveTimerHandle, this, &ADefaultGameMode::DoServerTravelToMenu, 0.5f, false);
			}
			else
			{
				// fallback in case delegate never fires
				GetWorldTimerManager().SetTimer(
					LeaveTimerHandle, this, &ADefaultGameMode::DoServerTravelToMenu, 5.0f, false);
			}
		}
	}
	else
	{
		// No sessions, just travel after short delay
		GetWorldTimerManager().SetTimer(
			LeaveTimerHandle, this, &ADefaultGameMode::DoServerTravelToMenu, 0.25f, false);
	}
}

void ADefaultGameMode::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Session;
	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get())
		Session = OSS->GetSessionInterface();

	if (Session.IsValid() && DestroySessionCompleteHandle.bBound)
	{
		Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle.Handle);
		DestroySessionCompleteHandle.bBound = false;
	}
	bDestroyingSession = false;

	// Cancel the fallback timer and travel now
	GetWorldTimerManager().ClearTimer(LeaveTimerHandle);
	DoServerTravelToMenu();
}

void ADefaultGameMode::DoServerTravelToMenu()
{
	GetWorld()->ServerTravel(PendingMenuURL);
}

AActor* ADefaultGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ADefaultPlayerState* PS = Player->GetPlayerState<ADefaultPlayerState>();
	if (!PS)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
	FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

	FName DesiredTag;
	FString TeamString;

	if (ASC->HasMatchingGameplayTag(PastTag))
	{
		DesiredTag = FName("PlayerStart_Past");
		TeamString = "Past";
	}
	else if (ASC->HasMatchingGameplayTag(FutureTag))
	{
		DesiredTag = FName("PlayerStart_Future");
		TeamString = "Future";
	}
	else
	{
		DesiredTag = FName("PlayerStart_Past");
		TeamString = "Past";
	}

	TArray<APlayerStart*> TeamStarts;
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	for (AActor* Start : PlayerStarts)
	{
		APlayerStart* PSActor = Cast<APlayerStart>(Start);
		if (PSActor && PSActor->PlayerStartTag == DesiredTag)
		{
			TeamStarts.Add(PSActor);
		}
	}

	if (TeamStarts.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, TeamStarts.Num() - 1);
		APlayerStart* ChosenStart = TeamStarts[Index];
		return ChosenStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void ADefaultGameMode::OnAlarmStarted(float AlarmEndTime)
{
	if (!HasAuthority())
		return;

	GetWorldTimerManager().ClearTimer(PreAlarmTimerHandle);

	float Remaining = AlarmEndTime - GetWorld()->GetTimeSeconds();
	Remaining = FMath::Max(0.f, Remaining);

	GetWorldTimerManager().ClearTimer(RestartTimerHandle);
	GetWorldTimerManager().SetTimer(RestartTimerHandle, this, &ADefaultGameMode::RestartLevel, Remaining, false);
}

void ADefaultGameMode::OnAlarmCanceled()
{
	if (!HasAuthority())
		return;

	GetWorldTimerManager().ClearTimer(RestartTimerHandle);
}

void ADefaultGameMode::OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator)
{
	if (!HasAuthority())
		return;

	float Remaining = PreAlarmEndTime - GetWorld()->GetTimeSeconds();
	Remaining = FMath::Max(0.f, Remaining);

	GetWorldTimerManager().ClearTimer(PreAlarmTimerHandle);
	GetWorldTimerManager().SetTimer(PreAlarmTimerHandle, this, &ADefaultGameMode::PreAlarmTimeout, Remaining, false);
}

void ADefaultGameMode::OnPreAlarmCanceled()
{
	if (!HasAuthority())
		return;

	GetWorldTimerManager().ClearTimer(PreAlarmTimerHandle);
}

void ADefaultGameMode::PreAlarmTimeout()
{
	if (!HasAuthority())
		return;
	if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
	{
		GS->StartAlarm(GS->PreAlarmInstigator);
	}
}

void ADefaultGameMode::RestartLevel()
{
	if (!HasAuthority())
	{
		return;
	}

	FString LobbyURL = TEXT("/Game/Maps/LobbyMap");
	if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
	{
		if (!GS->LobbyMapPath.IsEmpty())
		{
			LobbyURL = GS->LobbyMapPath;
		}
	}

	// Show loading for all players before travel
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(Iterator->Get()))
		{
			PC->ClientShowLoadingScreen();
		}
	}

	// Send everyone to the lobby map (disconnects all)
	GetWorld()->ServerTravel(LobbyURL);
}