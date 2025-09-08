// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralLevelManager.h"
#include "Actors/ArchiveComputer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UProceduralLevelManager::UProceduralLevelManager()
{
    World = nullptr;
}

void UProceduralLevelManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    UsedCodes.Empty();
}

bool UProceduralLevelManager::GenerateArchivePuzzle(const FPuzzleConfiguration& Config)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralLevelManager: World is null, cannot generate puzzle"));
        return false;
    }

    // Store current configuration
    CurrentConfig = Config;
    
    // Clear any existing setup
    ClearPuzzleSetup();
    
    // Find existing archive computers and keypad scanners in the level
    TArray<AActor*> FoundArchiveComputers;
    TArray<AActor*> FoundKeypadScanners;
    
    UGameplayStatics::GetAllActorsOfClass(World, AArchiveComputer::StaticClass(), FoundArchiveComputers);
    UGameplayStatics::GetAllActorsOfClass(World, AKeypadScanner::StaticClass(), FoundKeypadScanners);
    
    // Setup archive computers
    for (int32 i = 0; i < FMath::Min(Config.NumArchiveComputers, FoundArchiveComputers.Num()); ++i)
    {
        if (AArchiveComputer* ArchiveComp = Cast<AArchiveComputer>(FoundArchiveComputers[i]))
        {
            // Set procedural name
            ArchiveComp->SetProceduralName(TEXT("Archive Terminal"), i);
            ArchiveComp->ArchiveIndex = i;
            
            // Mark the first one as main terminal if desired
            if (i == 0)
            {
                ArchiveComp->bIsMainArchiveTerminal = true;
            }
            
            ManagedArchiveComputers.Add(ArchiveComp);
        }
    }
    
    // Setup keypad scanners
    for (int32 i = 0; i < FMath::Min(Config.NumKeypadScanners, FoundKeypadScanners.Num()); ++i)
    {
        if (AKeypadScanner* Scanner = Cast<AKeypadScanner>(FoundKeypadScanners[i]))
        {
            ManagedKeypadScanners.Add(Scanner);
        }
    }
    
    // Link scanners to computers
    bool bLinkingSuccess = LinkScannersToComputers();
    
    // Validate the setup
    bool bValidSetup = ValidatePuzzleSetup();
    
    if (bLinkingSuccess && bValidSetup)
    {
        UE_LOG(LogTemp, Log, TEXT("ProceduralLevelManager: Successfully generated archive puzzle with %d computers and %d scanners"), 
               ManagedArchiveComputers.Num(), ManagedKeypadScanners.Num());
        
        if (GEngine)
        {
            FString SuccessMsg = FString::Printf(TEXT("Archive Puzzle Generated: %d Terminals, %d Scanners"), 
                                               ManagedArchiveComputers.Num(), ManagedKeypadScanners.Num());
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, SuccessMsg);
        }
        
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("ProceduralLevelManager: Failed to generate archive puzzle"));
    return false;
}

bool UProceduralLevelManager::SetupArchiveComputers(const FPuzzleConfiguration& Config)
{
    // This method is for more granular control - the main logic is in GenerateArchivePuzzle
    return ManagedArchiveComputers.Num() >= Config.NumArchiveComputers;
}

bool UProceduralLevelManager::SetupKeypadScanners(const FPuzzleConfiguration& Config)
{
    // This method is for more granular control - the main logic is in GenerateArchivePuzzle
    return ManagedKeypadScanners.Num() >= Config.NumKeypadScanners;
}

bool UProceduralLevelManager::LinkScannersToComputers()
{
    if (ManagedArchiveComputers.Num() == 0 || ManagedKeypadScanners.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralLevelManager: Cannot link scanners - no computers or scanners available"));
        return false;
    }
    
    // For each keypad scanner, assign a unique code and link to a random archive computer
    for (AKeypadScanner* Scanner : ManagedKeypadScanners)
    {
        if (!Scanner)
            continue;
            
        // Generate a unique code
        FString UniqueCode;
        if (CurrentConfig.bUseUniqueCodesPerScanner)
        {
            UniqueCode = GenerateUniqueCode(UsedCodes);
        }
        else
        {
            UniqueCode = GenerateRandomCode();
        }
        
        // Choose a random archive computer to store the code
        int32 RandomIndex = FMath::RandRange(0, ManagedArchiveComputers.Num() - 1);
        AArchiveComputer* TargetComputer = ManagedArchiveComputers[RandomIndex];
        
        if (TargetComputer)
        {
            // Set the code on both scanner and computer
            Scanner->CorrectCode = UniqueCode;
            TargetComputer->SetStoredCode(UniqueCode);
            
            // Track the used code
            UsedCodes.AddUnique(UniqueCode);
            
            UE_LOG(LogTemp, Log, TEXT("ProceduralLevelManager: Linked Scanner to %s with code %s"), 
                   *TargetComputer->GetDisplayName(), *UniqueCode);
        }
    }
    
    return true;
}

TArray<AArchiveComputer*> UProceduralLevelManager::GetManagedArchiveComputers() const
{
    return ManagedArchiveComputers;
}

TArray<AKeypadScanner*> UProceduralLevelManager::GetManagedKeypadScanners() const
{
    return ManagedKeypadScanners;
}

void UProceduralLevelManager::ClearPuzzleSetup()
{
    ManagedArchiveComputers.Empty();
    ManagedKeypadScanners.Empty();
    UsedCodes.Empty();
}

bool UProceduralLevelManager::ValidatePuzzleSetup() const
{
    // Check if we have the minimum required components
    bool bHasMinimumComputers = ManagedArchiveComputers.Num() > 0;
    bool bHasMinimumScanners = ManagedKeypadScanners.Num() > 0;
    
    // Check if all scanners have valid codes
    bool bAllScannersHaveCodes = true;
    for (const AKeypadScanner* Scanner : ManagedKeypadScanners)
    {
        if (!Scanner || Scanner->CorrectCode.IsEmpty() || !IsValidCode(Scanner->CorrectCode))
        {
            bAllScannersHaveCodes = false;
            break;
        }
    }
    
    // Check if all archive computers have valid codes
    bool bAllComputersHaveCodes = true;
    for (const AArchiveComputer* Computer : ManagedArchiveComputers)
    {
        if (!Computer || Computer->StoredCode.IsEmpty() || !IsValidCode(Computer->StoredCode))
        {
            // It's okay if some computers don't have codes, as long as we have enough with codes
            continue;
        }
    }
    
    return bHasMinimumComputers && bHasMinimumScanners && bAllScannersHaveCodes;
}

FString UProceduralLevelManager::GenerateRandomCode(int32 Length) const
{
    FString Code;
    FString Digits = TEXT("0123456789");
    
    for (int32 i = 0; i < Length; ++i)
    {
        int32 RandomIndex = FMath::RandRange(0, Digits.Len() - 1);
        Code += Digits.Mid(RandomIndex, 1);
    }
    
    return Code;
}

FString UProceduralLevelManager::GenerateUniqueCode(const TArray<FString>& ExistingCodes, int32 Length) const
{
    FString NewCode;
    int32 MaxAttempts = 1000; // Prevent infinite loops
    int32 Attempts = 0;
    
    do
    {
        NewCode = GenerateRandomCode(Length);
        Attempts++;
    } 
    while (ExistingCodes.Contains(NewCode) && Attempts < MaxAttempts);
    
    if (Attempts >= MaxAttempts)
    {
        // Fallback: append a unique number
        NewCode = GenerateRandomCode(Length - 1) + FString::FromInt(ExistingCodes.Num());
    }
    
    return NewCode;
}

bool UProceduralLevelManager::IsValidCode(const FString& Code) const
{
    // Check if code has valid length and only contains digits
    if (Code.Len() < 3 || Code.Len() > 8)
    {
        return false;
    }
    
    for (int32 i = 0; i < Code.Len(); ++i)
    {
        if (!FChar::IsDigit(Code[i]))
        {
            return false;
        }
    }
    
    return true;
}