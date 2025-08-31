#include "KeycardScanner.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/ItemBase.h"

AKeycardScanner::AKeycardScanner()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(DefaultSceneRoot);

    LinkedActor = nullptr;
}

void AKeycardScanner::BeginPlay()
{
    Super::BeginPlay();

    // Optionally: you could do anything with the LinkedActor here if needed
    // For example: set bRequiresKeycard, but that's usually done in the actor itself
}

void AKeycardScanner::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor || !LinkedActor)
        return;

    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
        return;

    FInventorySlot ActiveSlot = Inventory->GetActiveItem();
    UItemBase* ActiveItem = ActiveSlot.ItemAsset;
    if (ActiveItem && ActiveItem->ItemType == EItemType::Keycard)
    {
        // Use the keycard (trigger OnUsed logic)
        ActiveItem->OnUsed(Interactor);

        if (LinkedActor->GetClass()->ImplementsInterface(UKeycardUnlockable::StaticClass()))
        {
            IKeycardUnlockable::Execute_UnlockWithKeycard(LinkedActor, Interactor);
        }
        // Optional: feedback for success
    }
    else
    {
        // Optional: feedback for failure (no keycard in hand)
    }
}