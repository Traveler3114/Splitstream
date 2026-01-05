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

void AFuturePowerGenerator::HandlePastGeneratorCompleted(bool bPastSearched)
{
    // Only authority/server should set bEnabled
    if (HasAuthority())
    {
        bEnabled = !bPastSearched;
        if (bPastSearched)
        {
            if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
            {
                IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
            }
            SetHighlighted_Implementation(false);
            // Broadcast for repair, using CORRECT delegate
            OnRepairRequested.Broadcast(this);
        }
        else // bPastSearched == false
        {
            if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
            {
                IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
            }
        }

        APastPowerGenerator* Past = PastGenerator.IsValid() ? PastGenerator.Get() : nullptr;
        if (Past)
        {
            if (Past->ToggleCount == 2 && !bFutureInteracted)
            {
                bEasterEggActive = true;
            }
            else
            {
                bEasterEggActive = false;
            }
        }
        else
        {
            bEasterEggActive = false;
        }
    }
    else
    {
        // Clients request the server to update bEnabled -- DEPRECATED: use interface!
        FServerActionPayload Payload;
        Payload.BoolValue = !bPastSearched;
        // Get the owning PC!
        if (APlayerController* PC = LastInteractingPC.IsValid() ? LastInteractingPC.Get() : nullptr)
        {
            if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
            {
                MyPC->ServerExecuteAction(this, Payload);
            }
        }
    }
}

void AFuturePowerGenerator::Interact_Implementation(AActor* Interactor)
{
    bFutureInteracted = true;
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
        MiniGameInstance->bUseEasterEggSprites = bEasterEggActive;
        MiniGameInstance->OnMiniGameEnded.AddDynamic(this, &AFuturePowerGenerator::OnMiniGameEnded);
        MiniGameInstance->StartGame(PC);
        LastInteractingPC = PC;
    }
}

void AFuturePowerGenerator::ExecuteServerAction_Implementation(const FServerActionPayload& Payload)
{
    // This is executed on the server via the PC's ServerExecuteAction RPC
    UE_LOG(LogTemp, Warning, TEXT("AFuturePowerGenerator::ExecuteServerAction_Implementation called! Payload.BoolValue=%s"),
        Payload.BoolValue ? TEXT("true") : TEXT("false"));
    bEnabled = Payload.BoolValue;
    if (!bEnabled)
    {
        SetHighlighted_Implementation(false);
        // Fire puzzle completion / repair like Terminal
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
        }
        OnRepairRequested.Broadcast(this);
    }
    else
    {
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
        }
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

void AFuturePowerGenerator::OnMiniGameEnded(bool bWasVictory)
{
    if (MiniGameInstance)
    {
        MiniGameInstance = nullptr;
    }

    if (bWasVictory)
    {
        if (HasAuthority())
        {
            bEnabled = false;
            SetHighlighted_Implementation(false);
            // Fire puzzle completion / repair like Terminal
            if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
            {
                IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
            }
            OnRepairRequested.Broadcast(this);
        }
        else
        {
            // Use the new action pattern to safely communicate to the server!
            if (APlayerController* PC = LastInteractingPC.IsValid() ? LastInteractingPC.Get() : nullptr)
            {
                if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
                {
                    FServerActionPayload Payload;
                    Payload.BoolValue = false; // Disable this generator
                    MyPC->ServerExecuteAction(this, Payload);
                }
            }
        }
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
    DOREPLIFETIME(AFuturePowerGenerator, bFutureInteracted);
    DOREPLIFETIME(AFuturePowerGenerator, bEasterEggActive);
}

bool AFuturePowerGenerator::IsProgressiveInteract_Implementation()
{
    return false;
}

void AFuturePowerGenerator::RequestRepair_Implementation(AActor* RepairInstigator)
{
    if (HasAuthority())
    {
        bEnabled = true;
        if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
        {
            IPuzzleCompletionReceiver::Execute_OnPuzzleReset(CompletionTarget);
        }
    }
    else
    {
        // Use the server action interface for consistency!
        if (APlayerController* PC = LastInteractingPC.IsValid() ? LastInteractingPC.Get() : nullptr)
        {
            if (ADefaultPlayerController* MyPC = Cast<ADefaultPlayerController>(PC))
            {
                FServerActionPayload Payload;
                Payload.BoolValue = true; // Enable this generator
                MyPC->ServerExecuteAction(this, Payload);
            }
        }
    }
}