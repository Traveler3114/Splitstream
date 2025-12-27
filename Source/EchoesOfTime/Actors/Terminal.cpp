#include "Terminal.h"
#include "Minigames/FirewallMiniGame.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "Controllers/DefaultPlayerController.h"
#include "Net/UnrealNetwork.h"

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
    if (!bEnabled || !Interactor)
        return;

    AController* InstigatorController = Interactor->GetInstigatorController();
    if (!InstigatorController)
        return;

    APlayerController* PC = Cast<APlayerController>(InstigatorController);
    if (!PC || !PC->IsLocalController())
        return;

    if (MiniGameInstance)
        return;

    MiniGameInstance = NewObject<UFirewallMiniGame>(this, FirewallMiniGameClass);
    if (MiniGameInstance)
    {
        MiniGameInstance->OnMiniGameEnded.AddDynamic(this, &ATerminal::OnMiniGameEnded);
        MiniGameInstance->StartGame(PC);

        // --- Store the interacting PC for delegate callback later ---
        LastInteractingPC = PC;
    }
}

void ATerminal::OnMiniGameEnded(bool bWasVictory)
{
    // (A) CLIENT: Notify server via PlayerController
    if (!HasAuthority())
    {
        APlayerController* PC = LastInteractingPC.IsValid() ? LastInteractingPC.Get() : GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
            {
                FServerActionPayload Payload;
                Payload.BoolValue = bWasVictory;
                MyPC->ServerExecuteAction(this, Payload);
            }
        }
        return;
    }

    // (B) SERVER: Handle result
    HandleMiniGameEnded_Internal(bWasVictory);
}

void ATerminal::HandleMiniGameEnded_Internal(bool bWasVictory)
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
        UE_LOG(LogTemp, Log, TEXT("Minigame lost!"));
        // Optionally, puzzle failed logic here
    }

    MiniGameInstance = nullptr;
}

void ATerminal::SetHighlighted_Implementation(bool bHighlight)
{
    if (!bEnabled)
    {
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

void ATerminal::ExecuteServerAction_Implementation(const FServerActionPayload& Payload)
{
    UE_LOG(LogTemp, Warning, TEXT("ATerminal::ExecuteServerAction_Implementation called! Payload.BoolValue=%s, Payload.IntValue=%d, Payload.FloatValue=%.2f"),
        Payload.BoolValue ? TEXT("true") : TEXT("false"),
        Payload.IntValue,
        Payload.FloatValue);

    HandleMiniGameEnded_Internal(Payload.BoolValue);
}

void ATerminal::RequestRepair(AActor* RepairInstigator)
{
    bEnabled = true;
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
    }
}

void ATerminal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATerminal, bEnabled);
}