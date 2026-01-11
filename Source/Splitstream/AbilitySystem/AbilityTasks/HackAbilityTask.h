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

    bool bIsHacking = false;

    void BindInput();
    void UnbindInput();

    void OnCancel();

    void FinishTask(bool bSuccess);
};