#include "ArchiveComputer.h"
#include "ProceduralLevelGenerator.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Controllers/DefaultPlayerController.h"

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
}

void AArchiveComputer::Interact_Implementation(AActor* Interactor)
{
    APawn* Pawn = Cast<APawn>(Interactor);
    if (!Pawn) return;

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC) return;

    if (GeneratorRef)
    {
        // Call the client RPC to show the widget ONLY for the owning client
        // Cast to your custom controller if needed
        ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC);
        if (MyPC)
        {
            MyPC->ClientShowCalendarWidget(
                GeneratorRef->RandomDate.Year,
                GeneratorRef->RandomDate.Month,
                GeneratorRef->RandomDate.Day,
                GeneratorRef->CodeComputerStaffName
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