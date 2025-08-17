// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "FutureGAPastEcho.generated.h"

/**
 * Shows ghost locally via a Gameplay Cue.
 */
UCLASS()
class ECHOESOFTIME_API UFutureGAPastEcho : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UFutureGAPastEcho();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};