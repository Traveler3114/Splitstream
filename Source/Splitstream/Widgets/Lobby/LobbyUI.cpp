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
			DPS->OnTeamChanged.AddDynamic(this, &ULobbyUI::HandleLocalTeamChanged);
			RefreshReadyLabel(DPS->IsReady());
			RefreshTeamLabel(DPS->GetTeamName());
		}
	}
}

void ULobbyUI::SetMapName(const FText& Name)
{
	if (MapName_txt)
	{
		MapName_txt->SetText(Name);
	}
}

void ULobbyUI::OnChangeTeamButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		{
			FString NewTeam = DPS->GetTeamName() == "Past" ? "Future" : "Past";

			if (PC->GetNetMode() == NM_Standalone || PC->HasAuthority())
			{
				DPS->SetTeamLocal(NewTeam);
			}
			else
			{
				DPS->ServerSetTeam(NewTeam);
			}

			RefreshTeamLabel(NewTeam);
		}
	}
}

void ULobbyUI::RefreshTeamLabel(const FString& CurrentTeam)
{
	if (team_txt)
	{
		FString OppositeTeam = CurrentTeam == "Past" ? "Future" : "Past";
		team_txt->SetText(FText::FromString(OppositeTeam));
	}
}

void ULobbyUI::HandleLocalTeamChanged(ADefaultPlayerState* PS)
{
	if (PS) RefreshTeamLabel(PS->GetTeamName());
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
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ALobbyPlayerController* MyPC = Cast<ALobbyPlayerController>(PC))
        {
            MyPC->RequestLeaveToMainMenu();
        }
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

void ULobbyUI::OnReadyButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ADefaultPlayerState* DPS = PC->GetPlayerState<ADefaultPlayerState>())
		{
			const bool NewState = !DPS->IsReady();

			if (PC->GetNetMode() == NM_Standalone || PC->HasAuthority())
			{
				DPS->SetReadyLocal(NewState);
			}
			else
			{
				DPS->ServerSetReady(NewState);
			}

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