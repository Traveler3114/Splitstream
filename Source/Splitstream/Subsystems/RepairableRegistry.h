#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RepairableRegistry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairableRegistered, AActor*, Repairable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairableUnregistered, AActor*, Repairable);

/**
 * World subsystem that tracks all IRepairable actors in the world.
 *
 * Provides OnRepairableRegistered/OnRepairableUnregistered delegates
 * so that repair agents (e.g., ARobotGuardCharacter) can automatically
 * discover and bind to new repairables without world-wide iteration.
 */
UCLASS()
class SPLITSTREAM_API URepairableRegistry : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    void RegisterRepairable(AActor* Actor);
    void UnregisterRepairable(AActor* Actor);
    const TArray<AActor*>& GetRepairables() const { return Repairables; }

    UPROPERTY(BlueprintAssignable)
    FOnRepairableRegistered OnRepairableRegistered;

    UPROPERTY(BlueprintAssignable)
    FOnRepairableUnregistered OnRepairableUnregistered;
private:
    UPROPERTY()
    TArray<AActor*> Repairables;
};