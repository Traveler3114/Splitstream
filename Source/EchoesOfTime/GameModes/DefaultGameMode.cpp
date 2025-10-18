#include "DefaultGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameStates/DefaultGameState.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"


void ADefaultGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (ADefaultGameState* GS = GetGameState<ADefaultGameState>())
    {
        GS->OnRestartRequested.AddUObject(this, &ADefaultGameMode::RestartLevel);
    }
}


AActor* ADefaultGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    ADefaultPlayerState* PS = Player->GetPlayerState<ADefaultPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: No PlayerState found for controller %s"), *Player->GetName());
        return Super::ChoosePlayerStart_Implementation(Player);
    }

    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: No AbilitySystemComponent found for player %s"), *PS->GetName());
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

    // Find all PlayerStart actors for this team
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
        UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart: Chose PlayerStart %s for team %s"), *ChosenStart->GetName(), *TeamString);
        DrawDebugString(GetWorld(), ChosenStart->GetActorLocation() + FVector(0, 0, 100), FString::Printf(TEXT("%s Start"), *TeamString), nullptr, FColor::Green, 10.0f, true);
        return ChosenStart;
    }

    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart: No PlayerStart found for team %s, using default."), *TeamString);
    return Super::ChoosePlayerStart_Implementation(Player);
}

// In DefaultGameMode.cpp
void ADefaultGameMode::RestartLevel()
{
    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FString CurrentLevel = World->GetMapName();
            // Remove any prefix (e.g., streaming levels add "UEDPIE_0_" etc.)
            CurrentLevel.RemoveFromStart(World->StreamingLevelsPrefix);

            // Build travel URL with ?listen if you are using listen server
            FString URL = CurrentLevel;
            if (!URL.Contains(TEXT("?")))
                URL += TEXT("?listen");
            else if (!URL.Contains(TEXT("listen")))
                URL += TEXT("&listen");

            World->ServerTravel(URL);
        }
    }
}