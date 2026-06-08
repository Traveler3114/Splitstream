#include "HackAbilityTask.h"
#include "ActorComponents/HackComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "Widgets/HUD/HackWidget.h"

UHackAbilityTask* UHackAbilityTask::StartHackTask(UGameplayAbility* OwningAbility, UHackComponent* InHackComp)
{
    UHackAbilityTask* Task = NewAbilityTask<UHackAbilityTask>(OwningAbility);
    Task->HackComp = InHackComp;
    return Task;
}

void UHackAbilityTask::Activate()
{
    if (!HackComp)
    {
        FinishTask(false);
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(GetAvatarActor()->GetInstigatorController()))
    {
        if (HackWidgetClass && !HackWidget)
        {
            if (IsLocallyControlled())
            {
                HackWidget = CreateWidget<UHackWidget>(PC, HackWidgetClass);
                if (HackWidget)
                {
                    HackWidget->InitializeHackWidget(HackComp);
                    HackWidget->AddToViewport();
                }
            }
        }
    }

    TaskStartTime = GetWorld()->GetTimeSeconds();
    bIsHacking = true;
    BindInput();
    bTickingTask = true;
}

void UHackAbilityTask::OnDestroy(bool bInOwnerFinished)
{
    UnbindInput();
    bTickingTask = false;
    bIsHacking = false;

    if (HackWidget)
    {
        HackWidget->RemoveFromParent();
        HackWidget = nullptr;
    }
    Super::OnDestroy(bInOwnerFinished);
}

void UHackAbilityTask::TickTask(float DeltaTime)
{
    if (!bIsHacking || !HackComp)
    {
        FinishTask(false);
        return;
    }

    if (HackWidget && TaskDuration > 0.f)
    {
        float Elapsed = GetWorld()->GetTimeSeconds() - TaskStartTime;
        float Progress = FMath::Clamp(Elapsed / TaskDuration, 0.f, 1.f);
        HackWidget->UpdateProgress(Progress);
    }

    if (HackComp->bHacked)
    {
        FinishTask(true);
    }
}

void UHackAbilityTask::BindInput()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn || !Pawn->InputComponent)
        return;

    Pawn->InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &UHackAbilityTask::OnCancel);
}

void UHackAbilityTask::UnbindInput()
{
}

void UHackAbilityTask::OnCancel()
{
    if (!bIsHacking) return;
    bIsHacking = false;

    OnFinished.Broadcast(false);
    EndTask();
}

void UHackAbilityTask::FinishTask(bool bSuccess)
{
    bIsHacking = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}