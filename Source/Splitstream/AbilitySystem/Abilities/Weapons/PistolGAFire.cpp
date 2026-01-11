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
	if (!Character || !Character->EquippedItemMeshComp || !ProjectileClass) return;

	// Compute transform ONCE, use for both client and server
	const FName SocketName = FName("Muzzle");
	FVector MuzzleLoc = Character->EquippedItemMeshComp->GetSocketLocation(SocketName);
	FRotator MuzzleRot = Character->EquippedItemMeshComp->GetSocketRotation(SocketName);
	FVector Forward = MuzzleRot.Vector();
	float SafeDistance = Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius() + 10.f;
	FVector SpawnLocation = MuzzleLoc + Forward * SafeDistance;
	FRotator SpawnRotation = MuzzleRot;

	UWorld* World = Character->GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	// Local prediction (only on the controlling client)
	if (IsLocallyControlled() && !HasAuthority(&ActivationInfo))
	{
		FScopedPredictionWindow ScopedPrediction(ActorInfo->AbilitySystemComponent.Get(), true);

		ABullet* PredictedBullet = World->SpawnActor<ABullet>(
			ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (PredictedBullet)
		{
			PredictedBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
		}
	}

	// Only ever spawn on server with the provided transform, no local recomputation
	if (IsLocallyControlled())
	{
		ServerSpawnProjectile(SpawnLocation, SpawnRotation);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPistolGAFire::ServerSpawnProjectile_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	ADefaultCharacter* Character = Cast<ADefaultCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !ProjectileClass) return;

	UWorld* World = Character->GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	ABullet* ServerBullet = World->SpawnActor<ABullet>(
		ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (ServerBullet)
		ServerBullet->SetIgnoreActorsAndComponents(Character, Character->EquippedItemMeshComp);
}