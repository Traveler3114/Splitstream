#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DetectionComponent.generated.h"

class ADefaultCharacter;
class ADefaultPlayerController;

/** Tracks per-detector detection progress and state (building, cooling, fully detected). */
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

USTRUCT()
struct FLastSentState
{
    GENERATED_BODY()
    float Progress = 0.f;
    bool bIsLocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionBegan, AActor*, OwnerActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionEnded, AActor*, OwnerActor);

/**
 * Component that manages progressive detection state for its owning actor.
 *
 * Tracks detection progress per-detector with distance-weighted fill rates.
 * Sends detection widget updates to player controllers via Client RPCs.
 * Fires OnDetectionBegan/OnDetectionEnded delegates for gameplay responses.
 * Tick interval is configurable and disabled when no detection is active.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPLITSTREAM_API UDetectionComponent : public UActorComponent
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
    float BaseDetectionSpeedMultiplier = 1200.f; // Tune as needed!

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

    // ------- Bandwidth optimization: throttle detection widget RPCs -------

    TMap<AActor*, FLastSentState> LastSentStates;

    TWeakObjectPtr<ADefaultCharacter> CachedOwner;
    TWeakObjectPtr<ADefaultPlayerController> CachedController;
};