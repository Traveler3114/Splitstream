// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "GarageDoor.generated.h"

UCLASS()
class ECHOESOFTIME_API AGarageDoor : public AActor, public IPuzzleCompletionReceiver
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGarageDoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* GarageMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnPuzzleCompleted_Implementation() override;
	virtual void OnPuzzleReset_Implementation() override;
};
