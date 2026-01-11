#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FutureGAPastEcho.generated.h"

/**
 * Toggle PastEcho presentation via Gameplay Cues.
 * - On 1st press: Execute GameplayCue.PastEcho.Activated (show locally) and add Character.Status.PastEcho.
 * - On 2nd press: Execute GameplayCue.PastEcho.Deactivated (hide locally) and remove Character.Status.PastEcho.
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
	// State tag we toggle on/off to track "is showing"
	FGameplayTag StatusTag;

	// Cue tags for show/hide
	FGameplayTag CueActivatedTag;
	FGameplayTag CueDeactivatedTag;
};