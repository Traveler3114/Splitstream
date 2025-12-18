#pragma once

#include "CoreMinimal.h"
#include "Characters/GuardCharacter.h"
#include "RobotGuardCharacter.generated.h"

class ARepairableBase;

UCLASS()
class ECHOESOFTIME_API ARobotGuardCharacter : public AGuardCharacter
{
    GENERATED_BODY()

public:
    ARobotGuardCharacter();

    // Holds all pending repairs
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<ARepairableBase*> RepairQueue;

    // Holds the current actor being repaired (nullptr if idle)
    UPROPERTY(BlueprintReadWrite)
    ARepairableBase* CurrentRepairTarget = nullptr;

    virtual void BeginPlay() override;

    // Called to request robot repair a destroyed object (already bound by Repairable)
    UFUNCTION()
    void OnRepairRequested(ARepairableBase* RepairableActor);

    // Called at *end* of repair task from StateTree (via Event or direct call)
    UFUNCTION(BlueprintCallable)
    void OnRepairFinished();

    // Starts the next repair if possible (can also be BlueprintCallable)
    UFUNCTION(BlueprintCallable)
    void TryStartNextRepair();

protected:
    // Helper to queue and start
    void QueueRepair(ARepairableBase* RepairableActor);
};