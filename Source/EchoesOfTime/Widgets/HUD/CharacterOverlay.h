#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UHorizontalBox;

UCLASS()
class ECHOESOFTIME_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	/** This function will be called when inventory changes */
	UFUNCTION()
	void OnInventoryChanged(const TArray<class UItemBase*>& Items);

protected:
	// Bind this to your UMG HorizontalBox (named InventoryBox)
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* InventoryBox;
};