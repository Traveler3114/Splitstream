#include "PistolGAFire.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/Projectiles/Bullet.h"
#include "Characters/DefaultCharacter.h"

UPistolGAFire::UPistolGAFire()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
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
    if (!Character || !Character->HasAuthority())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // Get muzzle location and rotation from the equipped item mesh
    FVector MuzzleLocation = Character->EquippedItemMeshComp->GetSocketLocation(FName("Muzzle"));
    FRotator MuzzleRotation = Character->EquippedItemMeshComp->GetSocketRotation(FName("Muzzle"));

    // Optional: For true FPS, use camera rotation for bullet direction
    // FRotator MuzzleRotation = Character->GetControlRotation();

    UWorld* World = Character->GetWorld();
    if (World && ProjectileClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Character;
        SpawnParams.Instigator = Character;

        // Spawn the bullet and capture the pointer
        ABullet* SpawnedBullet = World->SpawnActor<ABullet>(
            ProjectileClass,
            MuzzleLocation,
            MuzzleRotation,
            SpawnParams
        );

        if (SpawnedBullet)
        {
            // Call the helper function instead of manipulating collision directly
            SpawnedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}