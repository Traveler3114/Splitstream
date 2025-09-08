// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchivePuzzleComponent.h"
#include "Actors/ArchiveComputer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UArchivePuzzleComponent::UArchivePuzzleComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Create the level manager
    LevelManager = CreateDefaultSubobject<UProceduralLevelManager>(TEXT("ProceduralLevelManager"));

    // Set default configuration
    PuzzleConfig.NumArchiveComputers = 3;
    PuzzleConfig.NumKeypadScanners = 1;
    PuzzleConfig.DifficultyLevel = 1;
    PuzzleConfig.bUseUniqueCodesPerScanner = true;

    bAutoSetupOnBeginPlay = false;
    SetupDelay = 3.0f;
}

void UArchivePuzzleComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the level manager
    if (LevelManager)
    {
        LevelManager->Initialize(GetWorld());
    }

    if (bAutoSetupOnBeginPlay)
    {
        // Delay setup to ensure all actors are spawned
        GetWorld()->GetTimerManager().SetTimer(SetupTimerHandle, this, &UArchivePuzzleComponent::DelayedSetup, SetupDelay, false);
    }
}

void UArchivePuzzleComponent::DelayedSetup()
{
    SetupArchivePuzzle();
}

bool UArchivePuzzleComponent::SetupArchivePuzzle()
{
    return SetupArchivePuzzleWithConfig(PuzzleConfig);
}

bool UArchivePuzzleComponent::SetupArchivePuzzleWithConfig(const FPuzzleConfiguration& Config)
{
    if (!LevelManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleComponent: LevelManager is null!"));
        return false;
    }

    bool bSuccess = LevelManager->GenerateArchivePuzzle(Config);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleComponent: Archive puzzle setup successfully"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Archive Puzzle Setup Complete"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ArchivePuzzleComponent: Failed to setup archive puzzle"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Archive Puzzle Setup Failed"));
        }
    }

    return bSuccess;
}

void UArchivePuzzleComponent::ClearArchivePuzzle()
{
    if (LevelManager)
    {
        LevelManager->ClearPuzzleSetup();
        UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleComponent: Archive puzzle cleared"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Archive Puzzle Cleared"));
        }
    }
}

bool UArchivePuzzleComponent::ValidateArchivePuzzle()
{
    if (!LevelManager)
    {
        return false;
    }

    bool bValid = LevelManager->ValidatePuzzleSetup();
    
    FString ValidationMsg = bValid ? TEXT("Valid") : TEXT("Invalid");
    UE_LOG(LogTemp, Log, TEXT("ArchivePuzzleComponent: Puzzle validation: %s"), *ValidationMsg);
    
    if (GEngine)
    {
        FString ScreenMsg = FString::Printf(TEXT("Puzzle Validation: %s"), *ValidationMsg);
        FColor MsgColor = bValid ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, MsgColor, ScreenMsg);
    }

    return bValid;
}

TArray<AArchiveComputer*> UArchivePuzzleComponent::GetArchiveComputers()
{
    if (LevelManager)
    {
        return LevelManager->GetManagedArchiveComputers();
    }
    return TArray<AArchiveComputer*>();
}

TArray<AKeypadScanner*> UArchivePuzzleComponent::GetKeypadScanners()
{
    if (LevelManager)
    {
        return LevelManager->GetManagedKeypadScanners();
    }
    return TArray<AKeypadScanner*>();
}

void UArchivePuzzleComponent::GetPuzzleStats(int32& NumComputers, int32& NumScanners, bool& bIsValid)
{
    if (LevelManager)
    {
        TArray<AArchiveComputer*> Computers = LevelManager->GetManagedArchiveComputers();
        TArray<AKeypadScanner*> Scanners = LevelManager->GetManagedKeypadScanners();
        
        NumComputers = Computers.Num();
        NumScanners = Scanners.Num();
        bIsValid = LevelManager->ValidatePuzzleSetup();
    }
    else
    {
        NumComputers = 0;
        NumScanners = 0;
        bIsValid = false;
    }
}