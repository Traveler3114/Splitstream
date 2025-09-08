// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralLevelManager.h"
#include "Computer.h"
#include "KeypadScanner/KeypadScanner.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// Static instance
AProceduralLevelManager* AProceduralLevelManager::Instance = nullptr;

AProceduralLevelManager::AProceduralLevelManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default clue templates
    ClueTemplates.Add(TEXT("Check the archive terminal for access code: {CODE}"));
    ClueTemplates.Add(TEXT("Security clearance required. Code found in system: {CODE}"));
    ClueTemplates.Add(TEXT("Archive entry protected. Authentication: {CODE}"));
    ClueTemplates.Add(TEXT("Temporal data locked. Access sequence: {CODE}"));
    ClueTemplates.Add(TEXT("Historical records secured. Unlock key: {CODE}"));
}

void AProceduralLevelManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set this as the global instance
    Instance = this;
    
    // Generate and distribute codes when the level starts
    GenerateAndDistributeCodes();
}

void AProceduralLevelManager::GenerateAndDistributeCodes()
{
    if (!HasAuthority())
    {
        return; // Only server generates codes
    }

    // Clear existing data
    GeneratedCodes.Empty();
    CodeToComputerMap.Empty();

    // Find available computers
    TArray<AComputer*> AvailableComputers = FindAvailableComputers();
    
    if (AvailableComputers.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralLevelManager: No computers found in level"));
        return;
    }

    // Generate codes and assign them to computers
    int32 CodesToGenerate = FMath::Min(NumberOfCodesToGenerate, AvailableComputers.Num());
    
    for (int32 i = 0; i < CodesToGenerate; ++i)
    {
        FString NewCode = GenerateRandomCode();
        GeneratedCodes.Add(NewCode);
        
        // Assign code to a random available computer
        int32 ComputerIndex = FMath::RandRange(0, AvailableComputers.Num() - 1);
        AComputer* SelectedComputer = AvailableComputers[ComputerIndex];
        
        // Store the code in the computer
        SelectedComputer->SetStoredCode(NewCode);
        CodeToComputerMap.Add(NewCode, SelectedComputer);
        
        // Remove this computer from available list to avoid duplicates
        AvailableComputers.RemoveAt(ComputerIndex);
        
        UE_LOG(LogTemp, Log, TEXT("ProceduralLevelManager: Generated code %s for computer"), *NewCode);
    }

    // Now assign codes to keypad scanners
    TArray<AKeypadScanner*> Keypads = FindAllKeypads();
    for (AKeypadScanner* Keypad : Keypads)
    {
        if (GeneratedCodes.Num() > 0)
        {
            // Assign a random generated code to this keypad
            int32 CodeIndex = FMath::RandRange(0, GeneratedCodes.Num() - 1);
            FString AssignedCode = GeneratedCodes[CodeIndex];
            Keypad->CorrectCode = AssignedCode;
            
            UE_LOG(LogTemp, Log, TEXT("ProceduralLevelManager: Assigned code %s to keypad"), *AssignedCode);
        }
    }
}

FClueData AProceduralLevelManager::GetRandomClueForCode(const FString& Code)
{
    FClueData ClueData;
    
    if (ClueTemplates.Num() > 0)
    {
        // Select a random clue template
        int32 TemplateIndex = FMath::RandRange(0, ClueTemplates.Num() - 1);
        FString Template = ClueTemplates[TemplateIndex];
        
        // Replace {CODE} with the actual code
        ClueData.ClueText = Template.Replace(TEXT("{CODE}"), *Code);
        ClueData.AssociatedCode = Code;
        
        // If we have a computer associated with this code, use its location
        if (AComputer** ComputerPtr = CodeToComputerMap.Find(Code))
        {
            if (*ComputerPtr)
            {
                ClueData.Location = (*ComputerPtr)->GetActorLocation();
            }
        }
    }
    
    return ClueData;
}

FString AProceduralLevelManager::GenerateRandomCode()
{
    FString Digits = TEXT("0123456789");
    FString NewCode;
    
    for (int32 i = 0; i < CodeLength; ++i)
    {
        int32 Index = FMath::RandRange(0, Digits.Len() - 1);
        NewCode += Digits.Mid(Index, 1);
    }
    
    return NewCode;
}

TArray<AComputer*> AProceduralLevelManager::FindAvailableComputers()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundActors);
    
    TArray<AComputer*> Computers;
    for (AActor* Actor : FoundActors)
    {
        if (AComputer* Computer = Cast<AComputer>(Actor))
        {
            Computers.Add(Computer);
        }
    }
    
    return Computers;
}

TArray<AKeypadScanner*> AProceduralLevelManager::FindAllKeypads()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundActors);
    
    TArray<AKeypadScanner*> Keypads;
    for (AActor* Actor : FoundActors)
    {
        if (AKeypadScanner* Keypad = Cast<AKeypadScanner>(Actor))
        {
            Keypads.Add(Keypad);
        }
    }
    
    return Keypads;
}

AProceduralLevelManager* AProceduralLevelManager::GetInstance(UWorld* World)
{
    if (!Instance && World)
    {
        // Try to find existing instance in the world
        TArray<AActor*> FoundManagers;
        UGameplayStatics::GetAllActorsOfClass(World, AProceduralLevelManager::StaticClass(), FoundManagers);
        
        if (FoundManagers.Num() > 0)
        {
            Instance = Cast<AProceduralLevelManager>(FoundManagers[0]);
        }
    }
    
    return Instance;
}