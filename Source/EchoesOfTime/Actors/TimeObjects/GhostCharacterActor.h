// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostCharacterActor.generated.h"

UCLASS()
class ECHOESOFTIME_API AGhostCharacterActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGhostCharacterActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Root
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	// Visible mesh for the ghost
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* GhostMesh;

	// Character whose pose we mirror (replicated)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "GhostCharacter")
	ACharacter* CharacterToMirror;

	// Material to apply on the ghost (replicated)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	UMaterialInterface* GhostMaterial = nullptr;

};