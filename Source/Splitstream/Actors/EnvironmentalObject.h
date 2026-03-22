// AEnvironmentalObject.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "TimelineEra.h"
#include "Actors/PointActors/EnvironmentalSlot.h"
#include "EnvironmentalObject.generated.h"

class UAnimMontage;

/**
 * Base class for all interactable environment objects — chairs, ATMs,
 * water machines, sofas etc.
 *
 * Place slots (AEnvironmentalSlot) in the world and assign them to
 * the Slots array. The NPC interact task queries this object for a
 * free slot, walks to it, occupies it, and releases it when done.
 *
 * Subclass in Blueprint to set the mesh, tags, slots, and montage
 * per object type.
 */
UCLASS()
class SPLITSTREAM_API AEnvironmentalObject : public AActor, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    AEnvironmentalObject();

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
    {
        TagContainer.AppendTags(ObjectTags);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // ── Tags ─────────────────────────────────────────────────────────────────

    // Type tag (e.g. World.Interactable.Chair) +
    // Zone tag (e.g. World.Floor.Basement)
    // NPC tasks filter by these to find valid objects
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer ObjectTags;

    // ── Slots ────────────────────────────────────────────────────────────────

    // World-placed AEnvironmentalSlot actors belonging to this object.
    // Assign in the level — one per seat/position.
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    TArray<TObjectPtr<AEnvironmentalSlot>> Slots;

    // Returns the first available slot, or nullptr if all occupied
    UFUNCTION(BlueprintCallable)
    AEnvironmentalSlot* GetAvailableSlot() const;

    // Returns true if at least one slot is free
    UFUNCTION(BlueprintPure)
    bool HasAvailableSlot() const { return GetAvailableSlot() != nullptr; }

    // ── Animation (stubbed — not used yet) ───────────────────────────────────

    // Montage the NPC plays when interacting with this object
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> InteractionMontage;

    // ── Mesh ─────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Mesh;
};
