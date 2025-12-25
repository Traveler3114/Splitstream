#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DetectionRegistry.generated.h"

UCLASS()
class ECHOESOFTIME_API UDetectionRegistry : public UWorldSubsystem
{
    GENERATED_BODY()
public:

    // Add actor to detected list
    void RegisterDetectedActor(AActor* Actor);

    // Remove actor from detected list
    void UnregisterDetectedActor(AActor* Actor);

    // Get list of currently detected actors
    const TSet<TWeakObjectPtr<AActor>>& GetDetectedActors() const;

    // Optional: Remove nulls every few seconds; you can tick for this if needed
protected:
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> DetectedActors;
};