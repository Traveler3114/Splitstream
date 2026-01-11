#include "LeverActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/ArrowComponent.h"

ALeverActor::ALeverActor()
{
    bReplicates = true;

    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRootComp);

    LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
    LeverMesh->SetupAttachment(SceneRootComp);

    LeverBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverBaseMesh"));
    LeverBaseMesh->SetupAttachment(SceneRootComp);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);
}

void ALeverActor::BeginPlay()
{
    Super::BeginPlay();
}

void ALeverActor::Interact_Implementation(AActor* Interactor)
{
    // Use solo flag
    if (bIsSolo)
    {
        if (!bActivated)
        {
            bActivated = true;
            OnRep_Activated();
        }
        return;
    }
    OnLeverInteracted.Broadcast(this);
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

// Remove OnRep_OrderIndex and references to OrderIndex, unless you still want to display something!
void ALeverActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALeverActor, bActivated);
    DOREPLIFETIME(ALeverActor, SpawnLocationName);
}