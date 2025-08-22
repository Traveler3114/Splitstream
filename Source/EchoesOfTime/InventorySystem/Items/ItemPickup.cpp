#include "ItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Actors/TimeObjects/FutureItemPickup.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetIsReplicated(true);
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

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (Inventory)
    {
        // Register for invalidation event if this is a Future pickup
        if (AFutureItemPickup* FuturePickup = Cast<AFutureItemPickup>(this))
        {
            Inventory->RegisterFutureInstance(ItemData->ItemInstanceID);
        }

        if (Inventory->AddItem(ItemData->GetClass(), ItemData->ItemInstanceID))
        {
            Destroy();
        }
    }
}