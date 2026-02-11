#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DetectionWidget.generated.h"

UCLASS()
class SPLITSTREAM_API UDetectionWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void SetDetectionProgress(float Progress, bool bIsLocked);
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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

    float DisplayedProgress = 0.f;
    float TargetProgress = 0.f;
    bool bIsLockedCached = false; // To store last lock state

};