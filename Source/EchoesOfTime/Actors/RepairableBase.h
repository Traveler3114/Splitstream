#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "RepairableBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestRepair, ARepairableBase*, RepairableActor);

UCLASS(Abstract)
class ECHOESOFTIME_API ARepairableBase : public AActor
{
    GENERATED_BODY()
public:
    ARepairableBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
    /** Called to request this object be repaired. Call this to begin the repair process. */
    UFUNCTION(BlueprintCallable, Category = "Repair")
    virtual void RequestRepair(AActor* RepairInstigator);

    /** Returns how long it takes to repair this object. */
    UFUNCTION(BlueprintCallable, Category = "Repair")
    virtual float GetRepairTime() const;

    /** Broadcast when this repairable actor needs repair, such as after being 'searched' or 'destroyed'. */
    UPROPERTY(BlueprintAssignable, Category = "Repair")
    FOnRequestRepair OnRequestRepair;

    /** How long it takes to repair this actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
    float RepairTime = 5.0f;
};