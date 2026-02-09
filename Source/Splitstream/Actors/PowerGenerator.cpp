#include "PowerGenerator.h"
#include "ActorComponents/SearchComponent.h"
#include "UtilityLibrary.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"

APowerGenerator::APowerGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
    GeneratorMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);
}

void APowerGenerator::BeginPlay()
{
    Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &APowerGenerator::OnSearchComplete);
    }
    UUtilityLibrary::RegisterRepairable(this,this);
}

void APowerGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UUtilityLibrary::UnregisterRepairable(this, this);
    Super::EndPlay(EndPlayReason);
}

void APowerGenerator::OnSearchComplete()
{
    SetHighlighted_Implementation(false);

    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }

    // Notify robot guards/repair system via generic interface
    OnRepairRequested.Broadcast(this);
}

void APowerGenerator::RequestRepair_Implementation(AActor* RepairInstigator)
{
    if (SearchComponent)
    {
        SearchComponent->bSearched = false;
    }
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
    }
}

void APowerGenerator::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void APowerGenerator::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

bool APowerGenerator::IsProgressiveInteract_Implementation()
{
    return true;
}

void APowerGenerator::SetHighlighted_Implementation(bool bHighlight)
{
    if (GeneratorMesh)
    {
        GeneratorMesh->SetRenderCustomDepth(bHighlight);
        GeneratorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}