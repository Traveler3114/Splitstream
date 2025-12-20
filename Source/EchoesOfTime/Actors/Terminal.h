#pragma once

#include "CoreMinimal.h"
#include "Actors/RepairableBase.h"
#include "Interfaces/IInteractable.h"
#include "Terminal.generated.h"

UCLASS()
class ECHOESOFTIME_API ATerminal : public ARepairableBase, public IInteractable
{
    GENERATED_BODY()
public:
    ATerminal();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* TerminalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniGame")
    TSubclassOf<class UFirewallMiniGame> FirewallMiniGameClass;

    void HandleMiniGameEnded_Internal(bool bWasVictory);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual void RequestRepair(AActor* RepairInstigator) override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
    bool bEnabled = true;

    UPROPERTY()
    UFirewallMiniGame* MiniGameInstance;

    UFUNCTION()
    void OnMiniGameEnded(bool bWasVictory);

private:

    TWeakObjectPtr<APlayerController> LastInteractingPC;
};