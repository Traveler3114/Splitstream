#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "LockPickWidget.generated.h"

class ULockPickComponent;
class UImage;

UCLASS()
class ECHOESOFTIME_API ULockPickWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // Call this after creating the widget to set up the pins
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void InitializeLockPickWidget(ULockPickComponent* InLockComp);

    // Update the widget's display (call when input angle or active pin changes)
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void UpdatePins(float CurrentInputAngle);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* PinContainer;

    UPROPERTY(EditAnywhere, Category = "LockPick|Style")
    TSubclassOf<UImage> PinImageClass; // Optional: use if you want a BP subclass

    UPROPERTY(EditAnywhere, Category = "LockPick|Style")
    UTexture2D* PinImageTexture;

    UPROPERTY(EditAnywhere, Category = "LockPick|Style")
    FLinearColor ActivePinColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, Category = "LockPick|Style")
    FLinearColor InactivePinColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category = "LockPick|Style")
    FLinearColor SetPinColor = FLinearColor::Green;

    UPROPERTY()
    ULockPickComponent* LockComp = nullptr;

    TArray<UImage*> PinImages;

    float LastInputAngle = 0.f;
};