#pragma once

#include "CoreMinimal.h"
#include "Actors/PowerGenerator.h"
#include "FuturePowerGenerator.generated.h"

UCLASS()
class ECHOESOFTIME_API AFuturePowerGenerator : public APowerGenerator
{
    GENERATED_BODY()

public:
    AFuturePowerGenerator();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastPowerGenerator")
    TSoftObjectPtr<class APastPowerGenerator> PastGenerator;

    // Optionally, allow getting this generator's completion state
    UFUNCTION(BlueprintPure, Category = "FuturePowerGenerator")
    bool IsCompleted() const;

protected:
    UFUNCTION()
    void HandlePastGeneratorCompleted(bool bCompleted);
};