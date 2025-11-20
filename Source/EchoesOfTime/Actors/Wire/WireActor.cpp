#include "WireActor.h"
#include "Net/UnrealNetwork.h"

AWireActor::AWireActor()
{
    bReplicates = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    WireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WireMesh"));
    WireMesh->SetupAttachment(SceneRoot);

    WireColor = EWireColor::Red;
    bIsCut = false;
}

void AWireActor::BeginPlay()
{
    Super::BeginPlay();
}


void AWireActor::Interact_Implementation(AActor* InteractingActor)
{
    // Always notify manager, even if already cut
    OnWireCut.Broadcast(this);

    // Only hide/cut if not already cut
    if (!bIsCut)
    {
        bIsCut = true;
        OnRep_CutState();
    }
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