#include "LockPickAbilityTask.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "Widgets/HUD/LockPickWidget.h"
#include "Controllers/DefaultPlayerController.h"

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


    // Only the server should reset and start lockpicking!
    if (GetAvatarActor()->HasAuthority())
    {
        LockComp->StartLockPicking();
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
                    LockPickWidget->AddToViewport();
                }
            }
        }
    }

    LockPickInputVector = FVector2D::ZeroVector;
    LockPickDialAngle = 0.0f;
    bIsLockPicking = true;

    BindInput();

    bTickingTask = true; // Enable ticking
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

    // --- ADD THIS BLOCK ---
    if (LockPickWidget)
    {
        LockPickWidget->UpdatePins(LockPickDialAngle);
    }
    // ----------------------

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
    // Unreal handles InputComponent cleanup; explicit unbinding is unnecessary.
}

void ULockPickAbilityTask::OnMouseX(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.X += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnMouseY(float Axis)
{
    if (!bIsLockPicking) return;
    LockPickInputVector.Y += Axis * 2.0f;
    LockPickInputVector = LockPickInputVector.GetClampedToMaxSize(150.f);
    LockPickDialAngle = FMath::Atan2(LockPickInputVector.Y, LockPickInputVector.X) * (180.f / PI);
    if (LockPickDialAngle < 0) LockPickDialAngle += 360.f;
}

void ULockPickAbilityTask::OnConfirm()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn) return;
    ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(Pawn->GetController());
    if (!MyPC || !LockComp) return;

    MyPC->ServerLockPickConfirm(LockComp->GetOwner(), LockPickDialAngle);
}

void ULockPickAbilityTask::ServerConfirmPin_Implementation(float Angle)
{
    // Not used in current architecture, but left for possible future direct server RPCs
    if (!LockComp || !bIsLockPicking) return;
    bool bCorrect = LockComp->TrySetCurrentPin(Angle);
    if (bCorrect)
    {
        bool bUnlocked = LockComp->AdvancePin();
        if (bUnlocked)
        {
            LockComp->EndLockPicking();
        }
    }
}

void ULockPickAbilityTask::OnCancel()
{
    if (!bIsLockPicking) return;
    bIsLockPicking = false;

    // Only the server should end picking for everyone
    if (GetAvatarActor()->HasAuthority())
    {
        if (LockComp) LockComp->EndLockPicking();
    }
    OnFinished.Broadcast(false);
    EndTask();
}

void ULockPickAbilityTask::FinishTask(bool bSuccess)
{
    bIsLockPicking = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}
