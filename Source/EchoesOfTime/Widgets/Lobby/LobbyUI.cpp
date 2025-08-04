// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyUI.h"
#include "Components/Button.h"

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();
	//if (start_btn)
	//{
	//	start_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnStartButtonClicked);
	//}
	//if (leave_btn)
	//{
	//	leave_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnLeaveButtonClicked);
	//}
	//if (ready_btn)
	//{
	//	ready_btn->OnClicked.AddDynamic(this, &ULobbyUI::OnReadyButtonClicked);
	//}
}

void ULobbyUI::SetStartButtonVisibility(ESlateVisibility InVisibility)
{
	if (start_btn)
	{
		start_btn->SetVisibility(InVisibility);
	}
}
