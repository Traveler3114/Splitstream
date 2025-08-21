#include "ItemPickup.h"
#include "InventorySystem/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

AItemPickup::AItemPickup()
{
    bReplicates = true;
    SetReplicateMovement(true);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetSimulatePhysics(true);
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
    if (Inventory && Inventory->AddItem(ItemData))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ItemPickup: Item added to inventory!"));
        }
        UE_LOG(LogTemp, Warning, TEXT("ItemPickup: %s picked up %s"), *Interactor->GetName(), *ItemData->GetName());
        Destroy(); // Only the server destroys the actor, replicates to all clients
    }
}
