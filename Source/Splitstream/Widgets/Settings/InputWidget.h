#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ISettingsTabInterface.h"
#include "InputWidget.generated.h"

class USliderWidget;
class UVerticalBox;
class UKeybindWidget;
class UInputMappingContext;
class UInputAction;
class UDefaultGameInstance;

USTRUCT(BlueprintType)
struct FKeybindDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybinding")
    UInputAction* InputAction = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybinding")
    FText DisplayName;
};

UCLASS()
class SPLITSTREAM_API UInputWidget : public UUserWidget, public ISettingsTabInterface
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // ISettingsTabInterface
    virtual void ApplySettings_Implementation() override;

protected:
    UPROPERTY(meta = (BindWidget))
    USliderWidget* MouseSensitivityWidget;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* KeybindsList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TArray<FKeybindDefinition> KeybindsToExpose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TSubclassOf<UKeybindWidget> KeybindWidgetClass;

    TArray<UKeybindWidget*> KeybindWidgets;
    UInputAction* PendingRebindAction = nullptr;
    UKeybindWidget* PendingRebindWidget = nullptr;

    float MouseSensitivity;
    float MouseSensitivityMin;
    float MouseSensitivityMax;

    void SetupWidgets();
    void BuildKeybindList();

    UFUNCTION() void OnMouseSensitivityChanged(float Value);
    UFUNCTION() void HandleRowClicked(UKeybindWidget* Source);

    void UpdateKeybindDisplay(UInputAction* InputAction);
    void SaveUserSettingsToGameInstance();
    void LoadUserSettingsFromGameInstance();
    void UpdateTexts();
};
