#include "FutureGAPastEcho.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UFutureGAPastEcho::UFutureGAPastEcho()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// Optional: asset tag for discovery/debugging
	FGameplayTagContainer Tags;
	FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.Future.PastEcho"));
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);

	// We won't rely on ActivationOwnedTags (since we end immediately), but keeping it is harmless
	//ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.PastEcho")));

	// Initialize toggle/cue tags
	StatusTag = FGameplayTag::RequestGameplayTag(FName("Character.Status.PastEcho"));
	CueActivatedTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.PastEcho.Activated"));
	CueDeactivatedTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.PastEcho.Deactivated"));
}

void UFutureGAPastEcho::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/false, /*bWasCancelled*/true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	// Toggle based on presence of the status tag
	const bool bActive = ASC->HasMatchingGameplayTag(StatusTag);
	FGameplayCueParameters Params; // add parameters later if needed

	if (!bActive)
	{
		// First press: mark active and show locally via "Activated" cue
		ASC->AddLooseGameplayTag(StatusTag);
		ASC->ExecuteGameplayCue(CueActivatedTag, Params);
	}
	else
	{
		// Second press: clear and hide locally via "Deactivated" cue
		ASC->RemoveLooseGameplayTag(StatusTag);
		ASC->ExecuteGameplayCue(CueDeactivatedTag, Params);
	}

	// One-shot ability; end immediately
	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/false, /*bWasCancelled*/false);
}

void UFutureGAPastEcho::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}