// ProximityHackWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProximityHackWidget.generated.h"

/**
 * Simple widget for showing proximity hack progress via a UProgressBar.
 * Receives progress via SetHackProgress(float) from UProximityHackComponent.
 */
UCLASS()
class ECHOESOFTIME_API UProximityHackWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Called from ProximityHackComponent with progress in [0..1]. */
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void SetHackProgress(float Progress);

protected:
    /** Progress bar in the widget (bind this in UMG). */
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* ProgressBar;
};