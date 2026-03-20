#include "ArchiveComputer.h"
#include "Actors/ProceduralLevelGenerator.h"
#include "Computer.h"
#include "Actors/DeskActor.h"
#include "Actors/CodeGenerator.h"
#include "AI/Characters/CivilianCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Calendar/CalendarWidget.h"
#include "EngineUtils.h" // for TActorIterator

// Helper function: filter actors by TimelineEra
template <typename T>
void GetActorsByTimelineEra(UWorld* World, ETimelineEra TimelineEra, TArray<T*>& OutActors)
{
    OutActors.Empty();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, T::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        T* TypedActor = Cast<T>(Actor);
        if (TypedActor && TypedActor->TimelineEra == TimelineEra)
        {
            OutActors.Add(TypedActor);
        }
    }
}

AArchiveComputer::AArchiveComputer()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
    ComputerMesh->SetupAttachment(DefaultSceneRoot);
}

void AArchiveComputer::BeginPlay()
{
    Super::BeginPlay();

    GeneratorRef = Cast<AProceduralLevelGenerator>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AProceduralLevelGenerator::StaticClass())
    );

    // Cache all PAST computer actors once
    CodeComputers.Empty();
    GetActorsByTimelineEra<AComputer>(GetWorld(), ETimelineEra::Past, CodeComputers);

    // Build map: Computer -> Civilian (via Desk), only for Past
    ComputerToCivilianMap.Empty();
    for (TActorIterator<ACivilianCharacter> CivItr(GetWorld()); CivItr; ++CivItr)
    {
        ACivilianCharacter* Civ = *CivItr;
        if (Civ && Civ->AssignedDesk && Civ->AssignedDesk->DeskComputer && Civ->AssignedDesk->DeskComputer->TimelineEra == ETimelineEra::Past)
        {
            ComputerToCivilianMap.Add(Civ->AssignedDesk->DeskComputer, Civ);
        }
    }

    // Build map: Computer -> Desk (for StaffName lookup)
    ComputerToDeskMap.Empty();
    for (TActorIterator<ADeskActor> DeskItr(GetWorld()); DeskItr; ++DeskItr)
    {
        ADeskActor* Desk = *DeskItr;
        if (Desk && Desk->DeskComputer)
        {
            ComputerToDeskMap.Add(Desk->DeskComputer, Desk);
        }
    }
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn) return;
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) return;

    TArray<FCalendarDateRecord> CalendarDateRecords;

    // PAST
    if (GeneratorRef)
    {
        FCalendarDateRecord PastRec;
        PastRec.Year = GeneratorRef->PastDate.Year;
        PastRec.Month = GeneratorRef->PastDate.Month;
        PastRec.Day = GeneratorRef->PastDate.Day;
        PastRec.LeverOrderString = GeneratorRef->PastLeverOrderString;
        PastRec.WireDeviceOrder = GeneratorRef->PastWireDeviceSequence;

        for (AComputer* Comp : CodeComputers)
        {
            if (Comp && !Comp->StoredCode.IsEmpty())
            {
                FCivilianCalendarEntry Entry;
                if (ACivilianCharacter* Civ = ComputerToCivilianMap.FindRef(Comp))
                {
                    Entry.Name = Civ->CivilianName;
                    Entry.Portrait = Civ->PortraitTexture;
                }
                else if (ADeskActor* Desk = ComputerToDeskMap.FindRef(Comp))
                {
                    Entry.Name = Desk->StaffName;
                    Entry.Portrait = nullptr;
                }
                else
                {
                    Entry.Name = TEXT("Unknown Staff");
                    Entry.Portrait = nullptr;
                }
                PastRec.Civilians.Add(Entry);
            }
        }
        CalendarDateRecords.Add(PastRec);

        // FUTURE
        //FCalendarDateRecord FutureRec;
        //FutureRec.Year = GeneratorRef->FutureDate.Year;
        //FutureRec.Month = GeneratorRef->FutureDate.Month;
        //FutureRec.Day = GeneratorRef->FutureDate.Day;

        //TArray<AActor*> FoundCodeGenerators;
        //UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACodeGenerator::StaticClass(), FoundCodeGenerators);

        //for (AActor* Actor : FoundCodeGenerators)
        //{
        //    ACodeGenerator* CodeGen = Cast<ACodeGenerator>(Actor);
        //    if (CodeGen && CodeGen->TimelineEra == ETimelineEra::Future && CodeGen->TargetCivilian)
        //    {
        //        FCivilianCalendarEntry Entry;
        //        Entry.Name = CodeGen->TargetCivilian->CivilianName;
        //        Entry.Portrait = CodeGen->TargetCivilian->PortraitTexture;
        //        FutureRec.Civilians.Add(Entry);
        //        break;
        //    }
        //}
        //if (FutureRec.Civilians.Num() > 0)
        //    CalendarDateRecords.Add(FutureRec);
    }

    // **Only the server sends the RPC. This works for both listen server and remote clients!**
    if (HasAuthority())
    {
        if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
        {
            MyPC->ClientShowCalendarWidget(CalendarDateRecords);
        }
    }
}
void AArchiveComputer::SetHighlighted_Implementation(bool bHighlight)
{
    if (ComputerMesh)
    {
        ComputerMesh->SetRenderCustomDepth(bHighlight);
        ComputerMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}