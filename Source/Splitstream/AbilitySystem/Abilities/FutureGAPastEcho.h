#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FutureGAPastEcho.generated.h"

/**
 * Toggle PastEcho ghost visibility.
 * - On 1st press: Set bIsPastEchoAbilityActive=true on all GhostCharacterActors.
 * - On 2nd press: Set bIsPastEchoAbilityActive=false on all GhostCharacterActors.
 */
UCLASS()
class SPLITSTREAM_API UFutureGAPastEcho : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UFutureGAPastEcho();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	FGameplayTag StatusTag;
	FGameplayTag CueActivatedTag;
	FGameplayTag CueDeactivatedTag;
};