// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "InventorySystem/InventoryComponent.h"
#include "CharacterOverlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void ACharacterHUD::DrawHUD() {
	Super::DrawHUD();
	FVector2D ViewportSize;
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	}
}

// CharacterHUD.cpp
void ACharacterHUD::AddCharacterOverlay()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterOverlayClass)
    {
        CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        CharacterOverlay->AddToViewport();

        // Decoupled: Get any pawn, look for an InventoryComponent
        APawn* Pawn = PlayerController->GetPawn();
        if (Pawn)
        {
            UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
            if (Inventory)
            {
                Inventory->OnInventoryChanged.AddDynamic(CharacterOverlay, &UCharacterOverlay::OnInventoryChanged);
                CharacterOverlay->OnInventoryChanged(Inventory->Slots);
            }
        }
    }
}

