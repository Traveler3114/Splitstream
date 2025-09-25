// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PistolGAFire.generated.h"

/**
 * 
 */
UCLASS()
class ECHOESOFTIME_API UPistolGAFire : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPistolGAFire();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class ABullet> ProjectileClass;


};
