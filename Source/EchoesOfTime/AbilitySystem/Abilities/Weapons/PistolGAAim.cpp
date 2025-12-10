#include "PistolGAAim.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Characters/DefaultCharacter.h"

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

void UPistolGAAim::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

        // Get the character
        if (ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get()))
        {
            //FVector AimLocation(9.960482f, 15.432522f, 1.7f);
            //FRotator AimRotation(-14.932470f, 62.527103f, -102.804844f);
            FVector AimLocation = Character->CameraAimLocation - AimOffset;
            FRotator AimRotation = Character->CameraAimRotation;
            Character->StartAimCamera(AimLocation, AimRotation);
        }
    }
}

void UPistolGAAim::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get()))
    {
        FVector ReturnLocation = Character->CameraDefaultLocation;
        FRotator ReturnRotation = Character->CameraDefaultRotation;
        Character->StopAimCamera(ReturnLocation, ReturnRotation);
    }
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}