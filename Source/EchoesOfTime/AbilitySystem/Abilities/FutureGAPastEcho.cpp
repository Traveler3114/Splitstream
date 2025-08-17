#include "FutureGAPastEcho.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UFutureGAPastEcho::UFutureGAPastEcho()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FGameplayTagContainer Tags;
	FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.Future.PastEcho"));
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.PastEcho")));
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

	// Execute Gameplay Cue; cue will handle local-only visibility on the owning client.
	const FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.PastEcho.Activated"));
	FGameplayCueParameters Params; // Add custom params later if needed
	ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(CueTag, Params);

	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/false, /*bWasCancelled*/false);
}