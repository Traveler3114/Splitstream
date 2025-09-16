// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "SearchableActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ASearchableActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASearchableActor();

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
	USceneComponent* DefaultSceneRoot;

	UFUNCTION()
	virtual void OnSearchComplete();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	//virtual void SetHighlighted_Implementation(bool bHighlight) override;

	UPROPERTY()
	class USearchComponent* SearchComponent = nullptr;

};
