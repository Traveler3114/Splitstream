#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DefaultGAHack.generated.h"

class UHackAbilityTask;

UCLASS()
class ECHOESOFTIME_API UDefaultGAHack : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UDefaultGAHack();

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
    void OnHackTaskFinished(bool bSuccess);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hack|UI")
    TSubclassOf<class UHackWidget> HackWidgetClass;

protected:
    UPROPERTY()
    UHackAbilityTask* ActiveHackTask = nullptr;

    UPROPERTY()
    class UHackComponent* ActiveHackComp = nullptr;
};