#include "PowerGenerator.h"
#include "ActorComponents/SearchComponent.h"
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
}

void APowerGenerator::OnSearchComplete()
{
    SetHighlighted_Implementation(false);

    // Fire puzzle completion, if any
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }

    // Notify robot guards/repair system
    OnRequestRepair.Broadcast(this);
}

void APowerGenerator::RequestRepair(AActor* RepairInstigator)
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

float APowerGenerator::GetRepairTime() const
{
    // This could be dynamic/per-instance if you wish
    return RepairTime;
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