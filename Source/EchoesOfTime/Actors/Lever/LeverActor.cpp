#include "LeverActor.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"

ALeverActor::ALeverActor()
{
    bReplicates = true;

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRootComp);

    // Set up lever mesh
    LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
    LeverMesh->SetupAttachment(SceneRootComp);
    LeverMesh->SetIsReplicated(true);

    LeverBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverBaseMesh"));
    LeverBaseMesh->SetupAttachment(SceneRootComp);


    TextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComp"));
    TextRenderComp->SetupAttachment(SceneRootComp);
}

void ALeverActor::BeginPlay()
{
    Super::BeginPlay();
	TextRenderComp->SetText(FText::FromString(FString::Printf(TEXT("%d"), OrderIndex)));
}

void ALeverActor::Interact_Implementation(AActor* Interactor)
{
    if (OrderIndex < 0)
    {
        if (!bActivated)
        {
            bActivated = true;
            OnRep_Activated();
        }
        return;
    }
    OnLeverInteracted.Broadcast(this); // Broadcast the lever instance pointer!
}

void ALeverActor::OnRep_Activated()
{
    if (bActivated)
        OnActivated();
    else
        OnDeactivated();
}

void ALeverActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (LeverMesh)
    {
        LeverMesh->SetRenderCustomDepth(bHighlight);
        LeverMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
    if (LeverBaseMesh)
    {
        LeverBaseMesh->SetRenderCustomDepth(bHighlight);
        LeverBaseMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void ALeverActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALeverActor, bActivated);
    DOREPLIFETIME(ALeverActor, OrderIndex);
}