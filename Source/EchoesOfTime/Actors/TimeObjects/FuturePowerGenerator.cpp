#include "FuturePowerGenerator.h"
#include "Minigames/FirewallMiniGame.h"
#include "PastPowerGenerator.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "Net/UnrealNetwork.h"
#include "Controllers/DefaultPlayerController.h"

AFuturePowerGenerator::AFuturePowerGenerator()
{
}

void AFuturePowerGenerator::BeginPlay()
{
    Super::BeginPlay();
    SearchComponent = nullptr;

    APastPowerGenerator* Past = nullptr;
    if (PastGenerator.IsValid())
    {
        Past = PastGenerator.Get();
    }
    else if (PastGenerator.ToSoftObjectPath().IsValid())
    {
        Past = Cast<APastPowerGenerator>(PastGenerator.LoadSynchronous());
    }
    if (Past)
    {
        Past->OnGeneratorCompleted.AddDynamic(this, &AFuturePowerGenerator::HandlePastGeneratorCompleted);
    }
}

void AFuturePowerGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    APastPowerGenerator* Past = nullptr;
    if (PastGenerator.IsValid())
    {
        Past = PastGenerator.Get();
    }
    else if (PastGenerator.ToSoftObjectPath().IsValid())
    {
        Past = Cast<APastPowerGenerator>(PastGenerator.LoadSynchronous());
    }
    if (Past)
    {
        Past->OnGeneratorCompleted.RemoveDynamic(this, &AFuturePowerGenerator::HandlePastGeneratorCompleted);
    }
    Super::EndPlay(EndPlayReason);
}

void AFuturePowerGenerator::Interact_Implementation(AActor* Interactor)
{
    if (!bEnabled || !Interactor)
        return;

    // MINIGAME LOGIC
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
        MiniGameInstance->OnMiniGameEnded.AddDynamic(this, &AFuturePowerGenerator::OnMiniGameEnded);
        MiniGameInstance->StartGame(PC);
        LastInteractingPC = PC;
    }
}

void AFuturePowerGenerator::CancelInteract_Implementation(AActor* Interactor)
{
    // No-op or implement minigame cancellation if needed
}

void AFuturePowerGenerator::SetHighlighted_Implementation(bool bHighlight)
{
    // Highlight only if enabled (not completed)
    if (!bEnabled)
    {
        if (GeneratorMesh)
        {
            GeneratorMesh->SetRenderCustomDepth(false);
            GeneratorMesh->CustomDepthStencilValue = 0;
        }
        return;
    }

    if (GeneratorMesh)
    {
        GeneratorMesh->SetRenderCustomDepth(bHighlight);
        GeneratorMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void AFuturePowerGenerator::HandlePastGeneratorCompleted(bool bPastSearched)
{
    if (bPastSearched)
    {
        bEnabled = false;
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
        }
        // Remove highlight
        SetHighlighted_Implementation(false);
        // Optionally: Call OnRequestRepair.Broadcast(this); or puzzle completion handling
        OnRequestRepair.Broadcast(this);
    }
	else if (bPastSearched == false)
    {
        bEnabled = true;
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
        }
    }
}

void AFuturePowerGenerator::OnMiniGameEnded(bool bWasVictory)
{
    if (MiniGameInstance)
    {
        MiniGameInstance = nullptr;
    }

    if (bWasVictory)
    {
        bEnabled = false;
        SetHighlighted_Implementation(false);
        // Fire puzzle completion / repair like Terminal
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
        }
        OnRequestRepair.Broadcast(this);
    }
    else
    {
        // Optionally: MiniGame failed, handle as needed (sound, UI, etc)
    }
}

void AFuturePowerGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFuturePowerGenerator, bEnabled);
}

bool AFuturePowerGenerator::IsProgressiveInteract_Implementation()
{
    return false;
}

void AFuturePowerGenerator::RequestRepair(AActor* RepairInstigator)
{
    bEnabled = true;
    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
    }
}