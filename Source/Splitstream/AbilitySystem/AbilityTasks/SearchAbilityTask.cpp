#include "SearchAbilityTask.h"
#include "ActorComponents/SearchComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "Widgets/HUD/SearchWidget.h"

USearchAbilityTask* USearchAbilityTask::StartSearchTask(UGameplayAbility* OwningAbility, USearchComponent* InSearchComp)
{
    USearchAbilityTask* Task = NewAbilityTask<USearchAbilityTask>(OwningAbility);
    Task->SearchComp = InSearchComp;
    return Task;
}

void USearchAbilityTask::Activate()
{
    if (!SearchComp)
    {
        FinishTask(false);
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(GetAvatarActor()->GetInstigatorController()))
    {
        if (SearchWidgetClass && !SearchWidget)
        {
            if (IsLocallyControlled())
            {
                SearchWidget = CreateWidget<USearchWidget>(PC, SearchWidgetClass);
                if (SearchWidget)
                {
                    SearchWidget->InitializeSearchWidget(SearchComp);
                    SearchWidget->AddToViewport();
                }
            }
        }
    }

    TaskStartTime = GetWorld()->GetTimeSeconds();
    bIsSearching = true;
    BindInput();
    bTickingTask = true;
}

void USearchAbilityTask::OnDestroy(bool bInOwnerFinished)
{
    UnbindInput();
    bTickingTask = false;
    bIsSearching = false;

    if (SearchWidget)
    {
        SearchWidget->RemoveFromParent();
        SearchWidget = nullptr;
    }
    Super::OnDestroy(bInOwnerFinished);
}

void USearchAbilityTask::TickTask(float DeltaTime)
{
    if (!bIsSearching || !SearchComp)
    {
        FinishTask(false);
        return;
    }

    if (SearchWidget && TaskDuration > 0.f)
    {
        float Elapsed = GetWorld()->GetTimeSeconds() - TaskStartTime;
        float Progress = FMath::Clamp(Elapsed / TaskDuration, 0.f, 1.f);
        SearchWidget->UpdateProgress(Progress);
    }

    if (SearchComp->bSearched)
    {
        FinishTask(true);
    }
}

void USearchAbilityTask::BindInput()
{
    APawn* Pawn = Cast<APawn>(GetAvatarActor());
    if (!Pawn || !Pawn->InputComponent)
        return;

    Pawn->InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &USearchAbilityTask::OnCancel);
}

void USearchAbilityTask::UnbindInput()
{
}

void USearchAbilityTask::OnCancel()
{
    if (!bIsSearching) return;
    bIsSearching = false;

    OnFinished.Broadcast(false);
    EndTask();
}

void USearchAbilityTask::FinishTask(bool bSuccess)
{
    bIsSearching = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}