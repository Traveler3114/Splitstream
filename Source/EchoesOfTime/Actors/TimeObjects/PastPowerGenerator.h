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

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Fires when this generator becomes completed/searched
    UPROPERTY(BlueprintAssignable, Category = "PastPowerGenerator")
    FOnGeneratorCompleted OnGeneratorCompleted;

    UPROPERTY(Replicated)
    int32 ToggleCount = 0;

protected:
    void OnSearchComplete() override;
};