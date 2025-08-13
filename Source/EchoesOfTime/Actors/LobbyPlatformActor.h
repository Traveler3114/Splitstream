// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlatformActor.generated.h"


class APlayerState;
class UStaticMeshComponent;
class UArrowComponent;
class UWidgetComponent;
class APawn;


UCLASS()
class ECHOESOFTIME_API ALobbyPlatformActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALobbyPlatformActor();

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform")
	bool IsOccupied() const { return OccupantPlayerState != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Lobby|Platform")
	APlayerState* GetOccupant() const { return OccupantPlayerState; }


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Visual base */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Platform")
	UStaticMeshComponent* PlatformMesh;

	/* Forward marker / spawn anchor for display pawn */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Platform")
	UArrowComponent* SpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Platform")
	UWidgetComponent* OpenFriendListButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Platform")
	UWidgetComponent* FriendList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby|Platform")
	UWidgetComponent* PlayerLobbyInfo;


	/* Replicated occupant player state */
	UPROPERTY(ReplicatedUsing = OnRep_OccupantPlayerState)
	APlayerState* OccupantPlayerState = nullptr;

	UFUNCTION()
	void OnRep_OccupantPlayerState();




};
