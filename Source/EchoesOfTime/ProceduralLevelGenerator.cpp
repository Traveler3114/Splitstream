#include "ProceduralLevelGenerator.h"
#include "Actors/PointActors/CivilianSpawnPoint.h"
#include "Characters/CivilianCharacter.h"
#include "Actors/Computers/Computer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Actors/NewspaperActor.h"
#include "Actors/PointActors/RandomPointActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimelineEra.h"

AProceduralLevelGenerator::AProceduralLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AProceduralLevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    HandlePastSpawns();
    // Optionally: HandleFutureSpawns();
}

void AProceduralLevelGenerator::HandlePastSpawns()
{
    // ---- CIVILIAN SPAWN ----
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACivilianSpawnPoint::StaticClass(), SpawnPoints);

    TArray<ACivilianCharacter*> SpawnedCivilians;
    TSet<FString> UsedNames;

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

    for (AActor* Actor : SpawnPoints)
    {
        ACivilianSpawnPoint* SpawnPoint = Cast<ACivilianSpawnPoint>(Actor);
        if (SpawnPoint && SpawnPoint->TimelineEra == ETimelineEra::Past)
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
                Civilian->TimelineEra = ETimelineEra::Past;
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

    // ---- COMPUTER SETUP ----
    TArray<AActor*> FoundComputers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundComputers);

    TArray<AComputer*> PastComputers;
    for (AActor* Actor : FoundComputers)
    {
        AComputer* Computer = Cast<AComputer>(Actor);
        if (Computer && Computer->TimelineEra == ETimelineEra::Past)
        {
            PastComputers.Add(Computer);
        }
    }

    // Assign computer staff names from civilian names if possible
    TArray<FString> StaffNames;
    for (ACivilianCharacter* Civ : SpawnedCivilians)
    {
        StaffNames.Add(Civ->CivilianName);
    }
    for (int32 i = StaffNames.Num(); i < PastComputers.Num(); ++i)
    {
        StaffNames.Add(FString::Printf(TEXT("Staff%02d"), i + 1));
    }
    for (int32 i = 0; i < StaffNames.Num(); ++i)
    {
        int32 SwapIdx = FMath::RandRange(0, StaffNames.Num() - 1);
        StaffNames.Swap(i, SwapIdx);
    }
    for (int32 i = 0; i < PastComputers.Num(); ++i)
    {
        PastComputers[i]->SetupComputer(StaffNames[i], PastComputers[i]->StoredCode);
    }

    // ---- KEYPAD SETUP ----
    TArray<AActor*> FoundKeypads;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundKeypads);

    for (AActor* Actor : FoundKeypads)
    {
        AKeypadScanner* Keypad = Cast<AKeypadScanner>(Actor);
        if (Keypad && Keypad->TimelineEra == ETimelineEra::Past)
        {
            FString ThisKeypadCode = GenerateRandomCode(4); // Generates random code
            Keypad->SetCorrectCode(ThisKeypadCode);

            // Only store code on ONE random past computer
            if (Keypad->bStoreCodeOnComputer && PastComputers.Num() > 0)
            {
                int32 RandIndex = FMath::RandRange(0, PastComputers.Num() - 1);
                if (HasAuthority()) {
                    PastComputers[RandIndex]->SetupComputer(PastComputers[RandIndex]->StaffName, ThisKeypadCode);
                }
            }
        }
    }

    // ---- RANDOM DATE FOR PUZZLE ----
    RandomDate = GenerateRandomDate(); // Generates random date

    // ---- NEWSPAPER SPAWN ----
    TArray<AActor*> NewspaperPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARandomPointActor::StaticClass(), NewspaperPoints);

    TArray<ARandomPointActor*> PastRandomPoints;
    for (AActor* Actor : NewspaperPoints) {
        ARandomPointActor* Point = Cast<ARandomPointActor>(Actor);
        if (Point && Point->TimelineEra == ETimelineEra::Past) {
            PastRandomPoints.Add(Point);
        }
    }
    if (PastRandomPoints.Num() > 0)
    {
        int32 SpawnIndex = FMath::RandRange(0, PastRandomPoints.Num() - 1);
        FVector SpawnLocation = PastRandomPoints[SpawnIndex]->GetActorLocation();
        FRotator SpawnRotation = PastRandomPoints[SpawnIndex]->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        ANewspaperActor* Newspaper = GetWorld()->SpawnActor<ANewspaperActor>(ANewspaperActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
        if (Newspaper)
        {
            Newspaper->TimelineEra = ETimelineEra::Past;
            FString DateStr = FString::Printf(TEXT("%d-%02d-%02d"), RandomDate.Year, RandomDate.Month, RandomDate.Day);
            Newspaper->SetDateText(DateStr);
        }
    }

    // ---- PAIR CIVILIANS TO COMPUTERS (Past only) ----
    int32 PairCount = FMath::Min(SpawnedCivilians.Num(), PastComputers.Num());
    for (int32 i = 0; i < PairCount; ++i)
    {
        PastComputers[i]->SetupComputer(SpawnedCivilians[i]->CivilianName, PastComputers[i]->StoredCode);
        SpawnedCivilians[i]->AssignedComputer = PastComputers[i];
    }
    for (int32 i = PairCount; i < PastComputers.Num(); ++i)
    {
        PastComputers[i]->SetupComputer(FString::Printf(TEXT("Staff%02d"), i + 1), PastComputers[i]->StoredCode);
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