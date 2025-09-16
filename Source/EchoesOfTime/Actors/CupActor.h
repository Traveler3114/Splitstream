// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/SearchableActor.h"
#include "CupActor.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API ACupActor : public ASearchableActor
{
	GENERATED_BODY()
	
public:
	ACupActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CupActor")
	UStaticMeshComponent* CupMesh;

protected:
	virtual void OnSearchComplete() override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
};
