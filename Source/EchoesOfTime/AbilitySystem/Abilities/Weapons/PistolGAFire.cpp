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

            // --- Local Prediction: Client spawns its own bullet for instant feedback ---
            if (IsLocallyControlled()&&!HasAuthority(&ActivationInfo))
            {
                FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

                // Spawn predicted bullet
                ABullet* PredictedBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    MuzzleLocation,
                    MuzzleRotation,
                    SpawnParams
                );
                if (PredictedBullet)
                {
                    PredictedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                    // Mark as predicted (optional, up to you)
                }
            }

            // --- Server Authority: Server spawns real bullet, gets replicated ---
            if (HasAuthority(&ActivationInfo))
            {
                ABullet* ServerBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    MuzzleLocation,
                    MuzzleRotation,
                    SpawnParams
                );
                if (ServerBullet)
                {
                    ServerBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                    // Server-side only: set for replication, etc.
                }
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}