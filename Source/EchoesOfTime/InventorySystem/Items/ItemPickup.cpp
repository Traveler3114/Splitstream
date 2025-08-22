#include "ItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    //MeshComponent->SetSimulatePhysics(true);
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
    if (Inventory && Inventory->AddItem(ItemData->GetClass()))
    {
        Destroy();
    }
}