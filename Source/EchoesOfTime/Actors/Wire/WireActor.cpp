#include "WireActor.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"

AWireActor::AWireActor()
{
    bReplicates = true;
    NetUpdateFrequency = 10.f; // Wires change only when cut
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    WireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WireMesh"));
    WireMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);

    WireColor = EWireColor::Red;
    bIsCut = false;
}

void AWireActor::BeginPlay()
{
    Super::BeginPlay();

    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AWireActor::OnSearchComplete);
    }
}

void AWireActor::OnSearchComplete()
{
    OnWireCut.Broadcast(this);

    // Only hide/cut if not already cut
    if (!bIsCut)
    {
        bIsCut = true;
        OnRep_CutState();
    }
}

void AWireActor::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void AWireActor::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

void AWireActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (WireMesh && !bIsCut)
    {
        WireMesh->SetRenderCustomDepth(bHighlight);
        WireMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void AWireActor::OnRep_CutState()
{
    if (WireMesh)
        WireMesh->SetVisibility(!bIsCut, true);
}

void AWireActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireActor, bIsCut);
}