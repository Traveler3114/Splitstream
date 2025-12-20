// Fill out your copyright notice in the Description page of Project Settings.


#include "Terminal.h"
#include "Minigames/FirewallMiniGame.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"

ATerminal::ATerminal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalMesh"));
	TerminalMesh->SetupAttachment(SceneRoot);
}

void ATerminal::Interact_Implementation(AActor* Interactor)
{
	if (!bEnabled)
		return;

	if (!Interactor)
		return;

	AController* InstigatorController = Interactor->GetInstigatorController();
	if (!InstigatorController)
		return;

	APlayerController* PC = Cast<APlayerController>(InstigatorController);
	if (!PC)
		return;

	if (MiniGameInstance)
		return;

	MiniGameInstance = NewObject<UFirewallMiniGame>(this, FirewallMiniGameClass);
	if (MiniGameInstance)
	{
		MiniGameInstance->OnMiniGameEnded.AddDynamic(this, &ATerminal::OnMiniGameEnded);
		MiniGameInstance->StartGame(PC);
	}
}

void ATerminal::OnMiniGameEnded(bool bWasVictory)
{
	if (bWasVictory)
	{
		bEnabled = false;
		SetHighlighted_Implementation(false);
		if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
		{
			IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
		}
		OnRequestRepair.Broadcast(this);
	}
	else
	{
		// Do something for lose, e.g.
		UE_LOG(LogTemp, Log, TEXT("Minigame lost!"));
		// “Puzzle failed” logic...
	}

	MiniGameInstance = nullptr;
}


void ATerminal::SetHighlighted_Implementation(bool bHighlight)
{
	if (!bEnabled)
	{
		// Ensure highlight is never set if disabled
		if (TerminalMesh)
		{
			TerminalMesh->SetRenderCustomDepth(false);
			TerminalMesh->CustomDepthStencilValue = 0;
		}
		return;
	}

	if (TerminalMesh)
	{
		TerminalMesh->SetRenderCustomDepth(bHighlight);
		TerminalMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
	}
}


void ATerminal::RequestRepair(AActor* RepairInstigator)
{
	bEnabled = true;
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
    }
}
