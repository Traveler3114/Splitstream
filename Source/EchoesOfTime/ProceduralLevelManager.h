// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "ProceduralLevelManager.generated.h"

class AArchiveComputer;
class AKeypadScanner;

USTRUCT(BlueprintType)
struct FPuzzleConfiguration
{
    GENERATED_BODY()

    // Number of archive computers to include in the puzzle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    int32 NumArchiveComputers = 3;

    // Number of keypad scanners in the puzzle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    int32 NumKeypadScanners = 1;

    // Difficulty level (affects code complexity, number of computers, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    int32 DifficultyLevel = 1;

    // Whether to use unique codes for each scanner
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bUseUniqueCodesPerScanner = true;

    FPuzzleConfiguration()
    {
        NumArchiveComputers = 3;
        NumKeypadScanners = 1;
        DifficultyLevel = 1;
        bUseUniqueCodesPerScanner = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class ECHOESOFTIME_API UProceduralLevelManager : public UObject
{
    GENERATED_BODY()

public:
    UProceduralLevelManager();

    // Initialize the procedural level manager
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void Initialize(UWorld* InWorld);

    // Generate a complete archive puzzle setup
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    bool GenerateArchivePuzzle(const FPuzzleConfiguration& Config);

    // Setup individual components
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    bool SetupArchiveComputers(const FPuzzleConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Procedural")
    bool SetupKeypadScanners(const FPuzzleConfiguration& Config);

    // Link keypad scanners to archive computers with codes
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    bool LinkScannersToComputers();

    // Get all managed archive computers
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    TArray<AArchiveComputer*> GetManagedArchiveComputers() const;

    // Get all managed keypad scanners
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    TArray<AKeypadScanner*> GetManagedKeypadScanners() const;

    // Clear the current puzzle setup
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void ClearPuzzleSetup();

    // Validate puzzle setup
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    bool ValidatePuzzleSetup() const;

protected:
    // World reference
    UPROPERTY()
    UWorld* World;

    // Current puzzle configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    FPuzzleConfiguration CurrentConfig;

    // Managed actors
    UPROPERTY()
    TArray<AArchiveComputer*> ManagedArchiveComputers;

    UPROPERTY()
    TArray<AKeypadScanner*> ManagedKeypadScanners;

    // Helper functions
    FString GenerateRandomCode(int32 Length = 4) const;
    FString GenerateUniqueCode(const TArray<FString>& ExistingCodes, int32 Length = 4) const;
    bool IsValidCode(const FString& Code) const;

private:
    // Keep track of used codes to ensure uniqueness
    TArray<FString> UsedCodes;
};