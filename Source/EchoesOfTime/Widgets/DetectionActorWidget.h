#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DetectionActorWidget.generated.h"

UCLASS()
class ECHOESOFTIME_API UDetectionActorWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void SetDetectionProgress(float Progress, bool bIsLocked);

protected:
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* DetectionProgressBar;
};