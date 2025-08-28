#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "InventorySystem/InventoryComponent.h"
#include "CharacterOverlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "AbilitySystemComponent.h"
#include "DefaultPlayerState.h"
#include "AbilitySystem/EOTGameplayTags.h"
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
			BindTags(Pawn);
        }
    }
}

void ACharacterHUD::BindTags(APawn* Pawn) {
    if (!Pawn || !CharacterOverlay) return;

    UAbilitySystemComponent* ASC = nullptr;
    if (ADefaultPlayerState* PS = Pawn->GetPlayerState<ADefaultPlayerState>())
    {
        ASC = PS->GetAbilitySystemComponent();
    }
    if (ASC)
    {
        FGameplayTag IllegalTag = TAG_Character_Status_Illegal;
        ASC->RegisterGameplayTagEvent(IllegalTag, EGameplayTagEventType::NewOrRemoved)
            .AddLambda([this](const FGameplayTag Tag, int32 NewCount)
                {
                    if (CharacterOverlay)
                    {
                        if (NewCount > 0)
                        {
                            CharacterOverlay->SetStatusText(TEXT("Illegal"));
                        }
                        else
                        {
                            CharacterOverlay->SetStatusText(TEXT(""));
                        }
                    }
                });
    }
}