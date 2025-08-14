// (Only the OnLeaveButtonClicked implementation is added/changed)
#include "LobbyUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "DefaultPlayerState.h"
#include "GameFramework/PlayerController.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ready_btn)      ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
	if (changeteam_btn) changeteam_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnChangeTeamButtonClicked);
	if (start_btn)      start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
	if (leave_btn)      leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);

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
	if (start_btn) start_btn->SetIsEnabled(bEnabled);
}

void ULobbyUI::SetStartButtonVisibility(ESlateVisibility InVisibility)
{
	if (start_btn) start_btn->SetVisibility(InVisibility);
}

void ULobbyUI::OnLeaveButtonClicked()
{
	if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		// Always go through server; it will handle host vs. client
		LPC->ServerLeaveLobby();
	}
}

void ULobbyUI::OnStartButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->HasAuthority())
		{
			if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
			{
				LPC->ServerStartGame();
			}
		}
	}
}

void ULobbyUI::OnChangeTeamButtonClicked() {}

void ULobbyUI::OnReadyButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		{
			const bool NewState = !DPS->IsReady();

			// Standalone/Authority: apply locally (no RPC in Standalone)
			if (PC->GetNetMode() == NM_Standalone || PC->HasAuthority())
			{
				DPS->SetReadyLocal(NewState);
			}
			else
			{
				// Networked client: go through server
				DPS->ServerSetReady(NewState);
			}

			// Update local button label immediately for UX
			RefreshReadyLabel(NewState);
		}
	}
}

void ULobbyUI::RefreshReadyLabel(bool bIsReady)
{
	if (ready_txt)
	{
		ready_txt->SetText(bIsReady ? FText::FromString(TEXT("Unready"))
			: FText::FromString(TEXT("Ready")));
	}
}

void ULobbyUI::HandleLocalReadyChanged(ADefaultPlayerState* PS)
{
	if (PS) RefreshReadyLabel(PS->IsReady());
}