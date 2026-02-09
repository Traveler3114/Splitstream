#include "Subsystems/DetectorRegistry.h"

void UDetectorRegistry::Register(AActor* Detector)
{
    if (Detector)
    {
        Detectors.Add(Detector);
        // Opportunistically clean up stale references during registration
        if (Detectors.Num() > 50) // Only cleanup if set grows large
        {
            CleanupStaleReferences();
        }
    }
}

void UDetectorRegistry::Unregister(AActor* Detector)
{
    if (Detector)
    {
        Detectors.Remove(Detector);
    }
}

TArray<AActor*> UDetectorRegistry::GetValidDetectors() const
{
    TArray<AActor*> ValidDetectors;
    ValidDetectors.Reserve(Detectors.Num());
    
    for (const TWeakObjectPtr<AActor>& WeakDetector : Detectors)
    {
        if (AActor* Detector = WeakDetector.Get())
        {
            ValidDetectors.Add(Detector);
        }
    }
    
    return ValidDetectors;
}

void UDetectorRegistry::CleanupStaleReferences()
{
    for (auto It = Detectors.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
        {
            It.RemoveCurrent();
        }
    }
}
