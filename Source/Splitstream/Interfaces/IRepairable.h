#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TimelineEra.h"
#include "IRepairable.generated.h"

/** Broadcast when a repairable actor requests repair (e.g., after destruction). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairRequested, AActor*, Repairable);

/**
 * Interface for actors that can be repaired by the robot guard AI.
 *
 * Implement on destructible/disableable actors (drones, cameras, spawners)
 * that should be restored to a working state by ARobotGuardCharacter.
 * Each repairable provides its repair time, timeline era, and a delegate
 * to notify the repair system when repair is needed.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class URepairable : public UInterface
{
    GENERATED_BODY()
};

class IRepairable
{
    GENERATED_BODY()

public:
    /**
     * Called by the repair system to initiate repair on this actor.
     * @param RepairInstigator  The actor performing the repair (e.g., the robot guard).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    void RequestRepair(AActor* RepairInstigator);

    /**
     * Returns the time (in seconds) required to fully repair this actor.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    float GetRepairTime() const;

    /**
     * Returns the timeline era this repairable belongs to (Past or Future).
     * Used to match repairables with guards in the same era.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    ETimelineEra GetTimelineEra() const;

    /**
     * Returns the actor that should be targeted by the guard for navigation
     * when moving to repair this object. Usually returns self.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    AActor* GetCompletionTarget() const;

    /**
     * Returns a reference to the OnRepairRequested delegate.
     * Must be implemented in the concrete child class.
     * The repair system binds to this delegate to queue repairs when broadcast.
     */
    virtual FOnRepairRequested& GetOnRepairRequested() = 0;
};