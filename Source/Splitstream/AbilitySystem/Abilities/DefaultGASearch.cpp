#include "DefaultGASearch.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GE_Timer.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "AbilitySystem/AbilityTasks/SearchAbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
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

    if (CurrentActorInfo->IsNetAuthority())
    {
        if (TriggerEventData && TriggerEventData->Instigator)
        {
            AActor* InstigatorPtr = const_cast<AActor*>(TriggerEventData->Instigator.Get());
            ActiveSearchComp->LastInteractor = InstigatorPtr;
        }
        if (ActiveSearchComp->bAllowMultipleSearches)
        {
            ActiveSearchComp->bSearched = false;
        }
    }

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UGE_Timer::StaticClass());
    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Duration, ActiveSearchComp->SearchDuration);
        ActiveTimerHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
    }

    UAbilityTask_WaitGameplayEffectRemoved* WaitTask = UAbilityTask_WaitGameplayEffectRemoved::WaitForGameplayEffectRemoved(this, ActiveTimerHandle);
    if (WaitTask)
    {
        WaitTask->OnRemoved.AddDynamic(this, &UDefaultGASearch::OnTimerRemoved);
        WaitTask->ReadyForActivation();
    }

    ActiveSearchTask = USearchAbilityTask::StartSearchTask(this, ActiveSearchComp);
    ActiveSearchTask->SearchWidgetClass = SearchWidgetClass;
    ActiveSearchTask->SetTimerHandle(ActiveTimerHandle);
    ActiveSearchTask->SetTaskDuration(ActiveSearchComp->SearchDuration);
    ActiveSearchTask->OnFinished.AddDynamic(this, &UDefaultGASearch::OnSearchTaskFinished);
    ActiveSearchTask->ReadyForActivation();
}

void UDefaultGASearch::EndAbility(
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
    ActiveSearchComp = nullptr;
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDefaultGASearch::OnTimerRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
    if (!RemovalInfo.bPrematureRemoval && ActiveSearchComp && CurrentActorInfo->IsNetAuthority())
    {
        ActiveSearchComp->SetSearched();
    }
}

void UDefaultGASearch::OnSearchTaskFinished(bool bSuccess)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, !bSuccess);
}