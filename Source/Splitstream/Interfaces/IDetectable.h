#pragma once

#include "UObject/Interface.h"
#include "IDetectable.generated.h"

/**
 * Interface for actors that can be detected by security systems (cameras, drones, guards).
 *
 * Implement this on any actor that should respond to the detection pipeline:
 * detection start, loss of sight, full detection, and forced detection end.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDetectable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IDetectable
{
    GENERATED_BODY()
public:
    /**
     * Called when a detector begins detecting this actor (line of sight established).
     * @param Detector  The actor performing the detection (e.g., security camera, drone, guard).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnDetected(AActor* Detector);

    /**
     * Called when a detector loses sight of this actor (line of sight broken).
     * @param Detector  The actor that lost detection.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnLost(AActor* Detector);

    /**
     * Called when detection progress reaches 100% for a given detector.
     * Typically triggers alarm or pre-alarm logic.
     * @param DetectingActor  The actor that achieved full detection.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnFullyDetected(AActor* DetectingActor);

    /**
     * Forces detection to end immediately, bypassing the normal cooldown decay.
     * Used when a detector is destroyed, disabled, or otherwise removed.
     * @param Detector  The detector being force-removed.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    void OnForceDetectionEnd(AActor* Detector);

    /**
     * Returns true if this actor is currently being detected by the given detector.
     * @param DetectingActor  The detector to check.
     * @return True if detection is currently active for this detector.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detection")
    bool IsActorAlreadyDetected(AActor* DetectingActor) const;

};