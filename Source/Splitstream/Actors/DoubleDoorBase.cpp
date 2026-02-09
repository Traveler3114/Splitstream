#include "DoubleDoorBase.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"

ADoubleDoorBase::ADoubleDoorBase()
{
    // Replace single mesh with two meshes, and clear the base class mesh pointer
    if (DoorMesh)
    {
        DoorMesh->DestroyComponent();
        DoorMesh = nullptr;
    }

    DoorRightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorRightMesh"));
    DoorRightMesh->SetupAttachment(SceneRoot);
    DoorRightMesh->SetIsReplicated(true);
    DoorRightMesh->SetCanEverAffectNavigation(false);

    DoorLeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeftMesh"));
    DoorLeftMesh->SetupAttachment(SceneRoot);
    DoorLeftMesh->SetIsReplicated(true);
    DoorLeftMesh->SetCanEverAffectNavigation(false);
}

void ADoubleDoorBase::SetHighlighted_Implementation(bool bHighlight)
{
    if (DoorLeftMesh && DoorRightMesh)
    {
        DoorLeftMesh->SetRenderCustomDepth(bHighlight);
        DoorLeftMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
        DoorRightMesh->SetRenderCustomDepth(bHighlight);
        DoorRightMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}