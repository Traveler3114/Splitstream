// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"


UCLASS()
class ECHOESOFTIME_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Use AActor* for compatibility with GetAllActorsOfClass
	UPROPERTY(EditAnywhere,BlueprintReadWrite)

	TArray<AActor*> LobbyPlatforms;


	UFUNCTION()
	void CheckAllPlayersReady();

	//UFUNCTION(BlueprintCallable)
	//void StartGame();

	//UFUNCTION(BlueprintNativeEvent, Category = "Lobby")
	//void KickPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void KickPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	void HandleKickRequestedFromPlatform(class ALobbyPlatformActor* Platform);
};
