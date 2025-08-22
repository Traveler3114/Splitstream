#include "KeycardScanner.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/ItemBase.h"
#include "Actors/DoorBase.h"

AKeycardScanner::AKeycardScanner()
{
    // Initialize components if needed
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(DefaultSceneRoot);

    LinkedDoor = nullptr;
}

void AKeycardScanner::BeginPlay()
{
    Super::BeginPlay();

    if (LinkedDoor)
    {
        LinkedDoor->bRequiresKeycard = true;
    }
}

void AKeycardScanner::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor || !LinkedDoor) return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
        return;

    bool bHasKeycard = false;
    for (UItemBase* Item : Inventory->GetItemInstances())
    {
        if (Item && Item->ItemType == EItemType::Keycard)
        {
            bHasKeycard = true;
            break;
        }
    }

    if (bHasKeycard)
    {
        if (LinkedDoor->HasAuthority())
        {
            LinkedDoor->bIsOpen = true;
            LinkedDoor->OnRep_IsOpen(); // Optional: immediately update on server
        }
        // Optional: feedback for success
    }
    else
    {
        // Optional: feedback for failure
    }
}