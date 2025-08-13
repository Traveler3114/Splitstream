#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/LobbyGameMode.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	if (LobbyUIClass)
	{
		LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
		if (LobbyUIInstance)
		{
			LobbyUIInstance->AddToViewport();

			if (HasAuthority())
			{
				LobbyUIInstance->SetStartButtonVisibility(ESlateVisibility::Visible);
				LobbyUIInstance->SetStartButtonEnabled(false);
			}
			else
			{
				LobbyUIInstance->SetStartButtonVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void ALobbyPlayerController::ClientSetStartButtonEnabled_Implementation(bool bEnabled)
{
	if (LobbyUIInstance)
	{
		LobbyUIInstance->SetStartButtonEnabled(bEnabled);
	}
}

void ALobbyPlayerController::ServerStartGame_Implementation()
{
	if (HasAuthority())
	{
		if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
		{
			GM->StartGameIfAllowed(this);
		}
	}
}

void ALobbyPlayerController::ClientShowLoadingScreen_Implementation()
{
	if (LoadingWidgetClass)
	{
		if (UUserWidget* LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass, TEXT("LoadingWidget")))
		{
			LoadingWidget->AddToViewport();
		}
	}
	// Optional: set input mode / show mouse, etc.
}