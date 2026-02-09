#include "Subsystems/DetectorRegistry.h"

void UDetectorRegistry::Register(AActor* Detector)
{
    if (Detector)
    {
        Detectors.Add(Detector);
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
