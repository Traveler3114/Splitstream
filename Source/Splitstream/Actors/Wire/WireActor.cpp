// WireActor.cpp
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

    WireColor = EWireColor::None;
    bIsCut = false;
}

void AWireActor::BeginPlay()
{
    Super::BeginPlay();

    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AWireActor::OnSearchComplete);
    }
    if (HasAuthority() && WireColor != EWireColor::None)
    {
        ApplyWireColor();
    }
}

void AWireActor::OnSearchComplete()
{
    OnWireCut.Broadcast(this);

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

bool AWireActor::IsProgressiveInteract_Implementation()
{
    return true;
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

void AWireActor::OnRep_WireColor()
{
    ApplyWireColor();
}

void AWireActor::ApplyWireColor()
{
    if (WireMesh)
    {
        WireMID = WireMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (WireMID)
        {
            WireMID->SetVectorParameterValue(TEXT("WireColor"), GetWireLinearColor());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create MID in ApplyWireColor!"), *GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] WireMesh is null in ApplyWireColor!"), *GetName());
    }
}

void AWireActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireActor, bIsCut);
    DOREPLIFETIME(AWireActor, WireColor);
}