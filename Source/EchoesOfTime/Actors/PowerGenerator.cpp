// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerGenerator.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "ActorComponents/SearchComponent.h"

// Sets default values
APowerGenerator::APowerGenerator()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
    GeneratorMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void APowerGenerator::BeginPlay()
{
    Super::BeginPlay();
    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &APowerGenerator::OnSearchComplete);
    }
}

void APowerGenerator::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent && SearchComponent->bSearched)
        return;

    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void APowerGenerator::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent && SearchComponent->bSearched)
        return;

    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

bool APowerGenerator::IsProgressiveInteract_Implementation()
{
    return true;
}

void APowerGenerator::SetHighlighted_Implementation(bool bHighlight)
{
    // Do not highlight if already searched
    if (SearchComponent && SearchComponent->bSearched)
    {
        if (GeneratorMesh)
        {
            GeneratorMesh->SetRenderCustomDepth(false);
            GeneratorMesh->CustomDepthStencilValue = 0;
        }
        return;
    }

    if (GeneratorMesh)
    {
        GeneratorMesh->SetRenderCustomDepth(bHighlight);
        GeneratorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void APowerGenerator::OnSearchComplete()
{
	SearchComponent->bSearched = true;
    SetHighlighted_Implementation(false);
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }
}