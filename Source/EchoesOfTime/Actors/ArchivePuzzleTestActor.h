// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralLevelManager.h"
#include "ArchivePuzzleTestActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ECHOESOFTIME_API AArchivePuzzleTestActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AArchivePuzzleTestActor();

protected:
    virtual void BeginPlay() override;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    FPuzzleConfiguration TestConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float DelayBeforeGeneration = 2.0f;

    // Procedural level manager instance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle")
    UProceduralLevelManager* LevelManager;

public:    
    // Test functions
    UFUNCTION(BlueprintCallable, Category = "Test")
    void GenerateTestPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Test")
    void ValidateCurrentSetup();

    UFUNCTION(BlueprintCallable, Category = "Test")
    void ClearTestPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Test")
    void LogPuzzleStatus();

private:
    FTimerHandle GenerationTimerHandle;
    void DelayedGeneration();
};