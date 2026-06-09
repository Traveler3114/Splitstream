#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DefaultGALockPick.generated.h"

class ULockPickAbilityTask;

UCLASS()
class SPLITSTREAM_API UDefaultGALockPick : public UGameplayAbility
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

    UFUNCTION()
    void OnLockPickTaskFinished(bool bSuccess);

    UFUNCTION()
    void OnPinConfirmed(FGameplayEventData EventData);

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "LockPick|UI")
    TSubclassOf<class ULockPickWidget> LockPickWidgetClass;

protected:
    UPROPERTY()
    class ULockPickAbilityTask* ActiveLockPickTask = nullptr;

    UPROPERTY()
    class ULockPickComponent* ActiveLockComp = nullptr;
};