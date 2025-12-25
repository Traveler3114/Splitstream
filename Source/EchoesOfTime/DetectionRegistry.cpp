#include "DetectionRegistry.h"

void UDetectionRegistry::RegisterDetectedActor(AActor* Actor)
{
    if (Actor)
    {
        DetectedActors.Add(Actor);
    }
}

void UDetectionRegistry::UnregisterDetectedActor(AActor* Actor)
{
    DetectedActors.Remove(Actor);
}

const TSet<TWeakObjectPtr<AActor>>& UDetectionRegistry::GetDetectedActors() const
{
    return DetectedActors;
}