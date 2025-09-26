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

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;
};