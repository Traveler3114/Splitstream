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
#include "Actors/Lever/LeverManager.h"

AProceduralLevelGenerator::AProceduralLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AProceduralLevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    // Find LeverManager in level (or spawn one)


    if (HasAuthority()) HandlePastSpawns();
    //HandleFutureSpawns();
}

FRandomDate AProceduralLevelGenerator::GeneratePastDate() const
{
    return GenerateRandomDate();
}

FRandomDate AProceduralLevelGenerator::GenerateFutureDate(const FRandomDate& MinDate) const
{
    FRandomDate Date;
    do
    {
        Date = GenerateRandomDate();
    }
    while (!(MinDate < Date));
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
                NewItem->TimelineEra = Civ->TimelineEra;
                ACupActor* Cup = Cast<ACupActor>(NewItem);
                if (Cup)
                {
                    Cup->LinkedCivilian = Civ;
                }
            }
        }
    }
}

FString AProceduralLevelGenerator::GenerateUniqueName(const TArray<FString>& FirstNames, const TArray<FString>& Surnames, TSet<FString>& UsedNames) const
{
    FString Name;
    do {
        int32 FirstIdx = FMath::RandRange(0, FirstNames.Num() - 1);
        int32 LastIdx = FMath::RandRange(0, Surnames.Num() - 1);
        Name = FirstNames[FirstIdx] + TEXT(" ") + Surnames[LastIdx];
    } while (UsedNames.Contains(Name));
    UsedNames.Add(Name);
    return Name;
}

void AProceduralLevelGenerator::HandleEraSpawns(
    ETimelineEra Era,
    TArray<ACivilianCharacter*>& OutSpawnedCivilians,
    TArray<ADeskActor*>& OutDesks
)
{
    // ---- CIVILIAN SPAWN ----
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACivilianSpawnPoint::StaticClass(), SpawnPoints);

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

    OutSpawnedCivilians.Empty();

    for (AActor* Actor : SpawnPoints)
    {
        ACivilianSpawnPoint* SpawnPoint = Cast<ACivilianSpawnPoint>(Actor);
        if (SpawnPoint && SpawnPoint->TimelineEra == Era)
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
                Civilian->TimelineEra = Era;
                Civilian->CivilianName = GenerateUniqueName(FirstNames, Surnames, UsedNames);
                OutSpawnedCivilians.Add(Civilian);
            }
        }
    }

    // ---- DESK SETUP ----
    TArray<AActor*> FoundDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeskActor::StaticClass(), FoundDesks);

    OutDesks.Empty();
    for (AActor* Actor : FoundDesks)
    {
        ADeskActor* Desk = Cast<ADeskActor>(Actor);
        if (Desk && Desk->TimelineEra == Era)
        {
            OutDesks.Add(Desk);
        }
    }

    // ---- PAIR CIVILIANS TO DESKS ----
    int32 PairCount = FMath::Min(OutSpawnedCivilians.Num(), OutDesks.Num());
    for (int32 i = 0; i < PairCount; ++i)
    {
        ACivilianCharacter* Civ = OutSpawnedCivilians[i];
        ADeskActor* Desk = OutDesks[i];
        Civ->AssignedDesk = Desk;
        Desk->SetStaffName(Civ->CivilianName);
    }
}

void AProceduralLevelGenerator::HandlePastSpawns()
{
    TArray<AActor*> FoundWireManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWirePuzzleManager::StaticClass(), FoundWireManagers);

    if (FoundWireManagers.Num() > 0)
    {
        // Gather device order
        auto* WireManager = Cast<AWirePuzzleManager>(FoundWireManagers[0]);
        if (WireManager && WireManager->PuzzleDevices.Num() > 0 && WireManager->TimelineEra== ETimelineEra::Past)
        {
            TArray<int32> Order;
            Order.SetNum(WireManager->PuzzleDevices.Num());
            for (int32 i = 0; i < WireManager->PuzzleDevices.Num(); ++i)
                Order[i] = i;
            for (int32 i = Order.Num() - 1; i > 0; --i)
                Order.Swap(i, FMath::RandRange(0, i));

            TArray<FString> OrderStrings;
            for (int32 Num : Order) OrderStrings.Add(FString::FromInt(Num));
            PastWireDeviceOrderString = FString::Join(OrderStrings, TEXT(","));

            // Gather correct colors
            TArray<FString> ColorStrs;
            for (int32 i = 0; i < WireManager->PuzzleDevices.Num(); ++i)
            {
                auto* Device = WireManager->PuzzleDevices[Order[i]];
                if (Device && Device->WireActors.Num() > 0)
                {
                    int32 WireIdx = FMath::RandRange(0, Device->WireActors.Num() - 1);
                    auto WireColor = Device->WireActors[WireIdx]->WireColor;
                    ColorStrs.Add(UEnum::GetValueAsString(WireColor));
                }
                else
                {
                    ColorStrs.Add(UEnum::GetValueAsString(EWireColor::Red));
                }
            }
            PastWireCorrectColorString = FString::Join(ColorStrs, TEXT(","));
        }
    }


    TArray<AActor*> Managers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALeverManager::StaticClass(), Managers);

    if (Managers.Num() > 0)
    {
        auto* Manager = Cast<ALeverManager>(Managers[0]);
        if (Manager && Manager->PuzzleLevers.Num() > 0 && Manager->TimelineEra== ETimelineEra::Past)
        {
            TArray<int32> Order;
            Order.SetNum(Manager->PuzzleLevers.Num());
            for (int32 i = 0; i < Manager->PuzzleLevers.Num(); ++i)
                Order[i] = i;
            for (int32 i = Order.Num() - 1; i > 0; --i)
                Order.Swap(i, FMath::RandRange(0, i));

            // Convert Order to comma-separated string
            TArray<FString> OrderStrings;
            for (int32 Num : Order)
            {
                OrderStrings.Add(FString::FromInt(Num));
            }
            PastLeverOrderString = FString::Join(OrderStrings, TEXT(","));

            // Setup lever puzzle with randomized order
            Manager->SetupPuzzle(Order);
        }
    }
    TArray<ACivilianCharacter*> SpawnedCivilians;
    TArray<ADeskActor*> PastDesks;
    HandleEraSpawns(ETimelineEra::Past, SpawnedCivilians, PastDesks);

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
    TArray<ACivilianCharacter*> SpawnedCivilians;
    TArray<ADeskActor*> FutureDesks;
    HandleEraSpawns(ETimelineEra::Future, SpawnedCivilians, FutureDesks);

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
    DOREPLIFETIME(AProceduralLevelGenerator, PastLeverOrderString);
}