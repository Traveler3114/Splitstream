#include "LeverManager.h"
#include "LeverActor.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "Net/UnrealNetwork.h"

ALeverManager::ALeverManager()
{
    bReplicates = true;
}

void ALeverManager::SetupPuzzle(const TArray<int32>& OrderSeq)
{
    if (!HasAuthority()) return;

    Order = OrderSeq;
    ProgressIndex = 0;
    bCompleted = false;

    for (int32 idx = 0; idx < PuzzleLevers.Num(); ++idx)
    {
        auto* Lever = PuzzleLevers[idx];
        if (Lever)
        {
            Lever->OrderIndex = idx;
            Lever->bActivated = false;
            // Bind manager to lever event
            Lever->OnRep_OrderIndex();
            Lever->OnLeverInteracted.AddDynamic(this, &ALeverManager::OnLeverInteracted);
        }
    }
}

void ALeverManager::OnLeverInteracted(ALeverActor* Lever)
{
    if (!HasAuthority() || bCompleted) return;
    if (!Lever) return;
    int32 LeverIdx = PuzzleLevers.IndexOfByKey(Lever);
    if (LeverIdx == INDEX_NONE) return;

    if (Order.IsValidIndex(ProgressIndex) && Order[ProgressIndex] == LeverIdx)
    {
        ActivateLever(Lever);
        ++ProgressIndex;
        if (ProgressIndex >= Order.Num())
        {
            CompletePuzzle();
        }
    }
    else
    {
        ResetPuzzle();
    }
}

void ALeverManager::ActivateLever(ALeverActor* Lever)
{
    if (Lever && !Lever->bActivated)
    {
        Lever->bActivated = true;
        Lever->OnRep_Activated();
    }
}

void ALeverManager::ResetPuzzle()
{
    for (ALeverActor* Lever : PuzzleLevers)
    {
        if (Lever)
        {
            Lever->bActivated = false;
            Lever->OnRep_Activated();
        }
    }
    ProgressIndex = 0;
}

void ALeverManager::CompletePuzzle()
{
    bCompleted = true;
    OnRep_PuzzleCompleted();
    OnLeverPuzzleCompleted.Broadcast();

    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }
}

void ALeverManager::OnRep_PuzzleCompleted()
{
    // Add client-side completion VFX if you want!
}

void ALeverManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALeverManager, PuzzleLevers);
    DOREPLIFETIME(ALeverManager, Order);
    DOREPLIFETIME(ALeverManager, ProgressIndex);
    DOREPLIFETIME(ALeverManager, bCompleted);
}