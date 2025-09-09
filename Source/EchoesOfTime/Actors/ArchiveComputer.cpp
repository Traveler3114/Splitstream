#include "ArchiveComputer.h"
#include "ProceduralLevelGenerator.h"
#include "Widgets/CalendarWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

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
    if (CalendarWidgetClass && GeneratorRef)
    {
        CalendarWidgetInstance = CreateWidget<UCalendarWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), CalendarWidgetClass);
        if (CalendarWidgetInstance)
        {
            // Pass target date and staff name from generator to widget
            CalendarWidgetInstance->TargetYear = GeneratorRef->RandomDate.Year;
            CalendarWidgetInstance->TargetMonth = GeneratorRef->RandomDate.Month;
            CalendarWidgetInstance->TargetDay = GeneratorRef->RandomDate.Day;
            CalendarWidgetInstance->TargetStaffName = GeneratorRef->CodeComputerStaffName;

            CalendarWidgetInstance->AddToViewport();
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
