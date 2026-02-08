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

    if (ASC->HasMatchingGameplayTag(PastTag)) { DesiredTag = FName("PlayerStart_Past"); TeamString = "Past"; }
    else if (ASC->HasMatchingGameplayTag(FutureTag)) { DesiredTag = FName("PlayerStart_Future"); TeamString = "Future"; }
    else { DesiredTag = FName("PlayerStart_Past"); TeamString = "Past"; }

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

void ADefaultGameMode::OnAlarmStarted(float AlarmEndTime, ETimelineEra Era)
{
    if (!HasAuthority())
        return;

    float Remaining = AlarmEndTime - GetWorld()->GetTimeSeconds();
    Remaining = FMath::Max(0.f, Remaining);

    FTimerHandle& EraHandle = (Era == ETimelineEra::Past) ? AlarmTimerHandlePast : AlarmTimerHandleFuture;
    GetWorldTimerManager().ClearTimer(EraHandle);
    GetWorldTimerManager().SetTimer(
        EraHandle,
        FTimerDelegate::CreateUObject(this, &ADefaultGameMode::EliminatePlayersInEra, Era),
        Remaining, false
    );
}

void ADefaultGameMode::EliminatePlayersInEra(ETimelineEra Era)
{
    FGameplayTag EraTag = (Era == ETimelineEra::Past)
        ? FGameplayTag::RequestGameplayTag(TEXT("Team.Past"))
        : FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));

    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(Iterator->Get()))
        {
            if (ADefaultPlayerState* PS = PC->GetPlayerState<ADefaultPlayerState>())
            {
                if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
                {
                    if (ASC->HasMatchingGameplayTag(EraTag))
                    {
                        PC->ChangeState(NAME_Spectating);
                        // Optionally: PC->ClientShowDefeatOverlay();
                    }
                }
            }
        }
    }
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

    // Use a separate timer per era
    if (Era == ETimelineEra::Past)
    {
        GetWorldTimerManager().ClearTimer(PreAlarmTimerHandlePast);
        GetWorldTimerManager().SetTimer(PreAlarmTimerHandlePast, this, &ADefaultGameMode::PreAlarmTimeoutPast, Remaining, false);
    }
    else
    {
        GetWorldTimerManager().ClearTimer(PreAlarmTimerHandleFuture);
        GetWorldTimerManager().SetTimer(PreAlarmTimerHandleFuture, this, &ADefaultGameMode::PreAlarmTimeoutFuture, Remaining, false);
    }
}

void ADefaultGameMode::OnPreAlarmCanceled()
{
    if (!HasAuthority())
        return;
    GetWorldTimerManager().ClearTimer(PreAlarmTimerHandlePast);
    GetWorldTimerManager().ClearTimer(PreAlarmTimerHandleFuture);
}

void ADefaultGameMode::PreAlarmTimeoutPast()
{
    if (!HasAuthority())
        return;
    if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
    {
        GS->StartAlarm(GS->PastPreAlarm.SoonestInstigator, ETimelineEra::Past);
    }
}
void ADefaultGameMode::PreAlarmTimeoutFuture()
{
    if (!HasAuthority())
        return;
    if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
    {
        GS->StartAlarm(GS->FuturePreAlarm.SoonestInstigator, ETimelineEra::Future);
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