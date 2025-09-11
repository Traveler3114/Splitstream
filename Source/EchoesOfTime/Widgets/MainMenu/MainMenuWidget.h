// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
UCLASS()
class ECHOESOFTIME_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) UButton* createsession_btn;
	UPROPERTY(meta = (BindWidget)) UButton* settings_btn;
	UPROPERTY(meta = (BindWidget)) UButton* quit_btn;

	UPROPERTY(meta = (BindWidgetOptional))
    class USettingsWidget* SettingsWidget;

	UFUNCTION()
	void OnCreateSessionClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();
};
