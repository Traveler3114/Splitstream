#pragma once

#include "UObject/Interface.h"
#include "ActorComponents/InventoryComponent.h"
#include "IInteractable.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class SPLITSTREAM_API IInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void CancelInteract(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void SetHighlighted(bool bHighlight);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsProgressiveInteract();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsCorrectItem(const FInventorySlot& Slot) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool RequiresItem() const;
};