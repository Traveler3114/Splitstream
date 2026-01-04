#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TimelineEra.h"
#include "IRepairable.generated.h"

// Universal delegate for all repairables.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairRequested, AActor*, Repairable);

UINTERFACE(MinimalAPI, Blueprintable)
class URepairable : public UInterface
{
    GENERATED_BODY()
};

class IRepairable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    void RequestRepair(AActor* RepairInstigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    float GetRepairTime() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    ETimelineEra GetTimelineEra() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    AActor* GetCompletionTarget() const;

    // Delegate accessor – must be implemented in the concrete child
    virtual FOnRepairRequested& GetOnRepairRequested() = 0;
};