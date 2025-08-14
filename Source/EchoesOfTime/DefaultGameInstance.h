// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "DefaultGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API UDefaultGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "GameInstance")
	void CreateSession();
};
