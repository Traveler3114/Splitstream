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
    RequiredKeycardType = EItemType::KeycardL1; // Default
}

void AKeycardScanner::BeginPlay()
{
    Super::BeginPlay();
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
    if (ActiveItem && ActiveItem->ItemType == RequiredKeycardType)
    {
        ActiveItem->OnUsed(Interactor);

        if (LinkedActor->GetClass()->ImplementsInterface(UKeycardUnlockable::StaticClass()))
        {
            IKeycardUnlockable::Execute_UnlockWithKeycard(LinkedActor, Interactor);
        }
    }
    else
    {
        // Optional: feedback for failure (wrong/no keycard)
    }
}