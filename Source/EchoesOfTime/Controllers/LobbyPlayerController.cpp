// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "Widgets/Lobby/LobbyUI.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (LobbyUIClass)
	{
		ULobbyUI* LobbyUI = CreateWidget<ULobbyUI>(this, LobbyUIClass);
		if (LobbyUI)
		{
			LobbyUI->AddToViewport();
			if(HasAuthority())
			{
				LobbyUI->SetStartButtonVisibility(ESlateVisibility::Visible);
			}
		}
	}
}
