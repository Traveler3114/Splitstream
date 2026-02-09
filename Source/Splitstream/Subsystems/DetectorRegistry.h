#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DetectorRegistry.generated.h"

/**
 * World subsystem that tracks actors capable of detecting players
 * (guards, cameras, etc.) to avoid TActorIterator sweeps.
 *
 * Created automatically per-world on both server and clients.
 * Each world (server/clients) has its own registry instance.
 * 
 * Uses TWeakObjectPtr to safely handle actor destruction without explicit unregistration.
 */
UCLASS()
class SPLITSTREAM_API UDetectorRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void Register(AActor* Detector);
    void Unregister(AActor* Detector);

    /** Returns array of valid (non-destroyed) detectors. Safe for iteration. */
    TArray<AActor*> GetValidDetectors() const;

    /** Optional: Remove stale weak pointers to reduce memory overhead. Called automatically during Register/Unregister. */
    void CleanupStaleReferences();

private:
    TSet<TWeakObjectPtr<AActor>> Detectors;
};
