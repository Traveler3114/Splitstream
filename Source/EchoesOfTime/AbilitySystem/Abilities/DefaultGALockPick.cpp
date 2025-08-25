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
    {
        // The event should send the door actor (not the component!)
        AActor* HitActor = const_cast<AActor*>(Cast<AActor>(TriggerEventData->OptionalObject));
        if (HitActor)
        {
            // Always resolve the LockPickComponent from the actor in this context (on both client & server)
            ActiveLockComp = HitActor->FindComponentByClass<ULockPickComponent>();
        }
    }

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