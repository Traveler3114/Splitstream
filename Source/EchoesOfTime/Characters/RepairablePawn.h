#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TimelineEra.h"
#include "RepairablePawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnRequestRepair, ARepairablePawn*, PawnToRepair);
UCLASS(Abstract)
class ECHOESOFTIME_API ARepairablePawn : public APawn
{
    GENERATED_BODY()
public:
    ARepairablePawn();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
    /** Called to request this object be repaired. Call this to begin the repair process. */
    UFUNCTION(BlueprintCallable, Category = "Repair")
    virtual void RequestPawnRepair(AActor* RepairInstigator);

    /** Returns how long it takes to repair this object. */
    UFUNCTION(BlueprintCallable, Category = "Repair")
    virtual float GetRepairTime() const;

    /** Broadcast when this repairable actor needs repair, such as after being 'searched' or 'destroyed'. */
    UPROPERTY(BlueprintAssignable, Category = "Repair")
    FOnPawnRequestRepair OnPawnRequestRepair;

    /** How long it takes to repair this actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
    float RepairTime = 5.0f;
};