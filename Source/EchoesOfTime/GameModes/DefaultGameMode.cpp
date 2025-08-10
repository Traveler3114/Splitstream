// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/DefaultCharacter.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/AttributeSets/DefaultAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DefaultPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

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

    // Only select player start based on existing team tag
    FGameplayTag AttackersTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Future"));
    FGameplayTag DefendersTag = FGameplayTag::RequestGameplayTag(TEXT("Team.Past"));

    FName DesiredTag;
    FString TeamString;

    if (ASC->HasMatchingGameplayTag(AttackersTag))
    {
        DesiredTag = FName("PlayerStart_Future");
        TeamString = "Future";
    }
    else if (ASC->HasMatchingGameplayTag(DefendersTag))
    {
        DesiredTag = FName("PlayerStart_Past");
        TeamString = "Past";
    }
    else
    {
        // Fallback: no team assigned, use default behavior
        return Super::ChoosePlayerStart_Implementation(Player);
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
        DrawDebugString(GetWorld(), ChosenStart->GetActorLocation() + FVector(0, 0, 100), FString::Printf(TEXT("%s Start"), *TeamString), nullptr, FColor::Green, 10.0f, true);
        return ChosenStart;
    }

    return Super::ChoosePlayerStart_Implementation(Player);
}