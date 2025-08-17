// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"

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

    // Assign team tag if not set
    FGameplayTag AttackersTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Attackers"));
    FGameplayTag DefendersTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Defenders"));

    FName DesiredTag;
    FString TeamString;

    if (ASC->HasMatchingGameplayTag(AttackersTag))
    {
        DesiredTag = FName("PlayerStart_Attackers");
        TeamString = "Attackers";
    }
    else
    {
        DesiredTag = FName("PlayerStart_Defenders");
        TeamString = "Defenders";
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