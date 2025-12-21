#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SoloGATeleport.generated.h"

UCLASS()
class ECHOESOFTIME_API USoloGATeleport : public UGameplayAbility
{
    GENERATED_BODY()

public:
    USoloGATeleport();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};