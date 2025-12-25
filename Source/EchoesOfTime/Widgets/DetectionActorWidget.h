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

    /** The brush for the normal progress bar fill image */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    FSlateBrush NormalBrush;

    /** The brush for the normal background image */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    FSlateBrush NormalBackgroundBrush;

    /** The brush for the full progress bar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (AllowPrivateAccess = "true"))
    FSlateBrush FullBrush;

};