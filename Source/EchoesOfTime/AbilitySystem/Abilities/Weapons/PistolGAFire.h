#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PistolGAFire.generated.h"

UCLASS()
class ECHOESOFTIME_API UPistolGAFire : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UPistolGAFire();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class ABullet> ProjectileClass;

	// Explicit RPC for projectile spawn transform sync
	UFUNCTION(Server, Reliable)
	void ServerSpawnProjectile(const FVector& SpawnLocation, const FRotator& SpawnRotation);
};