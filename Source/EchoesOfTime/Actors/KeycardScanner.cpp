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
    if (GEngine)
    {
        FString NetModeString;
        switch (GetNetMode())
        {
        case NM_Client: NetModeString = TEXT("Client"); break;
        case NM_ListenServer: NetModeString = TEXT("ListenServer"); break;
        case NM_DedicatedServer: NetModeString = TEXT("DedicatedServer"); break;
        default: NetModeString = TEXT("Standalone"); break;
        }
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
            FString::Printf(TEXT("KeycardScanner Interact: HasAuthority=%d, NetMode=%s, Role=%d"),
                (int32)HasAuthority(), *NetModeString, (int32)GetLocalRole()));
    }
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