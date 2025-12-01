#include "PistolGAFire.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/Projectiles/Bullet.h"
#include "Characters/DefaultCharacter.h"
#include "AbilitySystemComponent.h"

UPistolGAFire::UPistolGAFire()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; // LOCAL PREDICTED

    FGameplayTagContainer Tags;
    FGameplayTag MyTag = TAG_Weapon_Ability_Pistol_Fire;
    Tags.AddTag(MyTag);
    SetAssetTags(Tags);

    ActivationOwnedTags.AddTag(TAG_Character_Status_Firing);
    ActivationOwnedTags.AddTag(TAG_Character_Status_Illegal_Action);
}

void UPistolGAFire::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    // Local prediction window for responsive input
    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

        ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo->AvatarActor.Get());
        if (Character && Character->EquippedItemMeshComp && ProjectileClass)
        {
            FVector MuzzleLocation = Character->EquippedItemMeshComp->GetSocketLocation(FName("Muzzle"));
            FRotator MuzzleRotation = Character->EquippedItemMeshComp->GetSocketRotation(FName("Muzzle"));
            UWorld* World = Character->GetWorld();
            if (World)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = Character;
                SpawnParams.Instigator = Character;

                ABullet* SpawnedBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    MuzzleLocation,
                    MuzzleRotation,
                    SpawnParams
                );

                if (SpawnedBullet)
                {
                    SpawnedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                }
            }
        }
    }

    // End ability immediately (predicted and server)
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}