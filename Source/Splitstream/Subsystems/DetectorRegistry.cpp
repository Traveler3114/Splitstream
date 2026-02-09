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
