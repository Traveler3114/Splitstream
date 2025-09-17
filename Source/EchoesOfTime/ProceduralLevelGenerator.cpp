#include "ProceduralLevelGenerator.h"
#include "Actors/PointActors/CivilianSpawnPoint.h"
#include "Characters/CivilianCharacter.h"
#include "Actors/Computers/Computer.h"
#include "Actors/KeypadScanner/KeypadScanner.h"
#include "Actors/NewspaperActor.h"
#include "Actors/PointActors/RandomPointActor.h"
#include "Actors/PointActors/SearchableItemSpawnPoint.h"
#include "Actors/SearchableActor.h"
#include "Actors/CupActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimelineEra.h"
#include "Actors/DeskActor.h"
#include "Actors/CodeGenerator.h"

AProceduralLevelGenerator::AProceduralLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AProceduralLevelGenerator::BeginPlay()
{
    Super::BeginPlay();
    if(HasAuthority()) HandlePastSpawns();
    //HandleFutureSpawns();
}

FRandomDate AProceduralLevelGenerator::GeneratePastDate() const
{
    // You can customize the range if you want
    return GenerateRandomDate();
}

FRandomDate AProceduralLevelGenerator::GenerateFutureDate(const FRandomDate& MinDate) const
{
    FRandomDate Date;
    do
    {
        Date = GenerateRandomDate();
    }
    while (!(MinDate < Date)); // Repeat until Date is after MinDate
    return Date;
}

void AProceduralLevelGenerator::SpawnCivilianDeskItems(const TArray<class ACivilianCharacter*>& Civilians, TSubclassOf<class ASearchableActor> ItemClass)
{
    for (ACivilianCharacter* Civ : Civilians)
    {
        if (!Civ || !Civ->AssignedDesk) continue;

        for (ASearchableItemSpawnPoint* SpawnPoint : Civ->AssignedDesk->ItemSpawnPoints)
        {
            if (!SpawnPoint) continue;

            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            FVector SpawnLocation = SpawnPoint->GetActorLocation();
            FRotator SpawnRotation = SpawnPoint->GetActorRotation();

            ASearchableActor* NewItem = GetWorld()->SpawnActor<ASearchableActor>(ItemClass, SpawnLocation, SpawnRotation, Params);

            if (NewItem)
            {
                // Set the TimelineEra
                NewItem->TimelineEra = Civ->TimelineEra;

                // If it's a cup, set the LinkedCivilian
                ACupActor* Cup = Cast<ACupActor>(NewItem);
                if (Cup)
                {
                    Cup->LinkedCivilian = Civ;
                }
            }
        }
    }
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

    // ---- DESK SETUP ----
    TArray<AActor*> FoundDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeskActor::StaticClass(), FoundDesks);

    TArray<ADeskActor*> PastDesks;
    for (AActor* Actor : FoundDesks)
    {
        ADeskActor* Desk = Cast<ADeskActor>(Actor);
        if (Desk && Desk->TimelineEra == ETimelineEra::Past)
        {
            PastDesks.Add(Desk);
        }
    }

    // ---- PAIR CIVILIANS TO DESKS ----
    int32 PairCount = FMath::Min(SpawnedCivilians.Num(), PastDesks.Num());
    for (int32 i = 0; i < PairCount; ++i)
    {
        ACivilianCharacter* Civ = SpawnedCivilians[i];
        ADeskActor* Desk = PastDesks[i];

        Civ->AssignedDesk = Desk;
        Desk->SetStaffName(Civ->CivilianName);
    }

    // ---- VAULT KEYPAD & COMPUTER CODE ASSIGNMENT ----
    TArray<ADeskActor*> DesksWithComputer;
    for (ADeskActor* Desk : PastDesks)
    {
        if (Desk && Desk->DeskComputer)
            DesksWithComputer.Add(Desk);
    }

    TArray<AActor*> FoundKeypads;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundKeypads);

    FString TheKeypadCode;
    if (DesksWithComputer.Num() > 0 && FoundKeypads.Num() > 0)
    {
        TheKeypadCode = GenerateRandomCode(4);

        int32 RandDeskIdx = FMath::RandRange(0, DesksWithComputer.Num() - 1);
        for (int32 i = 0; i < DesksWithComputer.Num(); ++i)
        {
            if (DesksWithComputer[i]->DeskComputer)
            {
                if (i == RandDeskIdx)
                {
                    DesksWithComputer[i]->DeskComputer->SetupComputer(TEXT(""), TheKeypadCode);
                }
                else
                {
                    DesksWithComputer[i]->DeskComputer->SetupComputer(TEXT(""), TEXT(""));
                }
            }
        }

        for (AActor* Actor : FoundKeypads)
        {
            AKeypadScanner* Keypad = Cast<AKeypadScanner>(Actor);
            if (Keypad && Keypad->TimelineEra == ETimelineEra::Past)
            {
                Keypad->SetCorrectCode(TheKeypadCode);
            }
        }
    }

    // ---- RANDOM DATE FOR PUZZLE ----
    PastDate = GeneratePastDate();

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
        ANewspaperActor* Newspaper = GetWorld()->SpawnActor<ANewspaperActor>(NewspaperBPClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (Newspaper)
        {
            Newspaper->TimelineEra = ETimelineEra::Past;
            FString DateStr = FString::Printf(TEXT("%d-%02d-%02d"), PastDate.Year, PastDate.Month, PastDate.Day);
            Newspaper->SetDateText(DateStr);
        }
    }

    SpawnCivilianDeskItems(SpawnedCivilians, SearchableItemBPClass);
    HandleFutureSpawns();
}

void AProceduralLevelGenerator::HandleFutureSpawns()
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
        if (SpawnPoint && SpawnPoint->TimelineEra == ETimelineEra::Future)
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
                Civilian->TimelineEra = ETimelineEra::Future;
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

    // ---- DESK SETUP ----
    TArray<AActor*> FoundDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeskActor::StaticClass(), FoundDesks);

    TArray<ADeskActor*> FutureDesks;
    for (AActor* Actor : FoundDesks)
    {
        ADeskActor* Desk = Cast<ADeskActor>(Actor);
        if (Desk && Desk->TimelineEra == ETimelineEra::Future)
        {
            FutureDesks.Add(Desk);
        }
    }

    // ---- PAIR CIVILIANS TO DESKS ----
    int32 PairCount = FMath::Min(SpawnedCivilians.Num(), FutureDesks.Num());
    for (int32 i = 0; i < PairCount; ++i)
    {
        ACivilianCharacter* Civ = SpawnedCivilians[i];
        ADeskActor* Desk = FutureDesks[i];

        Civ->AssignedDesk = Desk;
        Desk->SetStaffName(Civ->CivilianName);
    }

    ACivilianCharacter* TargetCivilian = nullptr;
    if (SpawnedCivilians.Num() > 0)
    {
        int32 VaultIdx = FMath::RandRange(0, SpawnedCivilians.Num() - 1);
        TargetCivilian = SpawnedCivilians[VaultIdx];
    }

    // ---- ASSIGN TO CODE GENERATOR(S) ----
    TArray<AActor*> FoundCodeGenerators;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACodeGenerator::StaticClass(), FoundCodeGenerators);
    for (AActor* Actor : FoundCodeGenerators)
    {
        ACodeGenerator* CodeGen = Cast<ACodeGenerator>(Actor);
        if (CodeGen && CodeGen->TimelineEra == ETimelineEra::Future)
        {
            CodeGen->TargetCivilian = TargetCivilian;
        }
    }


    // ---- RANDOM DATE FOR PUZZLE ----
    FutureDate = GenerateFutureDate(PastDate);

    // ---- NEWSPAPER SPAWN ----
    TArray<AActor*> NewspaperPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARandomPointActor::StaticClass(), NewspaperPoints);

    TArray<ARandomPointActor*> FutureRandomPoints;
    for (AActor* Actor : NewspaperPoints) {
        ARandomPointActor* Point = Cast<ARandomPointActor>(Actor);
        if (Point && Point->TimelineEra == ETimelineEra::Future) {
            FutureRandomPoints.Add(Point);
        }
    }
    if (FutureRandomPoints.Num() > 0)
    {
        int32 SpawnIndex = FMath::RandRange(0, FutureRandomPoints.Num() - 1);
        FVector SpawnLocation = FutureRandomPoints[SpawnIndex]->GetActorLocation();
        FRotator SpawnRotation = FutureRandomPoints[SpawnIndex]->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        ANewspaperActor* Newspaper = GetWorld()->SpawnActor<ANewspaperActor>(NewspaperBPClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (Newspaper)
        {
            Newspaper->TimelineEra = ETimelineEra::Future;
            FString DateStr = FString::Printf(TEXT("%d-%02d-%02d"), FutureDate.Year, FutureDate.Month, FutureDate.Day);
            Newspaper->SetDateText(DateStr);
        }
    }
	SpawnCivilianDeskItems(SpawnedCivilians, SearchableItemBPClass);

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
    DOREPLIFETIME(AProceduralLevelGenerator, PastDate);
    DOREPLIFETIME(AProceduralLevelGenerator, FutureDate);
}