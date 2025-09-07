#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ECHOESOFTIME_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    // Bindings to UMG widgets (must match variable names in designer)
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResolutionLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ResolutionRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ResolutionValueText;

    UPROPERTY(meta = (BindWidget))
    UButton* ShadowsLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* ShadowsRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ShadowsValueText;

    UPROPERTY(meta = (BindWidget))
    UButton* TexturesLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* TexturesRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TexturesValueText;

    UPROPERTY(meta = (BindWidget))
    UButton* AALeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* AARightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AAValueText;

    UPROPERTY(meta = (BindWidget))
    UButton* PPLeftButton;

    UPROPERTY(meta = (BindWidget))
    UButton* PPRightButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PPValueText;

    // Option data
    int32 ResolutionIndex;
    TArray<FIntPoint> ResolutionOptions;
    TArray<FString> ResolutionLabels;

    int32 ShadowsIndex;
    TArray<FString> ShadowsOptions;

    int32 TexturesIndex;
    TArray<FString> TexturesOptions;

    int32 AAIndex;
    TArray<FString> AAOptions;

    int32 PPIndex;
    TArray<FString> PPOptions;

    // Handlers
    UFUNCTION()
    void OnApplyClicked();

    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnResolutionLeft();

    UFUNCTION()
    void OnResolutionRight();

    UFUNCTION()
    void OnShadowsLeft();

    UFUNCTION()
    void OnShadowsRight();

    UFUNCTION()
    void OnTexturesLeft();

    UFUNCTION()
    void OnTexturesRight();

    UFUNCTION()
    void OnAALeft();

    UFUNCTION()
    void OnAARight();

    UFUNCTION()
    void OnPPLeft();

    UFUNCTION()
    void OnPPRight();

    void UpdateTexts();
    void ApplySettings();
};