#include "DefaultGALockPick.h"
#include "AbilitySystem/AbilityTasks/LockPickAbilityTask.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Engine/Engine.h"

UDefaultGALockPick::UDefaultGALockPick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

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

    ActiveLockComp = nullptr;
    if (TriggerEventData && TriggerEventData->OptionalObject)
        ActiveLockComp = const_cast<ULockPickComponent*>(Cast<ULockPickComponent>(TriggerEventData->OptionalObject));
    if (!ActiveLockComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Start the ability task (no longer needs a PlayerController)
    ActiveLockPickTask = ULockPickAbilityTask::StartLockPickTask(this, ActiveLockComp);
    ActiveLockPickTask->OnFinished.AddDynamic(this, &UDefaultGALockPick::OnLockPickTaskFinished);
    ActiveLockPickTask->ReadyForActivation();
}

void UDefaultGALockPick::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActiveLockComp)
    {
        ActiveLockComp->EndLockPicking();
        ActiveLockComp = nullptr;
    }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGALockPick::OnLockPickTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}