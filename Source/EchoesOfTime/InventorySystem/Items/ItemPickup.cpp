#include "ItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

AItemPickup::AItemPickup()
{
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetSimulatePhysics(true);
    ItemData = nullptr;
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();

    if (ItemData)
    {
        InitFromItemData(ItemData);
    }
}

void AItemPickup::InitFromItemData(UItemBase* InItemData)
{
    ItemData = InItemData;
    if (ItemData && ItemData->ItemMesh)
    {
        MeshComponent->SetStaticMesh(ItemData->ItemMesh);
        MeshComponent->SetWorldScale3D(ItemData->PickupMeshScale);
    }
}

void AItemPickup::Interact_Implementation(AActor* Interactor)
{
    if (!ItemData || !Interactor) return;

    // Try to find an inventory component on the character who interacted
    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        // Add the item to inventory. If successful, destroy the pickup.
        if (Inventory->AddItem(ItemData))
        {
            Destroy();
        }
        // Optionally, you can give feedback if inventory is full
    }
}