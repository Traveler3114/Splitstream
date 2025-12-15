#pragma once

#include "CoreMinimal.h"
#include "Actors/PowerGenerator.h"
#include "PastPowerGenerator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneratorCompleted, bool, bIsCompleted);

UCLASS()
class ECHOESOFTIME_API APastPowerGenerator : public APowerGenerator
{
    GENERATED_BODY()

public:
    APastPowerGenerator();

    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Fires when this generator becomes completed/searched
    UPROPERTY(BlueprintAssignable, Category = "PastPowerGenerator")
    FOnGeneratorCompleted OnGeneratorCompleted;

    // True if SearchComponent->bSearched == true
    UFUNCTION(BlueprintPure, Category = "PastPowerGenerator")
    bool IsCompleted() const;

protected:
    UFUNCTION()
    void OnSearchCompletedEvent();
};