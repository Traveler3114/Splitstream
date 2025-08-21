#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "InventorySystem/InventoryComponent.h"
#include "CharacterOverlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void ACharacterHUD::AddCharacterOverlay()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterOverlayClass)
    {
        CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        CharacterOverlay->AddToViewport();

        APawn* Pawn = PlayerController->GetPawn();
        if (Pawn)
        {
            UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
            if (Inventory)
            {
                CharacterOverlay->LinkedInventory = Inventory;
                Inventory->OnInventoryChanged.AddDynamic(CharacterOverlay, &UCharacterOverlay::OnInventoryChanged);
                CharacterOverlay->OnInventoryChanged(Inventory->Slots);
            }
        }
    }
}