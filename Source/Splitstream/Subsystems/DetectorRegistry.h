#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DetectorRegistry.generated.h"

/**
 * World subsystem that tracks actors capable of detecting players
 * (guards, cameras, etc.) to avoid TActorIterator sweeps.
 *
 * Created automatically per-world on both server and clients.
 */
UCLASS()
class SPLITSTREAM_API UDetectorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void Register(AActor* Detector);
    void Unregister(AActor* Detector);

    const TSet<AActor*>& GetDetectors() const { return Detectors; }

private:
    TSet<AActor*> Detectors;
};
