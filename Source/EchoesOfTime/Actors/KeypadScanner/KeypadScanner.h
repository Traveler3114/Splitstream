// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeypadScanner.generated.h"

UCLASS()
class ECHOESOFTIME_API AKeypadScanner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeypadScanner();

	UPROPERTY(EditAnywhere,BlueprintReadwrite,Category="KeypadScanner")
	USceneComponent* DefaultSceneRoot;

	// Static mesh child
	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "KeypadScanner")
	UStaticMeshComponent* KeypadScannerMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
