#include "PistolGAAim.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Characters/DefaultCharacter.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"

UPistolGAAim::UPistolGAAim()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; // LOCAL PREDICTED

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Weapon_Ability_Pistol_Aim;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_Aiming);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
}

void UPistolGAAim::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Local prediction window for responsive input
    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

        ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get());
        if (Character && Character->AimCameraTimeline)
        {
            Character->AimCameraTimeline->Play();
        }
    }
}

void UPistolGAAim::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get());
    if (Character && Character->AimCameraTimeline)
    {
        Character->AimCameraTimeline->Reverse();
    }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}