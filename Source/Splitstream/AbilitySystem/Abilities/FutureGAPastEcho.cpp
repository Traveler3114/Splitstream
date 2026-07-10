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
		EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	const bool bActive = ASC->HasMatchingGameplayTag(StatusTag);
	FGameplayCueParameters Params;

	if (!bActive)
	{
		ASC->AddLooseGameplayTag(StatusTag);
		ASC->ExecuteGameplayCue(CueActivatedTag, Params);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(StatusTag);
		ASC->ExecuteGameplayCue(CueDeactivatedTag, Params);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
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
