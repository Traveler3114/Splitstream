#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "HackAbilityTask.generated.h"

class UHackComponent;
class UHackWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHackTaskResult, bool, bSuccess);

UCLASS()
class SPLITSTREAM_API UHackAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FHackTaskResult OnFinished;

    static UHackAbilityTask* StartHackTask(UGameplayAbility* OwningAbility, UHackComponent* InHackComp);

    void SetTimerHandle(FActiveGameplayEffectHandle InHandle) { TimerEffectHandle = InHandle; }
    void SetTaskDuration(float InDuration) { TaskDuration = InDuration; }

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

    UPROPERTY(EditAnywhere, Category = "Hack|UI")
    TSubclassOf<UHackWidget> HackWidgetClass;

    UPROPERTY()
    UHackWidget* HackWidget = nullptr;

protected:
    UPROPERTY()
    UHackComponent* HackComp = nullptr;

    FActiveGameplayEffectHandle TimerEffectHandle;

    float TaskStartTime = 0.f;
    float TaskDuration = 0.f;

    bool bIsHacking = false;

    void BindInput();
    void UnbindInput();

    void OnCancel();

    void FinishTask(bool bSuccess);
};