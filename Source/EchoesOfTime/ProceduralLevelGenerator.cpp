#include "ProceduralLevelGenerator.h"
#include "Actors/Computer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Actors/NewspaperActor.h"
#include "Actors/RandomPointActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AProceduralLevelGenerator::AProceduralLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AProceduralLevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundComputers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundComputers);

    TArray<FString> StaffNames = GenerateShuffledStaffNames(FoundComputers.Num());

    for (int32 i = 0; i < FoundComputers.Num(); ++i)
    {
        AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
        if (Computer)
        {
            Computer->SetStaffName(StaffNames[i]);
            Computer->SetStoredCode(""); // Default to empty code
        }
    }

    KeypadCode = GenerateRandomCode(4);

    CodeComputerRef = nullptr;
    CodeComputerStaffName = "";
    if (FoundComputers.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FoundComputers.Num() - 1);
        AComputer* CodeComputer = Cast<AComputer>(FoundComputers[RandomIndex]);
        if (CodeComputer)
        {
            CodeComputer->SetStoredCode(KeypadCode);
            CodeComputerRef = CodeComputer;
            CodeComputerStaffName = CodeComputer->StaffName;
        }
    }

    TArray<AActor*> FoundKeypads;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundKeypads);

    for (int32 i = 0; i < FoundKeypads.Num(); ++i)
    {
        AKeypadScanner* Keypad = Cast<AKeypadScanner>(FoundKeypads[i]);
        if (Keypad)
        {
            FString ThisKeypadCode = GenerateRandomCode(4);
            Keypad->SetCorrectCode(ThisKeypadCode);

            // Only store code on computer if bStoreCodeOnComputer is true
            if (Keypad->bStoreCodeOnComputer && FoundComputers.Num() > i)
            {
                AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
                if (Computer)
                {
                    Computer->SetStoredCode(ThisKeypadCode);
                }
            }
        }
    }

    RandomDate = GenerateRandomDate();

    // Spawn newspaper at a random RandomPointActor
    TArray<AActor*> NewspaperPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARandomPointActor::StaticClass(), NewspaperPoints);

    if (NewspaperPoints.Num() > 0)
    {
        int32 SpawnIndex = FMath::RandRange(0, NewspaperPoints.Num() - 1);
        FVector SpawnLocation = NewspaperPoints[SpawnIndex]->GetActorLocation();
        FRotator SpawnRotation = NewspaperPoints[SpawnIndex]->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        ANewspaperActor* Newspaper = GetWorld()->SpawnActor<ANewspaperActor>(ANewspaperActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
        if (Newspaper)
        {
            FString DateStr = FString::Printf(TEXT("%d-%02d-%02d"), RandomDate.Year, RandomDate.Month, RandomDate.Day);
            Newspaper->SetDateText(DateStr);
        }
    }

    //Test for debug showing the date on screen
    if (GEngine)
    {
        FString DateMsg = FString::Printf(TEXT("Puzzle Date: %d-%02d-%02d"), RandomDate.Year, RandomDate.Month, RandomDate.Day);
        GEngine->AddOnScreenDebugMessage(12345, 999999.f, FColor::Yellow, DateMsg);
    }
}

// ... rest of your file unchanged ...

FString AProceduralLevelGenerator::GenerateRandomCode(int Length)
{
    FString Digits = "0123456789";
    FString Code;
    for (int i = 0; i < Length; ++i)
    {
        int32 Index = FMath::RandRange(0, Digits.Len() - 1);
        Code += Digits.Mid(Index, 1);
    }
    return Code;
}

TArray<FString> AProceduralLevelGenerator::GenerateShuffledStaffNames(int NumComputers)
{
    TArray<FString> Names;
    for (int32 i = 1; i <= NumComputers; ++i)
    {
        Names.Add(FString::Printf(TEXT("Staff%02d"), i));
    }
    for (int32 i = 0; i < Names.Num(); ++i)
    {
        int32 SwapIdx = FMath::RandRange(0, Names.Num() - 1);
        Names.Swap(i, SwapIdx);
    }
    return Names;
}

FRandomDate AProceduralLevelGenerator::GenerateRandomDate()
{
    FRandomDate Date;
    Date.Year = FMath::RandRange(1990, 2025);
    Date.Month = FMath::RandRange(1, 12);
    int32 DaysInMonth = 31;
    if (Date.Month == 2)
    {
        DaysInMonth = 28;
        if ((Date.Year % 4 == 0 && Date.Year % 100 != 0) || (Date.Year % 400 == 0)) DaysInMonth = 29;
    }
    else if (Date.Month == 4 || Date.Month == 6 || Date.Month == 9 || Date.Month == 11)
    {
        DaysInMonth = 30;
    }
    Date.Day = FMath::RandRange(1, DaysInMonth);
    return Date;
}

void AProceduralLevelGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AProceduralLevelGenerator, RandomDate);
    DOREPLIFETIME(AProceduralLevelGenerator, CodeComputerRef);
    DOREPLIFETIME(AProceduralLevelGenerator, CodeComputerStaffName);
    DOREPLIFETIME(AProceduralLevelGenerator, KeypadCode);
}