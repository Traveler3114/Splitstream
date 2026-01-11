#include "HackAbilityTask.h"
#include "ActorComponents/HackComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/HUD/HackWidget.h"
#include "Controllers/DefaultPlayerController.h"

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

    if (GetAvatarActor()->HasAuthority())
    {
        HackComp->StartHacking();
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

    if (HackWidget)
    {
        HackWidget->UpdateProgress(HackComp->GetHackProgress());
    }

    if (HackComp->bHacked)
    {
        FinishTask(true);
    }
    else if (!HackComp->bHackingInProgress)
    {
        FinishTask(false);
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
    // Unreal InputComponent handles cleanup; explicit unbinding is not needed.
}

void UHackAbilityTask::OnCancel()
{
    if (!bIsHacking) return;
    bIsHacking = false;

    if (GetAvatarActor()->HasAuthority())
    {
        if (HackComp) HackComp->CancelHacking();
    }
    OnFinished.Broadcast(false);
    EndTask();
}

void UHackAbilityTask::FinishTask(bool bSuccess)
{
    bIsHacking = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}