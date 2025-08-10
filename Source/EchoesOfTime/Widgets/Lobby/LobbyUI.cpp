#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"

void ULobbyUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (ready_btn)
    {
        ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
    }
}
void ULobbyUI::SetStartButtonEnabled(bool bEnabled)
{
    if (start_btn)
    {
        start_btn->SetIsEnabled(bEnabled);
    }
}

void ULobbyUI::SetStartButtonVisibility(ESlateVisibility InVisibility)
{
    if (start_btn)
    {
        start_btn->SetVisibility(InVisibility);
    }
}

void ULobbyUI::OnReadyButtonClicked()
{
    bLocalReady = !bLocalReady;

    if (ready_btn)
    {
        UTextBlock* ReadyText = Cast<UTextBlock>(ready_btn->GetChildAt(0));
        if (ReadyText)
        {
            ReadyText->SetText(bLocalReady ? FText::FromString(TEXT("Unready")) : FText::FromString(TEXT("Ready")));
        }
    }

    if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
    {
        PC->ServerSetReadyState(bLocalReady);
    }
}