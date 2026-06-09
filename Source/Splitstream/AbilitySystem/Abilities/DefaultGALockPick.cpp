#include "DefaultGALockPick.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "AbilitySystem/AbilityTasks/LockPickAbilityTask.h"
#include "ActorComponents/LockPickComponent.h"
#include "Widgets/HUD/LockPickWidget.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"

UDefaultGALockPick::UDefaultGALockPick()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Character_Ability_LockPick;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_LockPicking);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Movement);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Look);

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

    ActiveLockComp->StartLockPicking();

    // Listen for pin confirmation via WaitGameplayEvent task
    UAbilityTask_WaitGameplayEvent* WaitPinTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_LockPick_PinConfirmed, nullptr, false, true);
    if (WaitPinTask)
    {
        WaitPinTask->EventReceived.AddDynamic(this, &UDefaultGALockPick::OnPinConfirmed);
        WaitPinTask->ReadyForActivation();
    }

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
    // No manual unbind needed — WaitGameplayEvent cleans up automatically on ability end

    if (ActiveLockComp)
    {
        ActiveLockComp->EndLockPicking();
        ActiveLockComp = nullptr;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGALockPick::OnPinConfirmed(FGameplayEventData EventData)
{
    if (!CurrentActorInfo->IsNetAuthority()) return;
    if (!ActiveLockComp) return;

    ULockPickComponent* LockComp = Cast<ULockPickComponent>(const_cast<UObject*>(EventData.OptionalObject.Get()));
    if (!LockComp || LockComp != ActiveLockComp) return;

    if (LockComp->TrySetCurrentPin(EventData.EventMagnitude) && LockComp->AdvancePin())
    {
        LockComp->EndLockPicking();
    }
}

void UDefaultGALockPick::OnLockPickTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}