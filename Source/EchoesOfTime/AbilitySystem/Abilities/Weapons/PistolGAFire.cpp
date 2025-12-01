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
    UE_LOG(LogTemp, Warning, TEXT("PistolGAFire::ActivateAbility called on %s"),
        HasAuthority(&ActivationInfo) ? TEXT("SERVER") : TEXT("CLIENT"));

    // Local prediction window for responsive input
    if (IsLocallyControlled() && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Prediction: IsLocallyControlled TRUE -- running client local fire logic"));

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

                UE_LOG(LogTemp, Warning, TEXT("CLIENT [%s] Spawning bullet actor: location [%s], rotation [%s]"),
                    *Character->GetName(),
                    *MuzzleLocation.ToString(), *MuzzleRotation.ToString());

                ABullet* SpawnedBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    MuzzleLocation,
                    MuzzleRotation,
                    SpawnParams
                );

                if (SpawnedBullet)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Spawned bullet successfully on CLIENT [%s]"), *Character->GetName());
                    SpawnedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("CLIENT [%s]: Bullet spawn FAILED!"), *Character->GetName());
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("CLIENT: Character or mesh or ProjectileClass NOT valid, cannot spawn bullet"));
        }
    }

    // Spawn bullet on SERVER for all networked players
    if (HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Authority: Server spawning bullet for network sync"));

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

                UE_LOG(LogTemp, Warning, TEXT("SERVER [%s] Spawning bullet actor: location [%s], rotation [%s]"),
                    *Character->GetName(),
                    *MuzzleLocation.ToString(), *MuzzleRotation.ToString());

                ABullet* SpawnedBullet = World->SpawnActor<ABullet>(
                    ProjectileClass,
                    MuzzleLocation,
                    MuzzleRotation,
                    SpawnParams
                );

                if (SpawnedBullet)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Spawned bullet successfully on SERVER [%s]"), *Character->GetName());
                    SpawnedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("SERVER [%s]: Bullet spawn FAILED!"), *Character->GetName());
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SERVER: Character or mesh or ProjectileClass NOT valid, cannot spawn bullet"));
        }
    }

    // End ability immediately (predicted and server)
    UE_LOG(LogTemp, Warning, TEXT("PistolGAFire: Ending ability on %s"),
        HasAuthority(&ActivationInfo) ? TEXT("SERVER") : TEXT("CLIENT"));

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}