// AEnvironmentalSlot.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentalSlot.generated.h"

class AEnvironmentalObject;

/**
 * A world-placed actor that marks where an NPC stands when interacting
 * with a parent AEnvironmentalObject. Place these manually in the level
 * and assign them to the object's Slots array.
 *
 * One slot = one NPC at a time. Add more slots for sofas, benches etc.
 */
UCLASS()
class SPLITSTREAM_API AEnvironmentalSlot : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalSlot();

    // ── Occupancy ────────────────────────────────────────────────────────────

    // Returns true if no NPC is currently using this slot
    UFUNCTION(BlueprintCallable)
    bool IsAvailable() const { return OccupyingNPC == nullptr; }

    // Called by the interact task when the NPC arrives
    UFUNCTION(BlueprintCallable)
    void Occupy(AActor* NPC);

    // Called by the interact task when the NPC finishes
    UFUNCTION(BlueprintCallable)
    void Release();

    // Which NPC is currently using this slot, nullptr if free
    UFUNCTION(BlueprintPure)
    AActor* GetOccupyingNPC() const { return OccupyingNPC; }


#if WITH_EDITORONLY_DATA
    // Arrow so you can see facing direction in editor
    UPROPERTY()
    TObjectPtr<class UArrowComponent> ArrowComponent;
#endif

private:
    UPROPERTY()
    TObjectPtr<AActor> OccupyingNPC;
};
