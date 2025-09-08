// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchiveSystemTester.generated.h"

// Forward declarations
class AProceduralLevelManager;
class AArchiveComputer;
class AKeypadScanner;

UCLASS()
class ECHOESOFTIME_API AArchiveSystemTester : public AActor
{
    GENERATED_BODY()

public:
    AArchiveSystemTester();

    virtual void BeginPlay() override;

    // Test functions
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunArchiveSystemTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestProceduralManager();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestArchiveComputer();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestKeypadIntegration();

protected:
    // Test results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
    bool bProceduralManagerTest = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
    bool bArchiveComputerTest = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
    bool bKeypadIntegrationTest = false;

    // Log test results
    void LogTestResult(const FString& TestName, bool bPassed);
};