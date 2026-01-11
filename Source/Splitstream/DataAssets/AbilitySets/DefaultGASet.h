// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "DefaultGASet.generated.h"

/**
 * DataAsset used to store a set of Gameplay Abilities to grant.
 */
UCLASS()
class SPLITSTREAM_API UDefaultGASet : public UDataAsset
{
	GENERATED_BODY()

public:
	// Array of Gameplay Abilities to grant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
};
