#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/InventoryComponent.h"
#include "CharacterOverlay.generated.h"

class UHorizontalBox;

UCLASS()
class ECHOESOFTIME_API UCharacterOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* InventoryBox;

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Items);

    UPROPERTY()
    UInventoryComponent* LinkedInventory;
};