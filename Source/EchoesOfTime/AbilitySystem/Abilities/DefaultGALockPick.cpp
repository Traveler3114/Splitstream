// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultGALockPick.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "GameFramework/Actor.h"

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


	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.LockPick"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}


void UDefaultGALockPick::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Get the targeted lock
    ULockPickComponent* LockComp = nullptr;
    if (TriggerEventData && TriggerEventData->OptionalObject)
    {
        LockComp = const_cast<ULockPickComponent*>(Cast<ULockPickComponent>(TriggerEventData->OptionalObject));
    }
    if (!LockComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Start lockpicking logic
    LockComp->StartLockPicking();

    // Show lockpicking UI here (call Blueprint event, widget, etc.)

    // Save a pointer if you want to reference LockComp later (end/cancel)
    // MyLockComp = LockComp;

    // Wait for player input, call TrySetCurrentPin(...) etc from UI
}