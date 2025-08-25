#include "DefaultGALockPick.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
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

    ULockPickComponent* LockComp = nullptr;
    if (TriggerEventData && TriggerEventData->OptionalObject)
        LockComp = const_cast<ULockPickComponent*>(Cast<ULockPickComponent>(TriggerEventData->OptionalObject));
    if (!LockComp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    LockComp->StartLockPicking();

    // Just call directly on the controller!
    if (APlayerController* PC = Cast<APlayerController>(ActorInfo->PlayerController.Get()))
    {
        if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
        {
            MyPC->StartLockPickMinigame(LockComp);
        }
    }
}

void UDefaultGALockPick::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    // Optional: You could call EndLockPickMinigame here, but you probably want the controller to do it itself.
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}