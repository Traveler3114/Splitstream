#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "DetectionComponent.generated.h"

class UDetectionActorWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ECHOESOFTIME_API UDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UDetectionComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDuration = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionBarHideDelay = 10.f; // seconds to keep bar after fully detected

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TSubclassOf<UDetectionActorWidget> DetectionWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    UWidgetComponent* DetectionWidgetComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Detection")
    bool bDetectionInProgress = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Detection")
    bool bFullyDetected = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Detection")
    AActor* CurrentDetector = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void StartDetection(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void StopDetection(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDetectionProgress() const;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetDetectionElapsed();

protected:
    float DetectionElapsed = 0.f;
    float FullyDetectedElapsed = 0.f;
    UDetectionActorWidget* CachedWidget = nullptr;

    void UpdateWidget();

    UFUNCTION()
    void HandleFullyDetected();
};