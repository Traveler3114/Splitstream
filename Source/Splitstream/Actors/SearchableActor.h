// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "SearchableActor.generated.h"

UENUM(BlueprintType)
enum class EFloorLevel : uint8
{
	Basement     UMETA(DisplayName = "Basement"),
	FirstFloor   UMETA(DisplayName = "First Floor"),
	SecondFloor  UMETA(DisplayName = "Second Floor"),
};

UCLASS()
class SPLITSTREAM_API ASearchableActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASearchableActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor")
	EFloorLevel Floor = EFloorLevel::Basement;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupActor")
	UStaticMeshComponent* ActorMesh;

	UFUNCTION()
	virtual void OnSearchComplete();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	UItemBase* RewardItem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Searchable")
	bool bIsActivatedForPlayer = false;

	UPROPERTY()
	class ACivilianCharacter* PendingOwnerCivilian = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	bool bGivesItem = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual bool IsProgressiveInteract_Implementation() override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	class USearchComponent* SearchComponent = nullptr;





};
