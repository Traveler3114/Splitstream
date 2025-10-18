// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DefaultGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API ADefaultGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	UFUNCTION(BlueprintCallable)
	void RestartLevel();
};
