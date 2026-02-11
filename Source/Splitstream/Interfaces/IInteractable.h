#pragma once

#include "UObject/Interface.h"
#include "ActorComponents/InventoryComponent.h"
#include "IInteractable.generated.h"


/**
 * Interface for actors that can be interacted with by players.
 *
 * Implement this on any actor that should respond to player interaction input
 * (e.g., doors, buttons, items, NPCs, puzzle elements).
 *
 * Supports both instant and progressive (hold) interactions, item-gated
 * interactions, and visual highlight feedback.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IInteractable
{
    GENERATED_BODY()

public:
    /**
     * Called when the player performs an interaction on this actor.
     * @param Interactor  The actor initiating the interaction (usually the player pawn).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);

    /**
     * Called when a progressive (hold) interaction is canceled before completion.
     * @param Interactor  The actor that was interacting.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void CancelInteract(AActor* Interactor);

    /**
     * Enables or disables the visual highlight on this actor (e.g., custom depth outline).
     * Called by the interaction system when the player's line trace enters or leaves this actor.
     * @param bHighlight  True to enable highlight, false to disable.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void SetHighlighted(bool bHighlight);

    /**
     * Returns true if this interaction requires the player to hold the interact input
     * (progressive interaction). False for instant/tap interactions.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsProgressiveInteract();

    /**
     * Checks whether the given inventory slot contains the correct item for this interaction.
     * Only relevant when RequiresItem() returns true.
     * @param Slot  The player's currently active inventory slot.
     * @return True if the slot contains a valid item for this interactable.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsCorrectItem(const FInventorySlot& Slot) const;

    /**
     * Returns true if this interactable requires the player to have a specific item equipped
     * before interaction can proceed.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool RequiresItem() const;
};