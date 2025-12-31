#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DetectionComponent.generated.h"

class UDetectionActorWidget;

// Event delegates to notify UI/Registry (OPTIONAL, for UI update hooks)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionBegan, AActor*, OwnerActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionEnded, AActor*, OwnerActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API UDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDetectionComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDuration = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionBarHideDelay = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TSubclassOf<UDetectionActorWidget> DetectionWidgetClass;

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
    void ForceImmediateDetectionEnd(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDetectionProgress() const;

    UFUNCTION(NetMulticast, Reliable)
    void MulticastResetDetectionElapsed();

    // Optional UI/Registry delegates
    UPROPERTY(BlueprintAssignable)
    FOnDetectionBegan OnDetectionBegan;

    UPROPERTY(BlueprintAssignable)
    FOnDetectionEnded OnDetectionEnded;

protected:
    float DetectionElapsed = 0.f;
    float FullyDetectedElapsed = 0.f;

	UFUNCTION(NetMulticast, Reliable)
    void MulticastUpdateRegistry(bool bRegister);

    void HandleFullyDetected();
};