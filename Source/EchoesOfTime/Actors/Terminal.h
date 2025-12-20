// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/RepairableBase.h"
#include "Interfaces/IInteractable.h"
#include "Terminal.generated.h"

/**
 * 
 */
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
protected:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;

	virtual void RequestRepair(AActor* RepairInstigator) override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	bool bEnabled = true;

	UPROPERTY()
	UFirewallMiniGame* MiniGameInstance;

	UFUNCTION()
	void OnMiniGameEnded(bool bWasVictory);
};
