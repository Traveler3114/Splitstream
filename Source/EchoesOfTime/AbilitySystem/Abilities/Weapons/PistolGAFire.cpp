// Fill out your copyright notice in the Description page of Project Settings.


#include "PistolGAFire.h"
#include "AbilitySystem/EOTGameplayTags.h"


UPistolGAFire::UPistolGAFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	FGameplayTagContainer Tags;
	FGameplayTag MyTag = TAG_Weapon_Ability_Pistol_Fire;
	Tags.AddTag(MyTag);
	SetAssetTags(Tags);
	ActivationOwnedTags.AddTag(TAG_Character_Status_Firing);
	ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
	//FAbilityTriggerData TriggerData;
	//TriggerData.TriggerTag = TAG_Weapon_Ability_Pistol_Fire;
	//TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	//AbilityTriggers.Add(TriggerData);
}

