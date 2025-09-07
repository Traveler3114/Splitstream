#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "LockPickAbilityTask.generated.h"

class ULockPickComponent;
class ULockPickWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLockPickTaskResult, bool, bSuccess);

UCLASS()
class ECHOESOFTIME_API ULockPickAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FLockPickTaskResult OnFinished;

    static ULockPickAbilityTask* StartLockPickTask(UGameplayAbility* OwningAbility, ULockPickComponent* InLockComp);

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

    UPROPERTY(EditAnywhere, Category = "LockPick|UI")
    TSubclassOf<ULockPickWidget> LockPickWidgetClass;

    UPROPERTY()
    ULockPickWidget* LockPickWidget = nullptr;

protected:
    UPROPERTY()
    ULockPickComponent* LockComp = nullptr;

    FVector2D LockPickInputVector = FVector2D::ZeroVector;
    float LockPickDialAngle = 0.0f;
    bool bIsLockPicking = false;

    void BindInput();
    void UnbindInput();

    void OnMouseX(float Axis);
    void OnMouseY(float Axis);
    void OnConfirm();
    void OnCancel();

    void FinishTask(bool bSuccess);

    UFUNCTION(Server, Reliable)
    void ServerConfirmPin(float Angle); // Kept for possible future use
};