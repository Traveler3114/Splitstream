#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputAction.h"
#include "KeybindWidget.generated.h"

class UTextBlock;
class UButton;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeKeyClicked, UKeybindWidget*, Source);

UCLASS()
class ECHOESOFTIME_API UKeybindWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // Set these up for BindWidget in your UMG designer, or instantiate if pure C++
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DisplayNameText;

    UPROPERTY(meta = (BindWidget))
    USizeBox* ButtonContainer;

    UPROPERTY(meta = (BindWidget))
    UButton* KeyChangeButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* KeyInsideButton;

    UPROPERTY()
    UInputAction* InputAction;

    void Setup(const FText& DisplayName, UInputAction* InAction, const FText& KeyText);

    FOnChangeKeyClicked OnChangeKeyClicked;

    virtual void NativeConstruct() override;

    UFUNCTION()
    void HandleChangeKeyClicked();
};