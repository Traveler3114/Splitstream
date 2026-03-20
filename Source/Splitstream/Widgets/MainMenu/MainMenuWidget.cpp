// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Player/Controllers/MainMenuPlayerController.h"
#include "Widgets/Settings/SettingsWidget.h"
#include "DefaultGameInstance.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (host_btn)
    {
        host_btn->OnClicked.RemoveDynamic(this, &UMainMenuWidget::OnHostClicked);
        host_btn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHostClicked);
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

void UMainMenuWidget::OnHostClicked()
{
    if (!MapSelectionWidgetInstance && MapSelectionWidgetClass)
    {
        MapSelectionWidgetInstance = CreateWidget<UMapSelectionWidget>(GetWorld(), MapSelectionWidgetClass);
        if (MapSelectionWidgetInstance)
        {
            MapSelectionWidgetInstance->AddToViewport(200);

            // Bind callback for when map is selected
            MapSelectionWidgetInstance->OnMapSelected.AddDynamic(this, &UMainMenuWidget::OnMapSelected);

            // Optional: hide MainMenuWidget if you want only selection visible
            SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    else if (MapSelectionWidgetInstance)
    {
        MapSelectionWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        SetVisibility(ESlateVisibility::Collapsed);
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

void UMainMenuWidget::OnMapSelected(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset, const TSoftObjectPtr<UWorld>& LobbyLevelAsset)
{
    // Return MainMenu visible
    SetVisibility(ESlateVisibility::Visible);
    if (MapSelectionWidgetInstance)
    {
        MapSelectionWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
    }

    // SHOW LOADING WIDGET HERE!
    //if (APlayerController* PC = GetOwningPlayer())
    //{
    //    if (AMainMenuPlayerController* MainPC = Cast<AMainMenuPlayerController>(PC))
    //    {
    //        MainPC->ShowLoadingWidget();
    //    }
    //}

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
    {
        if (UDefaultGameInstance* DGI = Cast<UDefaultGameInstance>(GI))
        {
            DGI->CreateSession(LevelName, LevelAsset, LobbyLevelAsset);
        }
    }
}