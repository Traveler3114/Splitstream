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

	void SetIsPastEchoAbilityActive(bool bActive);

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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	FVector GhostOffset = FVector(0.0f, 0.-4190.0f, -80.0f);

	// Whether the Past Echo ability is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost")
	bool bIsPastEchoAbilityActive = false;

	// Call this whenever either bool changes to update visibility
	UFUNCTION(BlueprintCallable, Category = "Ghost")
	void UpdateGhostVisibility();
};