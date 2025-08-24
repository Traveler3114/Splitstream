// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGALockPick.h"

UDefaultGALockPick::UDefaultGALockPick()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// Optional: asset tag for discovery/debugging
	FGameplayTagContainer Tags;
	FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.LockPick"));
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.Status.LockPicking")));
}