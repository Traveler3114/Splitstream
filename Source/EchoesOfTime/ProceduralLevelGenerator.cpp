#include "ProceduralLevelGenerator.h"
#include "Actors/Computers/Computer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Actors/NewspaperActor.h"
#include "Actors/PointActors/RandomPointActor.h"
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

    // Find all computers and assign staff names/codes
    TArray<AActor*> FoundComputers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundComputers);

    TArray<FString> StaffNames = GenerateShuffledStaffNames(FoundComputers.Num());
    for (int32 i = 0; i < FoundComputers.Num(); ++i)
    {
        AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
        if (Computer)
        {
            Computer->SetupComputer(StaffNames[i], TEXT(""));
        }
    }
    // Setup keypads and optionally computers
    //Setup keypads and optionally computers
    TArray<AActor*> FoundKeypads;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundKeypads);
    for (int32 i = 0; i < FoundKeypads.Num(); ++i)
    {
        AKeypadScanner* Keypad = Cast<AKeypadScanner>(FoundKeypads[i]);
        if (Keypad)
        {
            FString ThisKeypadCode = GenerateRandomCode(4);
            Keypad->SetCorrectCode(ThisKeypadCode);

            // Only store code on ONE random computer
            if (Keypad->bStoreCodeOnComputer && FoundComputers.Num() > 0)
            {
                int32 RandIndex = FMath::RandRange(0, FoundComputers.Num() - 1);
                AComputer* Computer = Cast<AComputer>(FoundComputers[RandIndex]);
                if (Computer)
                {
                    Computer->StoredCode = ThisKeypadCode;
                }
            }
        }
    }

    // Generate and assign puzzle date
    RandomDate = GenerateRandomDate();

    // Spawn newspaper at a random location
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
}

FString AProceduralLevelGenerator::GenerateRandomCode(int Length) const
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

TArray<FString> AProceduralLevelGenerator::GenerateShuffledStaffNames(int NumComputers) const
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

FRandomDate AProceduralLevelGenerator::GenerateRandomDate() const
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
}