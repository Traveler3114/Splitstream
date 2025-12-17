#pragma once

#include "CoreMinimal.h"
#include "RepairableBase.h"
#include "Interfaces/IInteractable.h"
#include "PowerGenerator.generated.h"

UCLASS()
class ECHOESOFTIME_API APowerGenerator : public ARepairableBase, public IInteractable
{
    GENERATED_BODY()

public:
    APowerGenerator();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* GeneratorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
    class USearchComponent* SearchComponent;

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WirePuzzle|Completion")
    AActor* CompletionTarget = nullptr;

protected:

    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    /** Called when search is complete (searched by player) */
    UFUNCTION()
    virtual void OnSearchComplete();

    /** When the robot repairs this power generator */
    virtual void RequestRepair(AActor* RepairInstigator) override;
    virtual float GetRepairTime() const override;
};