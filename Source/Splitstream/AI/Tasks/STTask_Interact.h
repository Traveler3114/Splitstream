// STTask_Interact.h
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "STTask_Interact.generated.h"

USTRUCT(BlueprintType)
struct SPLITSTREAM_API FSTTask_InteractInstanceData
{
    GENERATED_BODY()

    // Slot the NPC is currently occupying, nullptr until arrived
    UPROPERTY()
    TObjectPtr<class AEnvironmentalSlot> OccupiedSlot = nullptr;

    // Remaining idle time once NPC has arrived at the slot
    UPROPERTY()
    float RemainingTime = 0.f;

    // Whether the NPC has arrived at the slot and is now idling
    UPROPERTY()
    bool bArrived = false;
};

/**
 * NPC finds the nearest available AEnvironmentalObject matching
 * ObjectTypeTags + ZoneTags, walks to a free slot, idles for a
 * random duration, then releases the slot and succeeds.
 *
 * Set ObjectTags to e.g. World.Interactable.WaterMachine + World.Floor.Basement
 *
 * If no valid object or free slot is found the task fails immediately
 * so the StateTree can fall back to another behavior.
 */
USTRUCT(BlueprintType, meta = (Category = "NPC|Behavior", DisplayName = "Interact Task"))
struct SPLITSTREAM_API FSTTask_Interact : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTTask_InteractInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    // Filter objects by tags — e.g. World.Interactable.WaterMachine + World.Floor.Basement
    // All tags must match. Leave empty to allow any object.
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer ObjectTags;

    // How long the NPC idles at the object once arrived
    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
    float MinDuration = 3.f;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
    float MaxDuration = 8.f;

    // Acceptance radius for arriving at the slot
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1.0"))
    float AcceptanceRadius = 50.f;

    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    virtual EStateTreeRunStatus Tick(
        FStateTreeExecutionContext& Context,
        const float DeltaTime) const override;

    virtual void ExitState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

private:
    // Finds the nearest available object matching both tag filters
    class AEnvironmentalSlot* FindBestSlot(class AAICharacter* NPC) const;
};