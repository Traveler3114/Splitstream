#include "DefaultGAHack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GE_Timer.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "AbilitySystem/AbilityTasks/HackAbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
#include "ActorComponents/HackComponent.h"
#include "Widgets/HUD/HackWidget.h"

UDefaultGAHack::UDefaultGAHack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Character_Ability_Hack;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_Hacking);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Movement);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Look);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TAG_Character_Ability_Hack;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);
}

void UDefaultGAHack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ActiveHackComp = nullptr;
    if (TriggerEventData && TriggerEventData->OptionalObject)
    {
        AActor* TargetActor = const_cast<AActor*>(Cast<AActor>(TriggerEventData->OptionalObject));
        if (TargetActor)
        {
            ActiveHackComp = TargetActor->FindComponentByClass<UHackComponent>();
        }
    }

    if (!ActiveHackComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Apply GE_Timer with SetByCaller duration
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UGE_Timer::StaticClass());
    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(TAG_Effect_Timer, ActiveHackComp->HackDuration);
        ActiveTimerHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
    }

    // Wait for GE removal to detect natural expiry vs cancel
    UAbilityTask_WaitGameplayEffectRemoved* WaitTask = UAbilityTask_WaitGameplayEffectRemoved::WaitForGameplayEffectRemoved(this, ActiveTimerHandle);
    if (WaitTask)
    {
        WaitTask->OnRemoved.AddDynamic(this, &UDefaultGAHack::OnTimerRemoved);
        WaitTask->ReadyForActivation();
    }

    ActiveHackTask = UHackAbilityTask::StartHackTask(this, ActiveHackComp);
    ActiveHackTask->HackWidgetClass = HackWidgetClass;
    ActiveHackTask->SetTimerHandle(ActiveTimerHandle);
    ActiveHackTask->SetTaskDuration(ActiveHackComp->HackDuration);
    ActiveHackTask->OnFinished.AddDynamic(this, &UDefaultGAHack::OnHackTaskFinished);
    ActiveHackTask->ReadyForActivation();
}

void UDefaultGAHack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActiveTimerHandle.IsValid() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveTimerHandle);
    }
    ActiveTimerHandle.Invalidate();
    ActiveHackComp = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGAHack::OnTimerRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
    ActiveTimerHandle.Invalidate();
    if (!RemovalInfo.bPrematureRemoval && ActiveHackComp && CurrentActorInfo->IsNetAuthority())
    {
        ActiveHackComp->SetHacked();
    }
}

void UDefaultGAHack::OnHackTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}