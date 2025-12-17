// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/GuardCharacter.h"
#include "RobotGuardCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API ARobotGuardCharacter : public AGuardCharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<class ARepairableBase*> RepairQueue;

	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void OnRepairRequested(ARepairableBase* RepairableActor);
	
};
