#include "ProceduralLevelGenerator.h"
#include "Actors/Computers/Computer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Actors/NewspaperActor.h"
#include "Actors/PointActors/RandomPointActor.h"
#include "Actors/PointActors/CivilianSpawnPoint.h"
#include "Characters/CivilianCharacter.h"
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

    //---- CIVILIAN SPAWN AND NAME GENERATION ----
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACivilianSpawnPoint::StaticClass(), SpawnPoints);

    // Name banks
    TArray<FString> FirstNames = {
        TEXT("John"), TEXT("Laura"), TEXT("Michael"), TEXT("Sarah"), TEXT("David"),
        TEXT("Emily"), TEXT("James"), TEXT("Olivia"), TEXT("Daniel"), TEXT("Sophia"),
        TEXT("Chris"), TEXT("Jessica"), TEXT("Ethan"), TEXT("Anna"), TEXT("Ryan"),
        TEXT("Megan"), TEXT("Luke"), TEXT("Chloe"), TEXT("Nathan"), TEXT("Grace")
    };
    TArray<FString> Surnames = {
        TEXT("Smith"), TEXT("Morgan"), TEXT("Davis"), TEXT("Lee"), TEXT("Clark"),
        TEXT("Turner"), TEXT("Harris"), TEXT("Bennett"), TEXT("Evans"), TEXT("Carter"),
        TEXT("Adams"), TEXT("Wright"), TEXT("Green"), TEXT("Hill"), TEXT("Cook"),
        TEXT("Lewis"), TEXT("Roberts"), TEXT("Walker"), TEXT("Young"), TEXT("King")
    };

    TArray<ACivilianCharacter*> SpawnedCivilians;
    TSet<FString> UsedNames;

    for (int32 i = 0; i < SpawnPoints.Num(); ++i)
    {
        ACivilianSpawnPoint* SpawnPoint = Cast<ACivilianSpawnPoint>(SpawnPoints[i]);
        if (SpawnPoint)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            ACivilianCharacter* Civilian = GetWorld()->SpawnActor<ACivilianCharacter>(
                CivilianBPClass,
                SpawnPoint->GetActorLocation(),
                SpawnPoint->GetActorRotation(),
                Params
            );

            if (Civilian)
            {
                // Generate a unique name
                FString Name;
                do {
                    int32 FirstIdx = FMath::RandRange(0, FirstNames.Num() - 1);
                    int32 LastIdx = FMath::RandRange(0, Surnames.Num() - 1);
                    Name = FirstNames[FirstIdx] + TEXT(" ") + Surnames[LastIdx];
                } while (UsedNames.Contains(Name));
                UsedNames.Add(Name);

                Civilian->CivilianName = Name;
                SpawnedCivilians.Add(Civilian);
            }
        }
    }

    //---- COMPUTER SETUP ----
    TArray<AActor*> FoundComputers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundComputers);

    // Assign computer staff names from civilian names if possible
    TArray<FString> StaffNames;
    for (ACivilianCharacter* Civ : SpawnedCivilians)
    {
        StaffNames.Add(Civ->CivilianName);
    }
    // If more computers than civilians, add generic names
    for (int32 i = StaffNames.Num(); i < FoundComputers.Num(); ++i)
    {
        StaffNames.Add(FString::Printf(TEXT("Staff%02d"), i + 1));
    }
    // Shuffle names for randomness
    for (int32 i = 0; i < StaffNames.Num(); ++i)
    {
        int32 SwapIdx = FMath::RandRange(0, StaffNames.Num() - 1);
        StaffNames.Swap(i, SwapIdx);
    }
    for (int32 i = 0; i < FoundComputers.Num(); ++i)
    {
        AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
        if (Computer)
        {
            Computer->SetupComputer(StaffNames[i], Computer->StoredCode);
        }
    }

    //---- KEYPAD SETUP ----
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
                if (HasAuthority()) {
                    Computer->SetupComputer(Computer->StaffName, ThisKeypadCode);
                }
            }
        }
    }

    //---- RANDOM DATE FOR PUZZLE ----
    RandomDate = GenerateRandomDate();

    //---- NEWSPAPER SPAWN ----
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

    int32 PairCount = FMath::Min(SpawnedCivilians.Num(), FoundComputers.Num());

    for (int32 i = 0; i < PairCount; ++i)
    {
        AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
        ACivilianCharacter* Civilian = SpawnedCivilians[i];
        if (Computer && Civilian)
        {
            Computer->SetupComputer(Civilian->CivilianName, Computer->StoredCode);
            Civilian->AssignedComputer = Computer;
        }
    }

    // If there are more computers than civilians, assign the remaining computers generic names
    for (int32 i = PairCount; i < FoundComputers.Num(); ++i)
    {
        AComputer* Computer = Cast<AComputer>(FoundComputers[i]);
        if (Computer)
        {
            Computer->SetupComputer(FString::Printf(TEXT("Staff%02d"), i + 1), Computer->StoredCode);
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