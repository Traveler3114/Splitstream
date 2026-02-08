#include "DefaultGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameStates/DefaultGameState.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "Controllers/DefaultPlayerController.h"
#include "DefaultGameInstance.h"

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


AActor* ADefaultGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ADefaultPlayerState* PS = Player->GetPlayerState<ADefaultPlayerState>();
	if (!PS)
		return Super::ChoosePlayerStart_Implementation(Player);

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
		return Super::ChoosePlayerStart_Implementation(Player);

	FGameplayTag PastTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));
	FGameplayTag FutureTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
	FName DesiredTag;
	FString TeamString;

	if (ASC->HasMatchingGameplayTag(PastTag)) {DesiredTag = FName("PlayerStart_Past"); TeamString = "Past";}
	else if (ASC->HasMatchingGameplayTag(FutureTag)) {DesiredTag = FName("PlayerStart_Future"); TeamString = "Future";}
	else {DesiredTag = FName("PlayerStart_Past"); TeamString = "Past";}

	TArray<APlayerStart*> TeamStarts;
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	for (AActor* Start : PlayerStarts)
	{
		APlayerStart* PSActor = Cast<APlayerStart>(Start);
		if (PSActor && PSActor->PlayerStartTag == DesiredTag)
			TeamStarts.Add(PSActor);
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

void ADefaultGameMode::OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator, ETimelineEra Era)
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
		GS->StartAlarm(GS->PreAlarmSoonestInstigator);
	}
}

void ADefaultGameMode::RestartLevel()
{
	if (!HasAuthority())
		return;



	FString LobbyURL = TEXT("/Game/Maps/LobbyMap");
	if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
		if (!GS->LobbyMapPath.IsEmpty())
			LobbyURL = GS->LobbyMapPath;

	if (UDefaultGameInstance* GI = Cast<UDefaultGameInstance>(GetGameInstance()))
	{
		LobbyURL = GI->LobbyMapPath;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(Iterator->Get()))
			PC->ClientShowLoadingScreen();
	}


	GetWorld()->ServerTravel(LobbyURL);
}