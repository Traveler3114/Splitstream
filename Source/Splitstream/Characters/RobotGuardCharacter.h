#pragma once

#include "CoreMinimal.h"
#include "Characters/GuardCharacter.h"
#include "RobotGuardCharacter.generated.h"

UCLASS()
class SPLITSTREAM_API ARobotGuardCharacter : public AGuardCharacter
{
    GENERATED_BODY()

public:
    ARobotGuardCharacter();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<AActor*> RepairQueue;
    UPROPERTY(BlueprintReadWrite)
    AActor* CurrentRepairTarget = nullptr;

    virtual void BeginPlay() override;
    UFUNCTION()
    void OnRepairRequested(AActor* Repairable);
    UFUNCTION(BlueprintCallable)
    void OnRepairFinished();
    UFUNCTION(BlueprintCallable)
    void TryStartNextRepair();


protected:
    void QueueRepair(AActor* Repairable);
    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data) override{}
};