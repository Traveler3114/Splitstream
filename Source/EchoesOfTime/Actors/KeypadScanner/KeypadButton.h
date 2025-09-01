// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeypadButton.generated.h"

UCLASS()
class ECHOESOFTIME_API AKeypadButton : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKeypadButton();

	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "KeypadButton")
	USceneComponent* DefaultSceneRoot;

	// Static mesh child
	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "KeypadButton")
	UStaticMeshComponent* KeypadButtonMesh;

	// Text render child
	UPROPERTY(EditAnywhere, BlueprintReadwrite, Category = "KeypadButton")
	UTextRenderComponent* NumberTextRenderComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
