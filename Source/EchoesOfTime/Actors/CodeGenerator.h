// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "CodeGenerator.generated.h"

UCLASS()
class ECHOESOFTIME_API ACodeGenerator : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACodeGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Desk")
	UStaticMeshComponent* CodeGenMesh;

	// In CodeGenerator.h
	UPROPERTY(Replicated,BlueprintReadOnly)
	class ACivilianCharacter* TargetCivilian = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;


};
