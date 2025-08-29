#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/InventoryComponent.h"
#include "CharacterOverlay.generated.h"

class UHorizontalBox;
class UTextBlock;

UCLASS()
class ECHOESOFTIME_API UCharacterOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* InventoryBox;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* status_txt;

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Items);

    UFUNCTION(BlueprintCallable)
    void SetStatusText(const FString& NewStatus);

    UPROPERTY()
    UInventoryComponent* LinkedInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float TextureScale = 1.25f;
};