// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/ItemBase.h"
#include "FingerprintItem.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API UFingerprintItem : public UItemBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ownership")
	TSoftObjectPtr<class ACivilianCharacter> OwnerCivilian;
	
};
