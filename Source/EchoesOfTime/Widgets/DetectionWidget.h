#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DetectionWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API UDetectionWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void SetDetectionProgress(float Progress, bool bIsLocked);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void SetDetectionBarAngle(float AngleDegrees);

protected:
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* DetectionProgressBar;
};