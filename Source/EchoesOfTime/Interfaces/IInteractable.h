#pragma once

#include "UObject/Interface.h"
#include "IInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IInteractable
{
    GENERATED_BODY()

public:
    // Use BlueprintNativeEvent so you can implement in C++ or Blueprint
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void CancelInteract(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void SetHighlighted(bool bHighlight);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsProgressiveInteract();
};