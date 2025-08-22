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

    UItemBase* ActiveItem = Inventory->GetActiveItem();
    if (ActiveItem && ActiveItem->ItemType == EItemType::Keycard)
    {
        // Use the keycard (trigger OnUsed logic)
        ActiveItem->OnUsed(Interactor);

        if (LinkedDoor->HasAuthority())
        {
            LinkedDoor->bIsOpen = true;
            LinkedDoor->OnRep_IsOpen();
        }
        // Optional: feedback for success
    }
    else
    {
        // Optional: feedback for failure (no keycard in hand)
    }
}