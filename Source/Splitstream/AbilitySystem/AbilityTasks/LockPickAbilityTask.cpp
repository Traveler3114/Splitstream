#include "LockPickAbilityTask.h"
#include "ActorComponents/LockPickComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "Widgets/HUD/LockPickWidget.h"

ULockPickAbilityTask* ULockPickAbilityTask::StartLockPickTask(UGameplayAbility* OwningAbility, ULockPickComponent* InLockComp)
{
    ULockPickAbilityTask* Task = NewAbilityTask<ULockPickAbilityTask>(OwningAbility);
    Task->LockComp = InLockComp;
    return Task;
}

void ULockPickAbilityTask::Activate()
{
    if (!LockComp)
    {
        FinishTask(false);
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(GetAvatarActor()->GetInstigatorController()))
    {
        if (LockPickWidgetClass && !LockPickWidget)
        {
            if (IsLocallyControlled())
            {
                LockPickWidget = CreateWidget<ULockPickWidget>(PC, LockPickWidgetClass);
                if (LockPickWidget)
                {
                    LockPickWidget->InitializeLockPickWidget(LockComp);
                    LockPickWidget->AddToViewport(1000);
                }
            }
        }
    }

    LockPickInputVector = FVector2D::ZeroVector;
    LockPickDialAngle = 0.0f;
    bIsLockPicking = true;

    BindInput();
    bTickingTask = true;
}

void ULockPickAbilityTask::OnDestroy(bool bInOwnerFinished)
{
    UnbindInput();
    bTickingTask = false;
    bIsLockPicking = false;

    if (LockPickWidget)
    {
        LockPickWidget->RemoveFromParent();
        LockPickWidget = nullptr;
    }
    Super::OnDestroy(bInOwnerFinished);
}

void ULockPickAbilityTask::TickTask(float DeltaTime)
{
    if (!bIsLockPicking || !LockComp)
    {
        FinishTask(false);
        return;
    }

    if (LockPickWidget)
    {
        LockPickWidget->UpdatePins(LockPickDialAngle);
    }

    if (LockComp->bUnlocked)
    {
        FinishTask(true);
    }
    else if (!LockComp->bPickingInProgress)
    {
        FinishTask(false);
    }
}

void ULockPickAbilityTask::BindInput()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn || !Pawn->InputComponent)
        return;

    Pawn->InputComponent->BindAxisKey(EKeys::MouseX, this, &ULockPickAbilityTask::OnMouseX);
    Pawn->InputComponent->BindAxisKey(EKeys::MouseY, this, &ULockPickAbilityTask::OnMouseY);
    Pawn->InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ULockPickAbilityTask::OnConfirm);
    Pawn->InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ULockPickAbilityTask::OnCancel);
}

void ULockPickAbilityTask::UnbindInput()
{
}

void ULockPickAbilityTask::OnMouseX(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.X += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = - FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnMouseY(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.Y += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = - FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnConfirm()
{
    if (!LockComp || !bIsLockPicking) return;

    FGameplayEventData EventData;
    EventData.EventMagnitude = LockPickDialAngle;
    EventData.OptionalObject = LockComp;
    EventData.Instigator = GetAvatarActor();

    if (GetAvatarActor()->HasAuthority())
    {
        if (AbilitySystemComponent.IsValid())
        {
            AbilitySystemComponent->HandleGameplayEvent(TAG_LockPick_PinConfirmed, &EventData);
        }
    }
    else if (UDefaultAbilitySystemComponent* DefaultASC = Cast<UDefaultAbilitySystemComponent>(AbilitySystemComponent))
    {
        DefaultASC->ServerHandleClientEvent(TAG_LockPick_PinConfirmed, EventData);
    }
}

void ULockPickAbilityTask::OnCancel()
{
    if (!bIsLockPicking) return;
    bIsLockPicking = false;

    OnFinished.Broadcast(false);
    EndTask();
}

void ULockPickAbilityTask::FinishTask(bool bSuccess)
{
    bIsLockPicking = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}