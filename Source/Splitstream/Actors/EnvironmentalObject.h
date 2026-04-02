// AEnvironmentalObject.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "Actors/PointActors/EnvironmentalSlot.h"
#include "EnvironmentalObject.generated.h"

class UAnimMontage;
class USearchComponent;
class UItemBase;
class ACivilianCharacter;

/**
 * Unified base class for all interactable environment objects.
 *
 * Covers both:
 *   - NPC interaction (walk to slot, occupy, idle, release)
 *   - Player search (SearchComponent, optional item reward)
 *
 * Per object in the level:
 *   - Add slots for NPC interaction (water machine, chair, etc.)
 *   - Enable bSearchable + bGivesItem + set RewardItem for searchable objects
 *   - Leave slots empty for pure searchable objects (no NPC interaction)
 *   - Leave bSearchable false for pure NPC interactables
 */
UCLASS()
class SPLITSTREAM_API AEnvironmentalObject : public AActor, public IGameplayTagAssetInterface, public IInteractable
{
    GENERATED_BODY()

public:
    AEnvironmentalObject();

    virtual void BeginPlay() override;

    // ── IGameplayTagAssetInterface ────────────────────────────────────────────

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override
    {
        TagContainer.AppendTags(ObjectTags);
    }

    // ── IInteractable ─────────────────────────────────────────────────────────

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override { return true; }

    // ── General ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    // ── Tags ─────────────────────────────────────────────────────────────────

    // Type tag (e.g. World.Interactable.Chair) +
    // Zone tag (e.g. World.Floor.Basement)
    // NPC tasks filter by these to find valid objects
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer ObjectTags;

    // ── NPC Interaction ───────────────────────────────────────────────────────

    // World-placed AEnvironmentalSlot actors belonging to this object.
    // Assign in the level — one per seat/position.
    // Leave empty if this object is not used for NPC interaction.
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    TArray<TObjectPtr<AEnvironmentalSlot>> Slots;

    // Returns the first available slot, or nullptr if all occupied
    UFUNCTION(BlueprintCallable)
    AEnvironmentalSlot* GetAvailableSlot() const;

    // Returns true if at least one slot is free
    UFUNCTION(BlueprintPure)
    bool HasAvailableSlot() const { return GetAvailableSlot() != nullptr; }

    // Montage the NPC plays when interacting with this object (stubbed — not used yet)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> InteractionMontage;

    // ── Player Search ─────────────────────────────────────────────────────────
    // If true, gives RewardItem to the player on search complete
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable",
        meta = (EditCondition = "bSearchable"))
    bool bGivesItem = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Searchable",
        meta = (EditCondition = "bSearchable && bGivesItem"))
    TObjectPtr<UItemBase> RewardItem = nullptr;

    // Set by NPC Interact_Implementation when a civilian unlocks this for player searching
    UPROPERTY()
    TObjectPtr<ACivilianCharacter> PendingOwnerCivilian = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Searchable")
    TObjectPtr<USearchComponent> SearchComponent = nullptr;

    // ── Mesh ─────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Mesh;

protected:
    UFUNCTION()
    virtual void OnSearchComplete();
};
