#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "DefaultPlayerState.h"
#include "GameFramework/PlayerController.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ready_btn)
	{
		ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
	}
	if (changeteam_btn)
	{
		changeteam_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnChangeTeamButtonClicked);
	}
	if (start_btn)
	{
		start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
	}
	if (leave_btn)
	{
		leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);
	}

	// Bind to local player's ready delegate
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		{
			DPS->OnReadyChanged.AddDynamic(this, &ULobbyUI::HandleLocalReadyChanged);
			RefreshReadyLabel(DPS->IsReady());
		}
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

void ULobbyUI::OnLeaveButtonClicked()
{
	// TODO: implement leave logic
}

void ULobbyUI::OnStartButtonClicked()
{
	// TODO: host start logic
}

void ULobbyUI::OnChangeTeamButtonClicked()
{
	// TODO: implement team change logic
}

void ULobbyUI::OnReadyButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		{
			DPS->ServerToggleReady();
			// Optional optimistic UI (the delegate will also refresh it when replication confirms):
			RefreshReadyLabel(!DPS->IsReady());
		}
	}
}

// NEW
void ULobbyUI::RefreshReadyLabel(bool bIsReady)
{
	if (ready_txt)
	{
		ready_txt->SetText(bIsReady ? FText::FromString(TEXT("Unready"))
			: FText::FromString(TEXT("Ready")));
	}
}

// NEW
void ULobbyUI::HandleLocalReadyChanged(ADefaultPlayerState* PS)
{
	if (PS)
	{
		RefreshReadyLabel(PS->IsReady());
	}
}