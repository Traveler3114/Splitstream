#include "DefaultGALockPick.h"
#include "AbilitySystem/AbilityTasks/LockPickAbilityTask.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Widgets/HUD/LockPickWidget.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Engine/Engine.h"

UDefaultGALockPick::UDefaultGALockPick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Character_Ability_LockPick;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_LockPicking);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TAG_Character_Ability_LockPick;
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
        // Always resolve the actor/component in *this* context!
        AActor* HitActor = const_cast<AActor*>(Cast<AActor>(TriggerEventData->OptionalObject));
        if (HitActor)
        {
            ActiveLockComp = HitActor->FindComponentByClass<ULockPickComponent>();
        }
    }

    if (!ActiveLockComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // THIS WILL RUN ON BOTH CLIENT AND SERVER DUE TO GAS REPLICATION
    ActiveLockPickTask = ULockPickAbilityTask::StartLockPickTask(this, ActiveLockComp);
	ActiveLockPickTask->LockPickWidgetClass = LockPickWidgetClass;
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