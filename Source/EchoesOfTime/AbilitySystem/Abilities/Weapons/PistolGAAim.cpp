// Fill out your copyright notice in the Description page of Project Settings.


#include "PistolGAAim.h"
#include "AbilitySystem/EOTGameplayTags.h"

UPistolGAAim::UPistolGAAim()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	FGameplayTagContainer Tags;
	FGameplayTag MyTag = TAG_Weapon_Ability_Pistol_Aim;
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);
	ActivationOwnedTags.AddTag(TAG_Character_Status_Aiming);
	ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
	//FAbilityTriggerData TriggerData;
	//TriggerData.TriggerTag = TAG_Weapon_Ability_Pistol_Aim;
	//TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	//AbilityTriggers.Add(TriggerData);
}

