// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SPLITSTREAM_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UMainMenuWidget> MainMenuUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LoadingWidgetClass;

	UFUNCTION(BlueprintCallable)
	void ShowLoadingWidget();

private:
	UPROPERTY()
	UUserWidget* MainMenuInstance = nullptr;
};
