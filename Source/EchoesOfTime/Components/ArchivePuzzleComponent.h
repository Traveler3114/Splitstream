// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralLevelManager.h"
#include "ArchivePuzzleComponent.generated.h"

class AArchiveComputer;
class AKeypadScanner;

/**
 * Component that can be added to GameMode or other actors to manage archive puzzles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ECHOESOFTIME_API UArchivePuzzleComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UArchivePuzzleComponent();

protected:
    virtual void BeginPlay() override;

    // Puzzle configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive Puzzle")
    FPuzzleConfiguration PuzzleConfig;

    // Auto-setup settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive Puzzle")
    bool bAutoSetupOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archive Puzzle")
    float SetupDelay = 3.0f;

    // Procedural level manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Archive Puzzle")
    UProceduralLevelManager* LevelManager;

public:    
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    bool SetupArchivePuzzle();

    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    bool SetupArchivePuzzleWithConfig(const FPuzzleConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    void ClearArchivePuzzle();

    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    bool ValidateArchivePuzzle();

    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    TArray<AArchiveComputer*> GetArchiveComputers();

    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    TArray<AKeypadScanner*> GetKeypadScanners();

    // Get current puzzle statistics
    UFUNCTION(BlueprintCallable, Category = "Archive Puzzle")
    void GetPuzzleStats(int32& NumComputers, int32& NumScanners, bool& bIsValid);

private:
    FTimerHandle SetupTimerHandle;
    void DelayedSetup();
};