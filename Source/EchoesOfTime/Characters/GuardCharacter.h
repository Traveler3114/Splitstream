// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuardCharacter.generated.h"


class ANavNode;
UCLASS()
class ECHOESOFTIME_API AGuardCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGuardCharacter();

	// Base probability to idle when allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	float BaseStayChance = 0.5f; // 0..1

	// How long this AI tends to idle
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Idle")
	float MinIdleTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	float MaxIdleTime = 3.0f;

	// If true, only consider nodes marked bStayPoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	bool bOnlyStayOnMarkedNodes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
	ANavNode* CurrentNode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
	ANavNode* PreviousNode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nodes")
	ANavNode* NextNode = nullptr;

};
