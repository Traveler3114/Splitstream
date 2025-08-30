#include "ItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    OverrideMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverrideMeshComp"));
    RootComponent = OverrideMeshComp;
    OverrideMeshComp->SetIsReplicated(true);

    ItemData = nullptr;
    ItemInstanceID.Invalidate();
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();

    // At BeginPlay, if OverrideMeshComp has a mesh set, keep it (do nothing)
    // If no mesh is set, fall back to ItemData mesh
    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::InitFromItemData(UItemBase* InItemData, FGuid InInstanceID)
{
    ItemData = InItemData;
    ItemInstanceID = InInstanceID;
    // Only set if there is no override mesh
    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::RefreshMeshFromItemData()
{
    if (OverrideMeshComp && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (!ItemData || !Interactor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        if (Inventory->AddItem(ItemData, ItemInstanceID))
        {
            OnPickedUp.Broadcast(Interactor, ItemData);
            Destroy();
        }
    }
}

#if WITH_EDITOR
void AItemPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Auto-generate unique instance ID if not set
    if (!ItemInstanceID.IsValid())
    {
        ItemInstanceID = FGuid::NewGuid();
    }

    // In editor, leave mesh as is (designer can set it directly on the component)
    // Optionally, you could preview ItemData mesh if none is set
    if (OverrideMeshComp && !OverrideMeshComp->GetStaticMesh() && ItemData && ItemData->ItemMesh)
    {
        OverrideMeshComp->SetStaticMesh(ItemData->ItemMesh);
        OverrideMeshComp->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}
#endif