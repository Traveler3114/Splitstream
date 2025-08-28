// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "DefaultGameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (createsession_btn)
	{
		createsession_btn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateSessionClicked);
	}
	if (quit_btn)
	{
		quit_btn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}
}

void UMainMenuWidget::OnCreateSessionClicked()
{
    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (UDefaultGameInstance* DGI = Cast<UDefaultGameInstance>(GI))
        {
			DGI->CreateSession();
        }
    }
}

void UMainMenuWidget::OnQuitClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
}
