#include "DefaultGAHack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "AbilitySystem/AbilityTasks/HackAbilityTask.h"
#include "ActorComponents/HackComponent.h"
#include "Widgets/HUD/HackWidget.h"

UDefaultGAHack::UDefaultGAHack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // Make this ability local-predicted so clients run ActivateAbility immediately for snappy UI.
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
        // Nothing to hack; finish ability immediately.
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // If this ability is LocalPredicted, the client will execute this function locally immediately.
    // Create a scoped prediction window on the owning ASC so this activation has a PredictionKey.
    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        // Create a scoped prediction window so this activation carries a PredictionKey.
        // This allows future predicted effects/operations to be reconciled by the server.
        FScopedPredictionWindow ScopedPred(ActorInfo->AbilitySystemComponent.Get(), /*bCreateNewPredictionKeyIfNotAvailable=*/true);

        // Start local predicted hack so UI appears instantly.
        ActiveHackComp->StartHacking();
    }
    else
    {
        // Server/authority side: start authoritative hacking (server validation can be added here).
        if (ActorInfo && ActorInfo->IsNetAuthority())
        {
            ActiveHackComp->StartHacking();
        }
    }

    // Start the ability task that manages the UI and cancellation input.
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
    if (ActiveHackComp && bWasCancelled)
    {
        ActiveHackComp->CancelHacking();
    }
    ActiveHackComp = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGAHack::OnHackTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}