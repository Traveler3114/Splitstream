#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralLevelGenerator.generated.h"

USTRUCT(BlueprintType)
struct FRandomDate
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Year;

    UPROPERTY(BlueprintReadOnly)
    int32 Month;

    UPROPERTY(BlueprintReadOnly)
    int32 Day;

    FRandomDate() : Year(2020), Month(1), Day(1) {}
};

UCLASS()
class ECHOESOFTIME_API AProceduralLevelGenerator : public AActor
{
    GENERATED_BODY()
public:
    AProceduralLevelGenerator();

    UPROPERTY(Replicated, BlueprintReadOnly)
    FRandomDate RandomDate;

    UPROPERTY(Replicated, BlueprintReadOnly)
    AActor* CodeComputerRef;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FString CodeComputerStaffName;

    UPROPERTY(Replicated, BlueprintReadOnly)
    FString KeypadCode;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    FString GenerateRandomCode(int Length = 4);
    TArray<FString> GenerateShuffledStaffNames(int NumComputers);
    FRandomDate GenerateRandomDate();
};