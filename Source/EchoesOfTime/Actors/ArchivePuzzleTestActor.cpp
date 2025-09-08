// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchivePuzzleTestActor.h"
#include "Actors/ArchiveComputer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AArchivePuzzleTestActor::AArchivePuzzleTestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create the level manager
    LevelManager = CreateDefaultSubobject<UProceduralLevelManager>(TEXT("ProceduralLevelManager"));

    // Set default test configuration
    TestConfig.NumArchiveComputers = 3;
    TestConfig.NumKeypadScanners = 1;
    TestConfig.DifficultyLevel = 1;
    TestConfig.bUseUniqueCodesPerScanner = true;
}

void AArchivePuzzleTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the level manager
    if (LevelManager)
    {
        LevelManager->Initialize(GetWorld());
    }

    if (bAutoGenerateOnBeginPlay)
    {
        // Delay generation to ensure all actors are spawned and ready
        GetWorldTimerManager().SetTimer(GenerationTimerHandle, this, &AArchivePuzzleTestActor::DelayedGeneration, DelayBeforeGeneration, false);
    }
}

void AArchivePuzzleTestActor::DelayedGeneration()
{
    GenerateTestPuzzle();
}

void AArchivePuzzleTestActor::GenerateTestPuzzle()
{
    if (!LevelManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleTestActor: LevelManager is null!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleTestActor: Generating test puzzle..."));

    bool bSuccess = LevelManager->GenerateArchivePuzzle(TestConfig);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleTestActor: Test puzzle generated successfully!"));
        LogPuzzleStatus();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleTestActor: Failed to generate test puzzle!"));
    }
}

void AArchivePuzzleTestActor::ValidateCurrentSetup()
{
    if (!LevelManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleTestActor: LevelManager is null!"));
        return;
    }

    bool bValid = LevelManager->ValidatePuzzleSetup();
    
    FString ValidationMsg = bValid ? TEXT("VALID") : TEXT("INVALID");
    UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleTestActor: Puzzle setup validation: %s"), *ValidationMsg);
    
    if (GEngine)
    {
        FString ScreenMsg = FString::Printf(TEXT("Puzzle Validation: %s"), *ValidationMsg);
        FColor MsgColor = bValid ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, MsgColor, ScreenMsg);
    }
}

void AArchivePuzzleTestActor::ClearTestPuzzle()
{
    if (!LevelManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleTestActor: LevelManager is null!"));
        return;
    }

    LevelManager->ClearPuzzleSetup();
    UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleTestActor: Test puzzle cleared"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Puzzle Cleared"));
    }
}

void AArchivePuzzleTestActor::LogPuzzleStatus()
{
    if (!LevelManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleTestActor: LevelManager is null!"));
        return;
    }

    TArray<AArchiveComputer*> ArchiveComputers = LevelManager->GetManagedArchiveComputers();
    TArray<AKeypadScanner*> KeypadScanners = LevelManager->GetManagedKeypadScanners();

    UE_LOG(LogTemp, Log, TEXT("=== PUZZLE STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Archive Computers: %d"), ArchiveComputers.Num());
    
    for (int32 i = 0; i < ArchiveComputers.Num(); ++i)
    {
        AArchiveComputer* Computer = ArchiveComputers[i];
        if (Computer)
        {
            FString ComputerInfo = FString::Printf(TEXT("  [%d] %s - Code: %s"), 
                                                  i, *Computer->GetDisplayName(), *Computer->StoredCode);
            UE_LOG(LogTemp, Log, TEXT("%s"), *ComputerInfo);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Keypad Scanners: %d"), KeypadScanners.Num());
    
    for (int32 i = 0; i < KeypadScanners.Num(); ++i)
    {
        AKeypadScanner* Scanner = KeypadScanners[i];
        if (Scanner)
        {
            FString ScannerInfo = FString::Printf(TEXT("  [%d] Scanner - Code: %s"), 
                                                 i, *Scanner->CorrectCode);
            UE_LOG(LogTemp, Log, TEXT("%s"), *ScannerInfo);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("===================="));

    // Also display on screen for easy testing
    if (GEngine)
    {
        FString StatusMsg = FString::Printf(TEXT("Puzzle: %d Computers, %d Scanners"), 
                                          ArchiveComputers.Num(), KeypadScanners.Num());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, StatusMsg);
    }
}