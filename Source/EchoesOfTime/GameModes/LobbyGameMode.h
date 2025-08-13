// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

// Forward declarations
class ALobbyGameState;
class ALobbyPlatformActor;

UCLASS()
class ECHOESOFTIME_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
    ALobbyGameMode();

    virtual void BeginPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Platform management - now using indices instead of direct references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
    TArray<AActor*> LobbyPlatforms;

    // Legacy readiness check - now delegates to GameState
    UFUNCTION()
    void CheckAllPlayersReady();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void StartGame();

    // Server RPC for validated kick functionality
    UFUNCTION(BlueprintCallable, Category = "Lobby", CallInEditor = false)
    void ServerKickPlayer(const FString& TargetPlayerId, APlayerController* RequestingPlayer);

    // Legacy kick function for backward compatibility
    UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
    void KickPlayer(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void HandleKickRequestedFromPlatform(ALobbyPlatformActor* Platform);

    // Platform management
    UFUNCTION(BlueprintPure, Category = "Lobby")
    int32 FindAvailablePlatformIndex() const;

    UFUNCTION(BlueprintPure, Category = "Lobby")
    ALobbyPlatformActor* GetPlatformByIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void FreePlatformIndex(int32 PlatformIndex);

protected:
    // Reference to our custom GameState
    UPROPERTY(BlueprintReadOnly, Category = "Lobby")
    ALobbyGameState* LobbyGameState;

    virtual void InitGameState() override;

private:
    // Helper to find player controller by unique ID
    APlayerController* FindPlayerControllerById(const FString& PlayerId) const;
};
