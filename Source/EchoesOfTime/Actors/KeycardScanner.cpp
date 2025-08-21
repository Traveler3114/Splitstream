#include "KeycardScanner.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/Items/ItemBase.h"
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

    // Check for a keycard
    bool bHasKeycard = false;
    for (UItemBase* Item : Inventory->GetItems())
    {
        if (Item && Item->ItemType == EItemType::Keycard)
        {
            bHasKeycard = true;
            break;
        }
    }

    if (bHasKeycard)
    {
        LinkedDoor->OpenDoor();
        // Optional: feedback for success
    }
    else
    {
        // Optional: feedback for failure
    }
}