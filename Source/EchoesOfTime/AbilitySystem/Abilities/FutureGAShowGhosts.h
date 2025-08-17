// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "FutureGAShowGhosts.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API UFutureGAShowGhosts : public UGameplayAbility
{
	GENERATED_BODY()
public:

	UFutureGAShowGhosts();
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
};
