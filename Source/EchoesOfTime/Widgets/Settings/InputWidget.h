#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputWidget.generated.h"

class UTextBlock;
class USlider;
class UButton;
class UVerticalBox;
class UInputMappingContext;
class UInputAction;
class UKeybindWidget;

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
class ECHOESOFTIME_API UInputWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    void ApplySettings();

protected:
    UPROPERTY(meta = (BindWidget))
    USlider* MouseSensitivitySlider;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MouseSensitivityValueText;
    float MouseSensitivity;
    float MouseSensitivityMin;
    float MouseSensitivityMax;

    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);
    void UpdateTexts();

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* KeybindsList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* InputMappingContext;
    UInputMappingContext* InputMappingContextRuntime = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TArray<FKeybindDefinition> KeybindsToExpose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TSubclassOf<UKeybindWidget> KeybindWidgetClass;

    TArray<UKeybindWidget*> KeybindWidgets;
    UInputAction* PendingRebindAction;
    UKeybindWidget* PendingRebindWidget = nullptr; // TRACKS ACTIVE ROW

    void BuildKeybindList();
    void UpdateKeybindDisplay(UInputAction* InputAction);

    UFUNCTION()
    void HandleRowClicked(UKeybindWidget* Source);

    void SaveUserSettings();
    void LoadUserSettings();
};