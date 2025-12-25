#include "PistolGAFire.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/Projectiles/Bullet.h"
#include "Characters/DefaultCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

UPistolGAFire::UPistolGAFire()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

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

    ADefaultCharacter* Character = Cast<ADefaultCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);

    if (Character && Character->EquippedItemMeshComp && ProjectileClass)
    {
        FRotator MuzzleRotation = Character->EquippedItemMeshComp->GetSocketRotation(FName("Muzzle"));
        FVector SpawnLocation = Character->EquippedItemMeshComp->GetSocketLocation(FName("Muzzle"));
        FVector Forward = Character->GetActorForwardVector();
        float SafeDistance = Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 10.0f;
        SpawnLocation += Forward * SafeDistance;

        UWorld* World = Character->GetWorld();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = Character;
            SpawnParams.Instigator = Character;

            // Local Prediction Spawn (client only)
            if (IsLocallyControlled() && !HasAuthority(&ActivationInfo))
            {
                FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

                ABullet* PredictedBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    SpawnLocation,
                    MuzzleRotation,
                    SpawnParams
                );
                if (PredictedBullet)
                {
                    PredictedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                }
            }

            // Server-authoritative spawn (this will replicate)
            if (HasAuthority(&ActivationInfo))
            {
                ABullet* RealBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    SpawnLocation,
                    MuzzleRotation,
                    SpawnParams
                );
                if (RealBullet)
                {
                    RealBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                }
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}