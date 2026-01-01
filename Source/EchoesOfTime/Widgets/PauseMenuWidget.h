#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseMenuResumed);

class UButton;
class USettingsWidget;

UCLASS()
class ECHOESOFTIME_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Broadcasts when Resume is clicked */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPauseMenuResumed OnPauseMenuResumed;

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* resume_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* settings_btn;

    UPROPERTY(meta = (BindWidget))
	UButton* mainmenu_btn;

    UPROPERTY(meta = (BindWidget))
    UButton* quit_btn;

    UPROPERTY(meta = (BindWidgetOptional))
    USettingsWidget* SettingsWidget;

    UFUNCTION()
    void OnResumeClicked();

    UFUNCTION()
    void OnSettingsClicked();

    UFUNCTION()
    void OnMainMenuClicked();

    UFUNCTION()
    void OnQuitClicked();
};