#include "PistolGAAim.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Characters/DefaultCharacter.h"
#include "Camera/CameraComponent.h"

UPistolGAAim::UPistolGAAim()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

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
    ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get());
	Character->CameraComponent->SetRelativeLocation(FVector(9.960482f, 15.432522f, 1.7f));
    Character->CameraComponent->SetRelativeRotation(FRotator(-14.932470f, 62.527103f, -102.804844f));
}

void UPistolGAAim::InputReleased(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get());
    Character->CameraComponent->SetRelativeLocation(FVector(0, 0, 0));
    Character->CameraComponent->SetRelativeRotation(FRotator(0, 0, 0));
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // Replicate, not cancelled
}