#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DefaultGASearch.generated.h"

class USearchAbilityTask;

UCLASS()
class ECHOESOFTIME_API UDefaultGASearch : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UDefaultGASearch();

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
    void OnSearchTaskFinished(bool bSuccess);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search|UI")
    TSubclassOf<class USearchWidget> SearchWidgetClass;

protected:
    UPROPERTY()
    USearchAbilityTask* ActiveSearchTask = nullptr;

    UPROPERTY()
    class USearchComponent* ActiveSearchComp = nullptr;
};