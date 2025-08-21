#include "ItemPickup.h"
#include "Components/StaticMeshComponent.h"

AItemPickup::AItemPickup()
{
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
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
    }
}