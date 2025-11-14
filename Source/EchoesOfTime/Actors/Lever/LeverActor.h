#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "LeverActor.generated.h"

// Only DECLARE the delegate ONCE here!
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeverInteracted, ALeverActor*, Lever);

UCLASS()
class ECHOESOFTIME_API ALeverActor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ALeverActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    USceneComponent* SceneRootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    UStaticMeshComponent* LeverMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
    UStaticMeshComponent* LeverBaseMesh;

    UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category="Lever")
    int32 OrderIndex = -1; // -1 for solo

    UPROPERTY(ReplicatedUsing=OnRep_Activated)
    bool bActivated = false;

    // Event for manager to bind to!
    UPROPERTY(BlueprintAssignable, Category="Lever")
    FOnLeverInteracted OnLeverInteracted;

    // Interaction interface
    virtual void Interact_Implementation(AActor* Interactor) override;

    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    UFUNCTION()
    void OnRep_Activated();

    UFUNCTION(BlueprintImplementableEvent, Category="Lever")
    void OnActivated();

    UFUNCTION(BlueprintImplementableEvent, Category="Lever")
    void OnDeactivated();


protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};