// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapSelectionWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS()
class SPLITSTREAM_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget)) UButton* host_btn;
    UPROPERTY(meta = (BindWidget)) UButton* settings_btn;
    UPROPERTY(meta = (BindWidget)) UButton* quit_btn;
    UPROPERTY(meta = (BindWidgetOptional))
    class USettingsWidget* SettingsWidget;

    // New: reference to MapSelectionWidget (created dynamically)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UMapSelectionWidget> MapSelectionWidgetClass;

    UPROPERTY()
    UMapSelectionWidget* MapSelectionWidgetInstance;

    UFUNCTION()
    void OnHostClicked();

    UFUNCTION()
    void OnSettingsClicked();

    UFUNCTION()
    void OnQuitClicked();

    // Called when map is chosen from selection widget
    UFUNCTION()
    void OnMapSelected(const FString& LevelName, const TSoftObjectPtr<UWorld>& LevelAsset);
};