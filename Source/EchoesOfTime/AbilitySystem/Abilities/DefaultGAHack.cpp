#include "DefaultGAHack.h"
#include "AbilitySystem/AbilityTasks/HackAbilityTask.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "HackingSystem/HackComponent.h"
#include "Widgets/HUD/HackWidget.h"

UDefaultGAHack::UDefaultGAHack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

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

    ActiveHackTask = UHackAbilityTask::StartHackTask(this, ActiveHackComp);
    ActiveHackTask->HackWidgetClass = HackWidgetClass;
    ActiveHackTask->OnFinished.AddDynamic(this, &UDefaultGAHack::OnHackTaskFinished);
    ActiveHackTask->ReadyForActivation();
}

void UDefaultGAHack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActiveHackComp)
    {
        ActiveHackComp->CancelHacking();
        ActiveHackComp = nullptr;
    }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGAHack::OnHackTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}