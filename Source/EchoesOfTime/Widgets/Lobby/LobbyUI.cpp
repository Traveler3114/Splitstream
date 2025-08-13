#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "GameModes/LobbyGameMode.h"
#include "DefaultPlayerState.h"

void ULobbyUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (ready_btn)
    {
        ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
    }

    if (changeteam_btn)
    {
        changeteam_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnChangeButtonClicked);
    }
    if (start_btn)
    {
        start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
    }
    if (leave_btn)
    {
        leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);
    }
}

void ULobbyUI::OnLeaveButtonClicked()
{

}


void ULobbyUI::OnStartButtonClicked()
{

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

}

void ULobbyUI::OnChangeTeamButtonClicked()
{

}