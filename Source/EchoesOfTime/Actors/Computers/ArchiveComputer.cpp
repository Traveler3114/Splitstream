#include "ArchiveComputer.h"
#include "ProceduralLevelGenerator.h"
#include "Computer.h"
#include "Characters/CivilianCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
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

    // Build map: Computer -> Civilian (for O(1) lookup), only for Past
    ComputerToCivilianMap.Empty();
    for (TActorIterator<ACivilianCharacter> CivItr(GetWorld()); CivItr; ++CivItr)
    {
        ACivilianCharacter* Civ = *CivItr;
        if (Civ && Civ->AssignedComputer && Civ->AssignedComputer->TimelineEra == ETimelineEra::Past)
        {
            ComputerToCivilianMap.Add(Civ->AssignedComputer, Civ);
        }
    }
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    // Filter for computers with a non-empty code
    TArray<AComputer*> ComputersWithCodes;
    for (AComputer* Computer : CodeComputers)
    {
        if (Computer && !Computer->StoredCode.IsEmpty())
        {
            ComputersWithCodes.Add(Computer);
        }
    }

    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn) return;
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) return;

    if (GeneratorRef && ComputersWithCodes.Num() > 0)
    {
        if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
        {
            TArray<FString> CivilianNames;
            TArray<UTexture2D*> CivilianPortraits;
            for (AComputer* Comp : ComputersWithCodes)
            {
                FString Name = Comp->StaffName;
                UTexture2D* Portrait = nullptr;
                if (ACivilianCharacter* FoundCiv = ComputerToCivilianMap.FindRef(Comp))
                {
                    Name = FoundCiv->CivilianName;
                    Portrait = FoundCiv->PortraitTexture;
                }
                CivilianNames.Add(Name);
                CivilianPortraits.Add(Portrait);
            }
            MyPC->ClientShowCalendarWidget(
                GeneratorRef->RandomDate.Year,
                GeneratorRef->RandomDate.Month,
                GeneratorRef->RandomDate.Day,
                CivilianNames,
                CivilianPortraits
            );
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