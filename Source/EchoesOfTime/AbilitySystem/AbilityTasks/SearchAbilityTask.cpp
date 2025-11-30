#include "SearchAbilityTask.h"
#include "ActorComponents/SearchComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/HUD/SearchWidget.h"
#include "Controllers/DefaultPlayerController.h"

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

    if (GetAvatarActor()->HasAuthority())
    {
        SearchComp->StartSearching();
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

    if (SearchWidget)
    {
        SearchWidget->UpdateProgress(SearchComp->GetSearchProgress());
    }

    if (SearchComp->bSearched)
    {
        FinishTask(true);
    }
    else if (!SearchComp->bSearchingInProgress)
    {
        FinishTask(false);
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
    // Unreal InputComponent handles cleanup; explicit unbinding is not needed.
}

void USearchAbilityTask::OnCancel()
{
    if (!bIsSearching) return;
    bIsSearching = false;

    if (GetAvatarActor()->HasAuthority())
    {
        if (SearchComp) SearchComp->CancelSearching();
    }
    OnFinished.Broadcast(false);
    EndTask();
}

void USearchAbilityTask::FinishTask(bool bSuccess)
{
    bIsSearching = false;
    OnFinished.Broadcast(bSuccess);
    EndTask();
}