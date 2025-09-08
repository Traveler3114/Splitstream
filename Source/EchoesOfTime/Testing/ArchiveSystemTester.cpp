// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchiveSystemTester.h"
#include "../Actors/ProceduralLevelManager.h"
#include "../Actors/ArchiveComputer.h"
#include "../Actors/KeypadScanner/KeypadScanner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AArchiveSystemTester::AArchiveSystemTester()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AArchiveSystemTester::BeginPlay()
{
    Super::BeginPlay();
    
    // Run tests after a brief delay to ensure all systems are initialized
    FTimerHandle TestTimer;
    GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &AArchiveSystemTester::RunArchiveSystemTests, 2.0f, false);
}

void AArchiveSystemTester::RunArchiveSystemTests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Archive System Tests Starting ==="));
    
    // Run individual tests
    bProceduralManagerTest = TestProceduralManager();
    bArchiveComputerTest = TestArchiveComputer();
    bKeypadIntegrationTest = TestKeypadIntegration();
    
    // Log overall results
    int32 PassedTests = 0;
    int32 TotalTests = 3;
    
    if (bProceduralManagerTest) PassedTests++;
    if (bArchiveComputerTest) PassedTests++;
    if (bKeypadIntegrationTest) PassedTests++;
    
    UE_LOG(LogTemp, Warning, TEXT("=== Archive System Tests Complete: %d/%d Passed ==="), PassedTests, TotalTests);
    
    // Display results on screen
    if (GEngine)
    {
        FString ResultMessage = FString::Printf(
            TEXT("Archive System Tests: %d/%d Passed\nProceduralManager: %s\nArchiveComputer: %s\nKeypadIntegration: %s"),
            PassedTests, TotalTests,
            bProceduralManagerTest ? TEXT("PASS") : TEXT("FAIL"),
            bArchiveComputerTest ? TEXT("PASS") : TEXT("FAIL"),
            bKeypadIntegrationTest ? TEXT("PASS") : TEXT("FAIL")
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, 
            PassedTests == TotalTests ? FColor::Green : FColor::Red, 
            ResultMessage);
    }
}

bool AArchiveSystemTester::TestProceduralManager()
{
    LogTestResult(TEXT("ProceduralManager"), false);
    
    // Test 1: Check if manager exists or can be created
    AProceduralLevelManager* Manager = AProceduralLevelManager::GetInstance(GetWorld());
    if (!Manager)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralManager test: No manager found, attempting to spawn one"));
        
        // Try to spawn a manager for testing
        FActorSpawnParameters SpawnParams;
        Manager = GetWorld()->SpawnActor<AProceduralLevelManager>(SpawnParams);
        
        if (!Manager)
        {
            LogTestResult(TEXT("ProceduralManager"), false);
            return false;
        }
    }
    
    // Test 2: Check if manager can generate codes
    TArray<FString> GeneratedCodes = Manager->GetAllCodes();
    if (GeneratedCodes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralManager test: No codes generated, triggering generation"));
        Manager->GenerateAndDistributeCodes();
        GeneratedCodes = Manager->GetAllCodes();
    }
    
    bool bSuccess = GeneratedCodes.Num() > 0;
    
    // Test 3: Check if clues can be generated
    if (bSuccess && GeneratedCodes.Num() > 0)
    {
        FClueData TestClue = Manager->GetRandomClueForCode(GeneratedCodes[0]);
        bSuccess = !TestClue.ClueText.IsEmpty();
    }
    
    LogTestResult(TEXT("ProceduralManager"), bSuccess);
    return bSuccess;
}

bool AArchiveSystemTester::TestArchiveComputer()
{
    LogTestResult(TEXT("ArchiveComputer"), false);
    
    // Test 1: Try to spawn an archive computer
    FActorSpawnParameters SpawnParams;
    AArchiveComputer* TestComputer = GetWorld()->SpawnActor<AArchiveComputer>(SpawnParams);
    
    if (!TestComputer)
    {
        LogTestResult(TEXT("ArchiveComputer"), false);
        return false;
    }
    
    // Test 2: Check if archive computer has basic functionality
    bool bSuccess = true;
    
    // Check if it has archive entries
    TArray<FString> ArchiveEntries = TestComputer->GetArchiveEntries();
    if (ArchiveEntries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchiveComputer test: No archive entries found"));
        bSuccess = false;
    }
    
    // Test 3: Check access levels
    EArchiveAccessLevel InitialLevel = TestComputer->GetAccessLevel();
    TestComputer->SetAccessLevel(EArchiveAccessLevel::Level2);
    EArchiveAccessLevel NewLevel = TestComputer->GetAccessLevel();
    
    if (NewLevel != EArchiveAccessLevel::Level2)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchiveComputer test: Access level setting failed"));
        bSuccess = false;
    }
    
    // Clean up test computer
    TestComputer->Destroy();
    
    LogTestResult(TEXT("ArchiveComputer"), bSuccess);
    return bSuccess;
}

bool AArchiveSystemTester::TestKeypadIntegration()
{
    LogTestResult(TEXT("KeypadIntegration"), false);
    
    // Test 1: Check if keypad scanner can work with the system
    FActorSpawnParameters SpawnParams;
    AKeypadScanner* TestKeypad = GetWorld()->SpawnActor<AKeypadScanner>(SpawnParams);
    
    if (!TestKeypad)
    {
        LogTestResult(TEXT("KeypadIntegration"), false);
        return false;
    }
    
    bool bSuccess = true;
    
    // Test 2: Check if keypad has a code assigned
    if (TestKeypad->CorrectCode.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("KeypadIntegration test: No code assigned to keypad"));
        bSuccess = false;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("KeypadIntegration test: Keypad has code: %s"), *TestKeypad->CorrectCode);
    }
    
    // Test 3: Check if code is valid format (should be 4 digits)
    if (bSuccess)
    {
        if (TestKeypad->CorrectCode.Len() != 4 || !TestKeypad->CorrectCode.IsNumeric())
        {
            UE_LOG(LogTemp, Warning, TEXT("KeypadIntegration test: Code format invalid: %s"), *TestKeypad->CorrectCode);
            bSuccess = false;
        }
    }
    
    // Clean up test keypad
    TestKeypad->Destroy();
    
    LogTestResult(TEXT("KeypadIntegration"), bSuccess);
    return bSuccess;
}

void AArchiveSystemTester::LogTestResult(const FString& TestName, bool bPassed)
{
    FString Status = bPassed ? TEXT("PASSED") : TEXT("FAILED");
    FColor LogColor = bPassed ? FColor::Green : FColor::Red;
    
    UE_LOG(LogTemp, Warning, TEXT("%s Test: %s"), *TestName, *Status);
}