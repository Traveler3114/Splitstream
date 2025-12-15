#include "WirePuzzleManager.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "ProceduralLevelGenerator.h"
#include "Engine/Engine.h"
#include "WireActor.h"
#include "WireDeviceActor.h"
#include "EngineUtils.h"
#include "GameStates/DefaultGameState.h"

AWirePuzzleManager::AWirePuzzleManager()
{
    bReplicates = true;
}

void AWirePuzzleManager::BeginPlay()
{
    Super::BeginPlay();
	SetupPuzzle();
}

void AWirePuzzleManager::SetupPuzzle()
{


    if (HasAuthority())
    {
        CompletedColors.Empty();
        bCompleted = false;
        ProgressIndex = 0;
        // Obtain wire sequence steps (location + color per device) from the generator
        TArray<FWireSequenceStep> DeviceSequence;
        for (TActorIterator<AProceduralLevelGenerator> It(GetWorld()); It; ++It)
        {
            DeviceSequence = It->PastWireDeviceSequence;
            break; // Only need one generator
        }

        RequiredColors.Empty();

        for (int32 i = 0; i < DeviceSequence.Num(); ++i)
        {
            int32 DeviceIdx = INDEX_NONE;
            for (int32 d = 0; d < PuzzleDevices.Num(); ++d)
            {
                if (PuzzleDevices[d] &&
                    PuzzleDevices[d]->SpawnLocationName == DeviceSequence[i].DeviceLocation)
                {
                    DeviceIdx = d;
                    break;
                }
            }

            RequiredColors.Add(DeviceSequence[i].WireColor);

            if (DeviceIdx == INDEX_NONE)
            {
            }
        }

        CompletedColors.Empty();
        bCompleted = false;

        // Bind to wires as before
        for (AWireDeviceActor* Device : PuzzleDevices)
        {
            if (Device)
            {
                for (AWireActor* Wire : Device->WireActors)
                {
                    if (Wire)
                    {
                        Wire->OnWireCut.RemoveDynamic(this, &AWirePuzzleManager::OnWireCut);
                        Wire->OnWireCut.AddDynamic(this, &AWirePuzzleManager::OnWireCut);
                    }
                }
            }
        }
    }
	//HighlightNextCorrectWire();
}

// void AWirePuzzleManager::HighlightNextCorrectWire()
// {
//     // Remove highlight from all wires first
//     for (AWireDeviceActor* Device : PuzzleDevices)
//     {
//         if (Device)
//         {
//             for (AWireActor* Wire : Device->WireActors)
//             {
//                 if (Wire)
//                     Wire->SetHighlighted_Implementation(false);
//             }
//         }
//     }

//     // Safety checks for completion and order arrays
//     if (bCompleted || !DeviceOrder.IsValidIndex(ProgressIndex) || !CorrectWireColors.IsValidIndex(ProgressIndex))
//         return;

//     int32 NextDeviceIdx = DeviceOrder[ProgressIndex];
//     EWireColor ExpectedColor = CorrectWireColors[ProgressIndex];

//     // Make sure index is valid
//     if (NextDeviceIdx == INDEX_NONE || !PuzzleDevices.IsValidIndex(NextDeviceIdx))
//         return;

//     AWireDeviceActor* NextDevice = PuzzleDevices[NextDeviceIdx];
//     if (!NextDevice)
//         return;

//     for (AWireActor* Wire : NextDevice->WireActors)
//     {
//         if (Wire && Wire->WireColor == ExpectedColor && !Wire->bIsCut)
//         {
//             Wire->SetHighlighted_Implementation(true);
//             break; // Only highlight one wire
//         }
//     }
// }

void AWirePuzzleManager::OnWireCut(AWireActor* CutWire)
{
    if (!HasAuthority() || bCompleted || !CutWire) return;

    const EWireColor CutColor = CutWire->WireColor;

    // --- ENFORCE ORDER ---
    // 1: Is there a next wire required? 
    if (!RequiredColors.IsValidIndex(ProgressIndex))
        return; // Out of bounds, probably already finished

    EWireColor ExpectedColor = RequiredColors[ProgressIndex];

    if (CutColor != ExpectedColor)
    {
        // Wrong color cut => Alarm!
        ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr;
        if (GS)
        {
            GS->StartAlarm();
        }
        return;
    }

    // If correct, progress
    CompletedColors.Add(CutColor);
    ProgressIndex++;

    // Complete if we're at the end
    if (ProgressIndex >= RequiredColors.Num())
    {
        CompletePuzzle();
    }
}
void AWirePuzzleManager::ResetPuzzle()
{
    CompletedColors.Empty();
    bCompleted = false;
    ProgressIndex = 0;
    for (auto* Device : PuzzleDevices)
    {
        if (Device)
        {
            for (auto* Wire : Device->WireActors)
            {
                Wire->bIsCut = false;
                Wire->OnRep_CutState();
            }
        }
    }

    CompletedColors.Empty();
    bCompleted = false;
}

void AWirePuzzleManager::CompletePuzzle()
{
    bCompleted = true;
    OnRep_PuzzleCompleted();
    OnWirePuzzleCompleted.Broadcast();

    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }
}

void AWirePuzzleManager::OnRep_PuzzleCompleted()
{

}

int32 AWirePuzzleManager::GetDeviceIndexForWire(AWireActor* Wire) const
{
    for (int32 i = 0; i < PuzzleDevices.Num(); ++i)
    {
        auto* Device = PuzzleDevices[i];
        if (Device && Device->WireActors.Contains(Wire))
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void AWirePuzzleManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWirePuzzleManager, PuzzleDevices);
    DOREPLIFETIME(AWirePuzzleManager, bCompleted);
    DOREPLIFETIME(AWirePuzzleManager, RequiredColors);
    DOREPLIFETIME(AWirePuzzleManager, CompletedColors);
    DOREPLIFETIME(AWirePuzzleManager, ProgressIndex);
}