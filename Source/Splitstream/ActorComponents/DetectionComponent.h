#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DetectionComponent.generated.h"

USTRUCT()
struct FDetectionState
{
    GENERATED_BODY()
    float Progress = 0.f;
    int8 Direction = 0; // +1 (building), -1 (cooling), 0 == idle
    bool bDetectionInProgress = false;
    bool bFullyDetected = false;

    FDetectionState() : Progress(0.f), Direction(0), bDetectionInProgress(false), bFullyDetected(false) {}
    FDetectionState(float P, int8 D, bool InDetection = false, bool InFullyDetected = false)
        : Progress(P), Direction(D), bDetectionInProgress(InDetection), bFullyDetected(InFullyDetected) {
    }
};

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

    // The higher this is, the faster it fills when up close.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Distance")
    float BaseDetectionSpeedMultiplier = 1600.f; // Tune as needed!

    UPROPERTY()
    TMap<AActor*, FDetectionState> DetectionStates;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void StartDetection(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void StopDetection(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void ForceImmediateDetectionEnd(AActor* Detector);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDetectionProgress(AActor* Detector) const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsDetectionInProgress(AActor* Detector) const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsFullyDetected(AActor* Detector) const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    TArray<AActor*> GetActiveDetectors() const;

    UPROPERTY(BlueprintAssignable)
    FOnDetectionBegan OnDetectionBegan;
    UPROPERTY(BlueprintAssignable)
    FOnDetectionEnded OnDetectionEnded;

protected:
    void HandleFullyDetected(AActor* Detector);
    float GetDetectionSpeedMultiplier(AActor* Detector) const;
};