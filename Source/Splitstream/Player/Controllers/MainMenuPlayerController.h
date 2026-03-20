// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Player/Controllers/BasePlayerController.h"
#include "MainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SPLITSTREAM_API AMainMenuPlayerController : public ABasePlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UMainMenuWidget> MainMenuUIClass;

private:
	UPROPERTY()
	UUserWidget* MainMenuInstance = nullptr;
};
