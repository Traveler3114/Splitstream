#include "ArchiveComputer.h"
#include "ProceduralLevelGenerator.h"
#include "Computer.h"
#include "Kismet/GameplayStatics.h"
#include "Controllers/DefaultPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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

    // Find all computers with a non-empty code
    TArray<AActor*> FoundComputers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AComputer::StaticClass(), FoundComputers);

    CodeComputers.Empty();
    for (AActor* Actor : FoundComputers)
    {
        AComputer* Computer = Cast<AComputer>(Actor);
        if (Computer && !Computer->StoredCode.IsEmpty())
        {
            CodeComputers.Add(Computer);
        }
    }
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn) return;

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) return;

    if (GeneratorRef && CodeComputers.Num() > 0)
    {
        ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
        if (MyPC)
        {
            TArray<FString> StaffNames;
            for (AComputer* Comp : CodeComputers)
                StaffNames.Add(Comp->StaffName);

            MyPC->ClientShowCalendarWidget(
                GeneratorRef->RandomDate.Year,
                GeneratorRef->RandomDate.Month,
                GeneratorRef->RandomDate.Day,
                StaffNames
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