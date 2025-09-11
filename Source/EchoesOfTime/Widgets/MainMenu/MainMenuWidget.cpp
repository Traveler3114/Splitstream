// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SettingsWidget.h"
#include "DefaultGameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (createsession_btn)
	{
		createsession_btn->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnCreateSessionClicked);
		createsession_btn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateSessionClicked);
	}
	if (settings_btn) {
        settings_btn->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnSettingsClicked);
        settings_btn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
    }
	if (quit_btn)
	{
		quit_btn->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnQuitClicked);
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

void UMainMenuWidget::OnSettingsClicked()
{
    if (SettingsWidget)
        SettingsWidget->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::OnQuitClicked()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
}
