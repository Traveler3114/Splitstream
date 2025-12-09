#include "ProceduralLevelGenerator.h"
#include "Actors/PointActors/CivilianSpawnPoint.h"
#include "Characters/CivilianCharacter.h"
#include "Characters/GuardCharacter.h"
#include "Actors/LockerActor.h"
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
#include "Actors/Wire/WirePuzzleManager.h"
#include "Actors/Wire/WireDeviceActor.h"
#include "Actors/Wire/WireActor.h"
#include "Actors/DisablingDevice/DevicesManagerActor.h"
#include "Actors/DisablingDevice/DisablingDeviceActor.h"
#include "Components/ArrowComponent.h"

// ============================================================
// Constants
// ============================================================
namespace ProceduralGenConstants
{
    constexpr int32 DefaultCodeLength = 4;
    constexpr int32 NumWireDevices = 3;
    constexpr int32 NumLevers = 3;
    constexpr int32 NumDisablingDevices = 3;

    // Name pools for procedurally generated NPCs
    static const TArray<FString> CivilianFirstNames = {
        TEXT("John"), TEXT("Laura"), TEXT("Michael"), TEXT("Sarah"), TEXT("David"),
        TEXT("Emily"), TEXT("James"), TEXT("Olivia"), TEXT("Daniel"), TEXT("Sophia"),
        TEXT("Chris"), TEXT("Jessica"), TEXT("Ethan"), TEXT("Anna"), TEXT("Ryan"),
        TEXT("Megan"), TEXT("Luke"), TEXT("Chloe"), TEXT("Nathan"), TEXT("Grace")
    };
    
    static const TArray<FString> CivilianSurnames = {
        TEXT("Smith"), TEXT("Morgan"), TEXT("Davis"), TEXT("Lee"), TEXT("Clark"),
        TEXT("Turner"), TEXT("Harris"), TEXT("Bennett"), TEXT("Evans"), TEXT("Carter"),
        TEXT("Adams"), TEXT("Wright"), TEXT("Green"), TEXT("Hill"), TEXT("Cook"),
        TEXT("Lewis"), TEXT("Roberts"), TEXT("Walker"), TEXT("Young"), TEXT("King")
    };

    static const TArray<FString> GuardFirstNames = {
        TEXT("Alex"), TEXT("Blake"), TEXT("Morgan"), TEXT("Pat"), TEXT("Jordan"),
        TEXT("Sam"), TEXT("Quinn"), TEXT("Taylor"), TEXT("Casey"), TEXT("Robin"),
        TEXT("Max"), TEXT("Jesse"), TEXT("Corey"), TEXT("Jamie"), TEXT("Cameron"),
        TEXT("Lee"), TEXT("Drew"), TEXT("Avery"), TEXT("Riley"), TEXT("Devon")
    };
    
    static const TArray<FString> GuardSurnames = {
        TEXT("Stone"), TEXT("Parker"), TEXT("Mills"), TEXT("Ford"), TEXT("King"),
        TEXT("Hunter"), TEXT("Knight"), TEXT("Brooks"), TEXT("Cole"), TEXT("West"),
        TEXT("Reed"), TEXT("Ray"), TEXT("Grant"), TEXT("Chase"), TEXT("Boone"),
        TEXT("Frost"), TEXT("Wells"), TEXT("Rhodes"), TEXT("Cross"), TEXT("Bishop")
    };
}

// ============================================================
// Template Helper Functions
// ============================================================
template<typename TSpawnActor, typename TManagerActor>
void SpawnActorsOnRandomPointsAndAddToManager(
    UWorld* World,
    TSubclassOf<TSpawnActor> SpawnActorBPClass,
    const FName& PointTag,
    ETimelineEra Era,
    TSubclassOf<TManagerActor> ManagerClass,
    const FName& ManagerTag,
    int32 NumToSpawn,
    TFunction<void(TManagerActor*, TSpawnActor*)> AddToManagerArray
)
{
    // 1. Find spawn points matching tag and era
    TArray<AActor*> FoundPoints;
    UGameplayStatics::GetAllActorsOfClass(World, ARandomPointActor::StaticClass(), FoundPoints);

    TArray<ARandomPointActor*> ValidPoints;
    for (AActor* Actor : FoundPoints)
    {
        ARandomPointActor* Point = Cast<ARandomPointActor>(Actor);
        if (Point
            && Point->TimelineEra == Era
            && Point->Tags.Contains(PointTag))
        {
            ValidPoints.Add(Point);
        }
    }

    // 2. Shuffle points randomly
    for (int32 i = ValidPoints.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        ValidPoints.Swap(i, j);
    }

    int32 SpawnCount = FMath::Min(NumToSpawn, ValidPoints.Num());

    TArray<TSpawnActor*> SpawnedActors;
    for (int32 i = 0; i < SpawnCount; ++i)
    {
        ARandomPointActor* Point = ValidPoints[i];
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        UArrowComponent* PointArrow = Point->FindComponentByClass<UArrowComponent>();
        FRotator NodeBaseRotation = PointArrow ? PointArrow->GetComponentRotation() : Point->GetActorRotation();
        FVector SpawnLocation = Point->GetActorLocation();

        TSpawnActor* SpawnedActor = World->SpawnActor<TSpawnActor>(SpawnActorBPClass, SpawnLocation, NodeBaseRotation, SpawnParams);

        if (SpawnedActor)
        {
            SpawnedActor->TimelineEra = Era;

            // Assign the location name from Tags[1]
            if (Point->Tags.Num() > 1)
            {
                SpawnedActor->SpawnLocationName = Point->Tags[1].ToString();
            }

            // Alignment fix (as you had)
            if (PointArrow)
            {
                UArrowComponent* SpawnedArrow = SpawnedActor->FindComponentByClass<UArrowComponent>();
                if (SpawnedArrow)
                {
                    const FVector DesiredForward = PointArrow->GetForwardVector();
                    const FVector DesiredUp = PointArrow->GetUpVector();
                    FRotator DesiredRotator = FRotationMatrix::MakeFromXZ(DesiredForward, DesiredUp).Rotator();
                    FRotator ArrowCompRelRot = SpawnedArrow->GetRelativeTransform().GetRotation().Rotator();
                    FRotator FinalActorRotation = DesiredRotator - ArrowCompRelRot;
                    SpawnedActor->SetActorRotation(FinalActorRotation);
                }
                else
                {
                    SpawnedActor->SetActorRotation(PointArrow->GetComponentRotation());
                }
            }
            SpawnedActors.Add(SpawnedActor);
        }
    }

    // 5. Find manager with given tag
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(World, ManagerClass, FoundManagers);

    TManagerActor* Manager = nullptr;
    for (AActor* Actor : FoundManagers)
    {
        TManagerActor* Candidate = Cast<TManagerActor>(Actor);
        if (Candidate && Candidate->Tags.Contains(ManagerTag))
        {
            Manager = Candidate;
            break;
        }
    }

    // 6. Add spawned actors to manager
    if (Manager)
    {
        for (TSpawnActor* SpawnedActor : SpawnedActors)
        {
            AddToManagerArray(Manager, SpawnedActor);
        }
    }
}
// ============================================================
// Constructor and Lifecycle
// ============================================================

AProceduralLevelGenerator::AProceduralLevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AProceduralLevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    // Only server handles procedural generation
    if (HasAuthority())
    {
        HandlePastSpawns();
    }
}

// ============================================================
// Utility Function Implementations
// ============================================================

FString AProceduralLevelGenerator::GenerateUniqueName(const TArray<FString>& FirstNames, const TArray<FString>& Surnames, TSet<FString>& UsedNames) const
{
    FString Name;
    do
    {
        int32 FirstIdx = FMath::RandRange(0, FirstNames.Num() - 1);
        int32 LastIdx = FMath::RandRange(0, Surnames.Num() - 1);
        Name = FirstNames[FirstIdx] + TEXT(" ") + Surnames[LastIdx];
    }
    while (UsedNames.Contains(Name));
    
    UsedNames.Add(Name);
    return Name;
}

FString AProceduralLevelGenerator::GenerateRandomCode(int32 Length) const
{
    static const FString Digits = TEXT("0123456789");
    FString Code;
    Code.Reserve(Length);
    
    for (int32 i = 0; i < Length; ++i)
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
    
    // Calculate days in month, accounting for leap years
    int32 DaysInMonth = 31;
    if (Date.Month == 2)
    {
        bool bIsLeapYear = (Date.Year % 4 == 0 && Date.Year % 100 != 0) || (Date.Year % 400 == 0);
        DaysInMonth = bIsLeapYear ? 29 : 28;
    }
    else if (Date.Month == 4 || Date.Month == 6 || Date.Month == 9 || Date.Month == 11)
    {
        DaysInMonth = 30;
    }
    
    Date.Day = FMath::RandRange(1, DaysInMonth);
    return Date;
}

FRandomDate AProceduralLevelGenerator::GeneratePastDate() const
{
    return GenerateRandomDate();
}

FRandomDate AProceduralLevelGenerator::GenerateFutureDate(const FRandomDate& MinDate) const
{
    FRandomDate Date;
    
    // Keep generating until we get a date after MinDate
    do
    {
        Date = GenerateRandomDate();
    }
    while (!(MinDate < Date));
    
    return Date;
}

void AProceduralLevelGenerator::SpawnCivilianDeskItems(const TArray<ACivilianCharacter*>& Civilians, TSubclassOf<ASearchableActor> ItemClass)
{
    for (ACivilianCharacter* Civilian : Civilians)
    {
        if (!Civilian || !Civilian->AssignedDesk)
        {
            continue;
        }

        for (ASearchableItemSpawnPoint* SpawnPoint : Civilian->AssignedDesk->ItemSpawnPoints)
        {
            if (!SpawnPoint)
            {
                continue;
            }

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            ASearchableActor* NewItem = GetWorld()->SpawnActor<ASearchableActor>(
                ItemClass, 
                SpawnPoint->GetActorLocation(), 
                SpawnPoint->GetActorRotation(), 
                SpawnParams
            );

            if (NewItem)
            {
                NewItem->TimelineEra = Civilian->TimelineEra;
                
                // Link cup actors to their civilian owner
                ACupActor* Cup = Cast<ACupActor>(NewItem);
                if (Cup)
                {
                    Cup->LinkedCivilian = Civilian;
                }
            }
        }
    }
}

// ============================================================
// Puzzle Setup Implementations
// ============================================================

void AProceduralLevelGenerator::SetupWirePuzzle()
{
    // Spawn wire devices at random points
    SpawnActorsOnRandomPointsAndAddToManager<AWireDeviceActor, AWirePuzzleManager>(
        GetWorld(),
        WireDeviceBPClass,
        "WireDevice",
        ETimelineEra::Past,
        AWirePuzzleManager::StaticClass(),
        "BarsTarget",
        ProceduralGenConstants::NumWireDevices,
        [](AWirePuzzleManager* Manager, AWireDeviceActor* Device) { Manager->PuzzleDevices.Add(Device); }
    );

    // Find wire puzzle manager and setup the puzzle
    TArray<AActor*> FoundWireManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWirePuzzleManager::StaticClass(), FoundWireManagers);

    if (FoundWireManagers.Num() == 0)
    {
        return;
    }

    AWirePuzzleManager* WireManager = Cast<AWirePuzzleManager>(FoundWireManagers[0]);
    if (!WireManager || WireManager->PuzzleDevices.Num() == 0 || WireManager->TimelineEra != ETimelineEra::Past)
    {
        return;
    }

    // Generate randomized device order
    TArray<int32> DeviceOrder;
    DeviceOrder.SetNum(WireManager->PuzzleDevices.Num());
    for (int32 i = 0; i < WireManager->PuzzleDevices.Num(); ++i)
    {
        DeviceOrder[i] = i;
    }
    
    // Shuffle the order
    for (int32 i = DeviceOrder.Num() - 1; i > 0; --i)
    {
        DeviceOrder.Swap(i, FMath::RandRange(0, i));
    }

    // Build sequence array of {location, color}
    PastWireDeviceSequence.Empty();
    for (int32 OrderIndex : DeviceOrder)
    {
        AWireDeviceActor* Device = WireManager->PuzzleDevices[OrderIndex];
        FString Location = Device ? Device->SpawnLocationName : TEXT("Unknown");

        EWireColor Color = EWireColor::Red;
        if (Device && Device->WireActors.Num() > 0)
        {
            int32 WireIdx = FMath::RandRange(0, Device->WireActors.Num() - 1);
            Color = Device->WireActors[WireIdx]->WireColor;
        }

        FWireSequenceStep Step;
        Step.DeviceLocation = Location;
        Step.WireColor = Color;
        PastWireDeviceSequence.Add(Step);
    }

    WireManager->SetupPuzzle();
}

void AProceduralLevelGenerator::SetupLeverPuzzle()
{
    // Spawn levers at random points
    SpawnActorsOnRandomPointsAndAddToManager<ALeverActor, ALeverManager>(
        GetWorld(),
        LeverBPClass,
        "Lever",
        ETimelineEra::Past,
        ALeverManager::StaticClass(),
        "LaserManagerTarget",
        ProceduralGenConstants::NumLevers,
        [](ALeverManager* Manager, ALeverActor* Lever) { Manager->PuzzleLevers.Add(Lever); }
    );

    // Find lever manager and setup the puzzle
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALeverManager::StaticClass(), FoundManagers);

    if (FoundManagers.Num() == 0)
    {
        return;
    }

    ALeverManager* Manager = Cast<ALeverManager>(FoundManagers[0]);
    if (!Manager || Manager->PuzzleLevers.Num() == 0 || Manager->TimelineEra != ETimelineEra::Past)
    {
        return;
    }

    // Generate randomized lever order
    TArray<int32> LeverOrder;
    LeverOrder.SetNum(Manager->PuzzleLevers.Num());
    for (int32 i = 0; i < Manager->PuzzleLevers.Num(); ++i)
    {
        LeverOrder[i] = i;
    }
    
    // Shuffle the order
    for (int32 i = LeverOrder.Num() - 1; i > 0; --i)
    {
        LeverOrder.Swap(i, FMath::RandRange(0, i));
    }

    // Build lever order string using location names
    TArray<FString> OrderLocationNames;
    for (int32 LeverIndex : LeverOrder)
    {
        if (Manager->PuzzleLevers.IsValidIndex(LeverIndex) && Manager->PuzzleLevers[LeverIndex])
        {
            OrderLocationNames.Add(Manager->PuzzleLevers[LeverIndex]->SpawnLocationName);
        }
        else
        {
            OrderLocationNames.Add(TEXT("Unknown"));
        }
    }
    PastLeverOrderString = FString::Join(OrderLocationNames, TEXT(","));

    // Setup lever puzzle with randomized order
    Manager->SetupPuzzle(LeverOrder);
}

void AProceduralLevelGenerator::SetupDisablingDevices()
{
    SpawnActorsOnRandomPointsAndAddToManager<ADisablingDeviceActor, ADevicesManagerActor>(
        GetWorld(),
        DisablingDeviceBPClass,
        "DisablingDevice",
        ETimelineEra::Past,
        ADevicesManagerActor::StaticClass(),
        "MetalDetectorTarget",
        ProceduralGenConstants::NumDisablingDevices,
        [](ADevicesManagerActor* Manager, ADisablingDeviceActor* Device) { Manager->Devices.Add(Device); }
    );
}

void AProceduralLevelGenerator::SetupKeypadAndComputerCodes(const TArray<ADeskActor*>& Desks)
{
    // Find desks with computers
    TArray<ADeskActor*> DesksWithComputer;
    for (ADeskActor* Desk : Desks)
    {
        if (Desk && Desk->DeskComputer)
        {
            DesksWithComputer.Add(Desk);
        }
    }

    // Find keypads in the level
    TArray<AActor*> FoundKeypads;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AKeypadScanner::StaticClass(), FoundKeypads);

    if (DesksWithComputer.Num() == 0 || FoundKeypads.Num() == 0)
    {
        return;
    }

    // Generate a random keypad code
    FString KeypadCode = GenerateRandomCode(ProceduralGenConstants::DefaultCodeLength);

    // Randomly select one desk to have the code
    int32 SelectedDeskIndex = FMath::RandRange(0, DesksWithComputer.Num() - 1);
    
    // Setup computers: only the selected one gets the code
    for (int32 i = 0; i < DesksWithComputer.Num(); ++i)
    {
        if (DesksWithComputer[i]->DeskComputer)
        {
            if (i == SelectedDeskIndex)
            {
                DesksWithComputer[i]->DeskComputer->SetupComputer(TEXT(""), KeypadCode);
            }
            else
            {
                DesksWithComputer[i]->DeskComputer->SetupComputer(TEXT(""), TEXT(""));
            }
        }
    }

    // Set the code on all past-era keypads
    for (AActor* Actor : FoundKeypads)
    {
        AKeypadScanner* Keypad = Cast<AKeypadScanner>(Actor);
        if (Keypad && Keypad->TimelineEra == ETimelineEra::Past)
        {
            Keypad->SetCorrectCode(KeypadCode);
        }
    }
}

void AProceduralLevelGenerator::SpawnNewspaper(ETimelineEra Era, const FRandomDate& Date)
{
    // Find all random point actors
    TArray<AActor*> AllRandomPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARandomPointActor::StaticClass(), AllRandomPoints);

    // Filter for newspaper spawn points in the specified era
    TArray<ARandomPointActor*> NewspaperPoints;
    for (AActor* Actor : AllRandomPoints)
    {
        ARandomPointActor* Point = Cast<ARandomPointActor>(Actor);
        if (Point 
            && Point->TimelineEra == Era 
            && Point->Tags.Num() > 0 
            && Point->Tags[0] == "Newspaper")
        {
            NewspaperPoints.Add(Point);
        }
    }

    if (NewspaperPoints.Num() == 0)
    {
        return;
    }

    // Randomly select a spawn point
    int32 SpawnIndex = FMath::RandRange(0, NewspaperPoints.Num() - 1);
    ARandomPointActor* SelectedPoint = NewspaperPoints[SpawnIndex];

    // Spawn the newspaper
    FActorSpawnParameters SpawnParams;
    ANewspaperActor* Newspaper = GetWorld()->SpawnActor<ANewspaperActor>(
        NewspaperBPClass, 
        SelectedPoint->GetActorLocation(), 
        SelectedPoint->GetActorRotation(), 
        SpawnParams
    );

    if (Newspaper)
    {
        Newspaper->TimelineEra = Era;
        FString DateStr = FString::Printf(TEXT("%d-%02d-%02d"), Date.Year, Date.Month, Date.Day);
        Newspaper->SetDateText(DateStr);
    }
}

void AProceduralLevelGenerator::SpawnSecurityDocument(const TArray<AGuardCharacter*>& Guards)
{
    // Find the security chief and their locker
    AGuardCharacter* SecurityChief = nullptr;
    ALockerActor* SecurityChiefLocker = nullptr;
    
    for (AGuardCharacter* Guard : Guards)
    {
        if (Guard && Guard->bIsSecurityChief)
        {
            SecurityChief = Guard;
            SecurityChiefLocker = Guard->AssignedLocker;
            break;
        }
    }

    if (!SecurityChiefLocker || !SecurityDocumentBPClass)
    {
        return;
    }

    // Find the child RandomPointActor within the locker
    ARandomPointActor* LockerPoint = nullptr;
    TArray<UChildActorComponent*> ChildComponents;
    SecurityChiefLocker->GetComponents(ChildComponents);
    
    for (UChildActorComponent* ChildComp : ChildComponents)
    {
        ARandomPointActor* Point = Cast<ARandomPointActor>(ChildComp->GetChildActor());
        if (Point)
        {
            LockerPoint = Point;
            break;
        }
    }

    if (!LockerPoint)
    {
        return;
    }

    // Get arrow component for precise rotation
    UArrowComponent* PointArrow = LockerPoint->FindComponentByClass<UArrowComponent>();
    FVector SpawnLocation = LockerPoint->GetActorLocation();
    FRotator SpawnRotation = PointArrow ? PointArrow->GetComponentRotation() : LockerPoint->GetActorRotation();

    // Spawn the security document
    FActorSpawnParameters DocSpawnParams;
    DocSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    DocSpawnParams.Owner = SecurityChiefLocker;

    AActor* DocActor = GetWorld()->SpawnActor<AActor>(
        SecurityDocumentBPClass,
        SpawnLocation,
        SpawnRotation,
        DocSpawnParams
    );

    // Align the document's arrow with the spawn point's arrow
    if (DocActor && PointArrow)
    {
        UArrowComponent* DocArrow = DocActor->FindComponentByClass<UArrowComponent>();
        if (DocArrow)
        {
            const FVector DesiredForward = PointArrow->GetForwardVector();
            const FVector DesiredUp = PointArrow->GetUpVector();
            FRotator DesiredRotator = FRotationMatrix::MakeFromXZ(DesiredForward, DesiredUp).Rotator();
            FRotator ArrowCompRelRot = DocArrow->GetRelativeTransform().GetRotation().Rotator();
            FRotator FinalActorRotation = DesiredRotator - ArrowCompRelRot;
            DocActor->SetActorRotation(FinalActorRotation);
        }
        else
        {
            DocActor->SetActorRotation(PointArrow->GetComponentRotation());
        }
    }
}

// ============================================================
// Era-Based Spawning Implementation
// ============================================================

void AProceduralLevelGenerator::SpawnCiviliansForEra(ETimelineEra Era, TArray<ACivilianCharacter*>& OutCivilians)
{
    OutCivilians.Empty();

    // Find all civilian spawn points for this era
    TArray<AActor*> AllSpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACivilianSpawnPoint::StaticClass(), AllSpawnPoints);

    TSet<FString> UsedNames;

    for (AActor* Actor : AllSpawnPoints)
    {
        ACivilianSpawnPoint* SpawnPoint = Cast<ACivilianSpawnPoint>(Actor);
        if (SpawnPoint && SpawnPoint->TimelineEra == Era)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            ACivilianCharacter* Civilian = GetWorld()->SpawnActor<ACivilianCharacter>(
                CivilianBPClass,
                SpawnPoint->GetActorLocation(),
                SpawnPoint->GetActorRotation(),
                SpawnParams
            );

            if (Civilian)
            {
                Civilian->TimelineEra = Era;
                Civilian->CivilianName = GenerateUniqueName(
                    ProceduralGenConstants::CivilianFirstNames, 
                    ProceduralGenConstants::CivilianSurnames, 
                    UsedNames
                );
                OutCivilians.Add(Civilian);
            }
        }
    }
}

void AProceduralLevelGenerator::GatherDesksForEra(ETimelineEra Era, TArray<ADeskActor*>& OutDesks)
{
    OutDesks.Empty();

    TArray<AActor*> AllDesks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeskActor::StaticClass(), AllDesks);

    for (AActor* Actor : AllDesks)
    {
        ADeskActor* Desk = Cast<ADeskActor>(Actor);
        if (Desk && Desk->TimelineEra == Era)
        {
            OutDesks.Add(Desk);
        }
    }
}

void AProceduralLevelGenerator::AssignCiviliansToDesks(const TArray<ACivilianCharacter*>& Civilians, const TArray<ADeskActor*>& Desks)
{
    int32 AssignmentCount = FMath::Min(Civilians.Num(), Desks.Num());
    
    for (int32 i = 0; i < AssignmentCount; ++i)
    {
        ACivilianCharacter* Civilian = Civilians[i];
        ADeskActor* Desk = Desks[i];
        
        Civilian->AssignedDesk = Desk;
        Desk->SetStaffName(Civilian->CivilianName);
    }
}

void AProceduralLevelGenerator::SetupGuardsAndLockers(ETimelineEra Era, TArray<AGuardCharacter*>& OutGuards, TArray<ALockerActor*>& OutLockers)
{
    OutGuards.Empty();
    OutLockers.Empty();

    // Gather all guards and lockers for this era
    TArray<AActor*> AllGuards;
    TArray<AActor*> AllLockers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGuardCharacter::StaticClass(), AllGuards);
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALockerActor::StaticClass(), AllLockers);

    for (AActor* Actor : AllGuards)
    {
        AGuardCharacter* Guard = Cast<AGuardCharacter>(Actor);
        if (Guard && Guard->TimelineEra == Era)
        {
            OutGuards.Add(Guard);
        }
    }

    for (AActor* Actor : AllLockers)
    {
        ALockerActor* Locker = Cast<ALockerActor>(Actor);
        if (Locker && Locker->TimelineEra == Era)
        {
            OutLockers.Add(Locker);
        }
    }

    // Shuffle guards and lockers for random pairing
    for (int32 i = OutGuards.Num() - 1; i > 0; --i)
    {
        OutGuards.Swap(i, FMath::RandRange(0, i));
    }
    for (int32 i = OutLockers.Num() - 1; i > 0; --i)
    {
        OutLockers.Swap(i, FMath::RandRange(0, i));
    }

    // Assign guards to lockers with unique names
    TSet<FString> UsedGuardNames;
    int32 PairCount = FMath::Min(OutGuards.Num(), OutLockers.Num());
    
    for (int32 i = 0; i < PairCount; ++i)
    {
        AGuardCharacter* Guard = OutGuards[i];
        ALockerActor* Locker = OutLockers[i];
        
        Guard->GuardName = GenerateUniqueName(
            ProceduralGenConstants::GuardFirstNames, 
            ProceduralGenConstants::GuardSurnames, 
            UsedGuardNames
        );
        Guard->AssignedLocker = Locker;
        Locker->SetStaffName(Guard->GuardName);
    }
}

void AProceduralLevelGenerator::HandleEraSpawns(
    ETimelineEra Era,
    TArray<ACivilianCharacter*>& OutSpawnedCivilians,
    TArray<ADeskActor*>& OutDesks,
    TArray<AGuardCharacter*>& OutEraGuards,
    TArray<ALockerActor*>& OutEraLockers
)
{
    // Spawn civilians and generate their names
    SpawnCiviliansForEra(Era, OutSpawnedCivilians);

    // Gather desks for this era
    GatherDesksForEra(Era, OutDesks);

    // Assign civilians to desks
    AssignCiviliansToDesks(OutSpawnedCivilians, OutDesks);

    // Setup guards with lockers
    SetupGuardsAndLockers(Era, OutEraGuards, OutEraLockers);
}

// ============================================================
// Main Spawning Orchestration
// ============================================================

void AProceduralLevelGenerator::HandlePastSpawns()
{
    // Setup all puzzle systems for the past era
    SetupWirePuzzle();
    SetupLeverPuzzle();
    SetupDisablingDevices();

    // Spawn and configure all NPCs and environment
    TArray<ACivilianCharacter*> PastCivilians;
    TArray<ADeskActor*> PastDesks;
    TArray<AGuardCharacter*> PastGuards;
    TArray<ALockerActor*> PastLockers;
    HandleEraSpawns(ETimelineEra::Past, PastCivilians, PastDesks, PastGuards, PastLockers);

    // Setup security systems
    SetupKeypadAndComputerCodes(PastDesks);

    // Generate date and spawn environmental objects
    PastDate = GeneratePastDate();
    SpawnNewspaper(ETimelineEra::Past, PastDate);
    SpawnCivilianDeskItems(PastCivilians, SearchableItemBPClass);
    SpawnSecurityDocument(PastGuards);

    // After past is setup, configure future era
    HandleFutureSpawns();
}

void AProceduralLevelGenerator::HandleFutureSpawns()
{
    // Spawn and configure all NPCs and environment for future era
    TArray<ACivilianCharacter*> FutureCivilians;
    TArray<ADeskActor*> FutureDesks;
    TArray<AGuardCharacter*> FutureGuards;
    TArray<ALockerActor*> FutureLockers;
    HandleEraSpawns(ETimelineEra::Future, FutureCivilians, FutureDesks, FutureGuards, FutureLockers);

    // Select a random civilian as the target for code generators
    ACivilianCharacter* TargetCivilian = nullptr;
    if (FutureCivilians.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, FutureCivilians.Num() - 1);
        TargetCivilian = FutureCivilians[RandomIndex];
    }

    // Configure all code generators to target the selected civilian
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

    // Generate date and spawn environmental objects
    FutureDate = GenerateFutureDate(PastDate);
    SpawnNewspaper(ETimelineEra::Future, FutureDate);
    SpawnCivilianDeskItems(FutureCivilians, SearchableItemBPClass);
}

// ============================================================
// Networking
// ============================================================

void AProceduralLevelGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AProceduralLevelGenerator, PastDate);
    DOREPLIFETIME(AProceduralLevelGenerator, FutureDate);
    DOREPLIFETIME(AProceduralLevelGenerator, PastLeverOrderString);
    DOREPLIFETIME(AProceduralLevelGenerator, PastWireDeviceSequence);
}