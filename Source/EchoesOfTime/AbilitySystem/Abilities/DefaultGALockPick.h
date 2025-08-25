#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "DefaultGALockPick.generated.h"

UCLASS()
class ECHOESOFTIME_API UDefaultGALockPick : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UDefaultGALockPick();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;
};