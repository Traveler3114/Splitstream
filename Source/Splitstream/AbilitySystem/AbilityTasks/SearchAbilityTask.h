#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SearchAbilityTask.generated.h"

class USearchComponent;
class USearchWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchTaskResult, bool, bSuccess);

UCLASS()
class SPLITSTREAM_API USearchAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FSearchTaskResult OnFinished;

    static USearchAbilityTask* StartSearchTask(UGameplayAbility* OwningAbility, USearchComponent* InSearchComp);

    void SetTimerHandle(FActiveGameplayEffectHandle InHandle) { TimerEffectHandle = InHandle; }
    void SetTaskDuration(float InDuration) { TaskDuration = InDuration; }

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

    UPROPERTY(EditAnywhere, Category = "Search|UI")
    TSubclassOf<USearchWidget> SearchWidgetClass;

    UPROPERTY()
    USearchWidget* SearchWidget = nullptr;

protected:
    UPROPERTY()
    USearchComponent* SearchComp = nullptr;

    FActiveGameplayEffectHandle TimerEffectHandle;

    float TaskStartTime = 0.f;
    float TaskDuration = 0.f;

    bool bIsSearching = false;

    void BindInput();
    void UnbindInput();

    void OnCancel();

    void FinishTask(bool bSuccess);
};