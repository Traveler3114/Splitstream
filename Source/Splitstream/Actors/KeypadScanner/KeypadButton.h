#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "KeypadButton.generated.h"

// DELEGATE DECLARATION
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonPressed, const FString&, Symbol);

UCLASS()
class ECHOESOFTIME_API AKeypadButton : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AKeypadButton();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    USceneComponent* DefaultSceneRoot;

    // Static mesh child
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    UStaticMeshComponent* KeypadButtonMesh;

    // Text render child
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    class UTextRenderComponent* NumberTextRenderComp;

    // The symbol this button represents ("1", "2", ..., "*", "#")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeypadButton")
    FString ButtonSymbol;

    // Delegate for press event
    UPROPERTY(BlueprintAssignable, Category = "KeypadButton")
    FOnButtonPressed OnButtonPressed;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Interactable interface implementation
    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
};