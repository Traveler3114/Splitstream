#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PistolGAAim.generated.h"

UCLASS()
class ECHOESOFTIME_API UPistolGAAim : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UPistolGAAim();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility = true,
		bool bWasCancelled = false) override;
};