// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuPlayerController.h"
#include "Widgets/MainMenu/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"

void AMainMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return;

    if (MainMenuUIClass)
    {
        MainMenuInstance = CreateWidget<UMainMenuWidget>(this, MainMenuUIClass);
        if (MainMenuInstance)
        {
            MainMenuInstance->AddToViewport();
        }
    }
}