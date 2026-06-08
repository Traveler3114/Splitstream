// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "DataAssets/AbilitySets/AbilityInputSet.h"
#include "DataAssets/AbilitySets/DefaultGASet.h"
#include "DefaultAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPLITSTREAM_API UDefaultAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	/** Grant abilities from an input-tagged ability set (FutureGASet, SoloGASet) */
	void GrantAbilitiesFromSet(const UAbilityInputSet* Set);

	/** Grant abilities from a default (non-input) ability set */
	void GrantAbilitiesFromDefaultSet(const UDefaultGASet* Set);

	/** Apply an attribute-initializing GameplayEffect to self (authority only) */
	void InitializeAttributes(TSubclassOf<UGameplayEffect> InitGE);

	/** Find and activate abilities matching InputTag */
	void HandleAbilityInputPressed(FGameplayTag InputTag);

	/** Find and cancel active abilities matching InputTag */
	void HandleAbilityInputReleased(FGameplayTag InputTag);

	/** Client → Server RPC: fire a gameplay event on the server's ASC */
	UFUNCTION(Server, Reliable)
	void ServerHandleClientEvent(FGameplayTag EventTag, FGameplayEventData EventData);
};
