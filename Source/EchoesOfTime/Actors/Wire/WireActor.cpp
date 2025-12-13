#include "WireActor.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"

AWireActor::AWireActor()
{
    bReplicates = true;
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

FLinearColor AWireActor::GetWireLinearColor() const
{
    switch (WireColor)
    {
    case EWireColor::Red:    return FLinearColor::Red;
    case EWireColor::Green:  return FLinearColor::Green;
    case EWireColor::Blue:   return FLinearColor::Blue;
    case EWireColor::Yellow: return FLinearColor(1.f, 1.f, 0.f);
    case EWireColor::Orange: return FLinearColor(1.f, 0.5f, 0.f);
    case EWireColor::Purple: return FLinearColor(0.5f, 0.f, 1.f);
    default:                 return FLinearColor::White;
    }
}

void AWireActor::ApplyWireColor()
{
    if (!WireMesh)
        return;

    // Create a dynamic material instance (slot 0) if not already
    UMaterialInstanceDynamic* MID = WireMesh->CreateAndSetMaterialInstanceDynamic(0);
    if (!MID)
        return;

    // Assumes your material has a Vector parameter named "WireColor"
    MID->SetVectorParameterValue(TEXT("WireColor"), GetWireLinearColor());
}

void AWireActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireActor, bIsCut);
}