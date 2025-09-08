// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralLevelManager.generated.h"

// Forward declarations
class AComputer;
class AKeypadScanner;

USTRUCT(BlueprintType)
struct FClueData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
    FString ClueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
    FString AssociatedCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clue")
    FVector Location;

    FClueData()
    {
        ClueText = TEXT("");
        AssociatedCode = TEXT("");
        Location = FVector::ZeroVector;
    }
};

UCLASS()
class ECHOESOFTIME_API AProceduralLevelManager : public AActor
{
    GENERATED_BODY()

public:
    AProceduralLevelManager();

    virtual void BeginPlay() override;

    // Generate random codes and distribute them to computers
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void GenerateAndDistributeCodes();

    // Get a random clue for a given code
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    FClueData GetRandomClueForCode(const FString& Code);

    // Get all generated codes
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    TArray<FString> GetAllCodes() const { return GeneratedCodes; }

protected:
    // Number of codes to generate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 NumberOfCodesToGenerate = 3;

    // Length of each code
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    int32 CodeLength = 4;

    // Pool of clue templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    TArray<FString> ClueTemplates;

    // Generated codes for this level
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Procedural")
    TArray<FString> GeneratedCodes;

    // Map of codes to computers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Procedural")
    TMap<FString, AComputer*> CodeToComputerMap;

    // Generate a random code
    FString GenerateRandomCode();

    // Find available computers in the level
    TArray<AComputer*> FindAvailableComputers();

    // Find all keypad scanners in the level
    TArray<AKeypadScanner*> FindAllKeypads();

public:
    // Static instance for global access
    static AProceduralLevelManager* GetInstance(UWorld* World);

private:
    static AProceduralLevelManager* Instance;
};