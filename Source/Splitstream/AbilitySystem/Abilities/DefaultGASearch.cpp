#include "DefaultGASearch.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AbilityTasks/SearchAbilityTask.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "ActorComponents/SearchComponent.h"
#include "Widgets/HUD/SearchWidget.h"

UDefaultGASearch::UDefaultGASearch()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Character_Ability_Search;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_Searching);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Movement);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Block_Look);

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TAG_Character_Ability_Search;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(TriggerData);
}

void UDefaultGASearch::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ActiveSearchComp = nullptr;
    if (TriggerEventData && TriggerEventData->OptionalObject)
    {
        AActor* TargetActor = const_cast<AActor*>(Cast<AActor>(TriggerEventData->OptionalObject));
        if (TargetActor)
        {
            ActiveSearchComp = TargetActor->FindComponentByClass<USearchComponent>();
        }
    }

    if (!ActiveSearchComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // LocalPredicted: open prediction window and start local predicted logic, server is authoritative.
    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FScopedPredictionWindow ScopedPred(ActorInfo->AbilitySystemComponent.Get(), /*bCreateNewPredictionKeyIfNotAvailable=*/true);
        ActiveSearchComp->StartSearching(); // Start predicted search (UI/progress) locally
    }
    else
    {
        if (ActorInfo && ActorInfo->IsNetAuthority())
        {
            ActiveSearchComp->StartSearching(); // Start authoritative search on the server
        }
    }

    ActiveSearchTask = USearchAbilityTask::StartSearchTask(this, ActiveSearchComp);
    ActiveSearchTask->SearchWidgetClass = SearchWidgetClass;
    ActiveSearchTask->OnFinished.AddDynamic(this, &UDefaultGASearch::OnSearchTaskFinished);
    ActiveSearchTask->ReadyForActivation();
}

void UDefaultGASearch::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    // Only cancel searching if the ability was actually cancelled (not on success)
    if (ActiveSearchComp && bWasCancelled)
    {
        ActiveSearchComp->CancelSearching();
    }
    ActiveSearchComp = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGASearch::OnSearchTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}