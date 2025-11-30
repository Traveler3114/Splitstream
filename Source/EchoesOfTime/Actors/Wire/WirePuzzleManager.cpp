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
    if (HasAuthority())
    {
        // Obtain wire sequence steps from the generator (location + color per step)
        TArray<FWireSequenceStep> DeviceSequence;
        for (TActorIterator<AProceduralLevelGenerator> It(GetWorld()); It; ++It)
        {
            DeviceSequence = It->PastWireDeviceSequence;
            break; // Only need one generator
        }

        // Sync arrays; add robust logging if device mapping fails!
        DeviceOrder.SetNum(DeviceSequence.Num());
        CorrectWireColors.SetNum(DeviceSequence.Num());
        for (int32 i = 0; i < DeviceSequence.Num(); ++i)
        {
            int32 DeviceIdx = INDEX_NONE;
            for (int32 d = 0; d < PuzzleDevices.Num(); ++d)
            {
                if (PuzzleDevices[d] && PuzzleDevices[d]->SpawnLocationName == DeviceSequence[i].DeviceLocation)
                {
                    DeviceIdx = d;
                    break;
                }
            }
            DeviceOrder[i] = DeviceIdx;
            CorrectWireColors[i] = DeviceSequence[i].WireColor;

            if (DeviceIdx == INDEX_NONE)
            {
                UE_LOG(LogTemp, Error, TEXT("WirePuzzleManager: Device for sequence location '%s' not found! This step will FAIL!"), *DeviceSequence[i].DeviceLocation);
            }
        }

        ProgressIndex = 0;
        bCompleted = false;
        for (AWireDeviceActor* Device : PuzzleDevices)
        {
            if (Device)
            {
                for (AWireActor* Wire : Device->WireActors)
                {
                    if (Wire)
                        Wire->OnWireCut.AddDynamic(this, &AWirePuzzleManager::OnWireCut);
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

    int32 DeviceIdx = GetDeviceIndexForWire(CutWire);

    // Check if sequence is valid for current ProgressIndex
    if (!DeviceOrder.IsValidIndex(ProgressIndex) || !CorrectWireColors.IsValidIndex(ProgressIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("WirePuzzleManager: Sequence arrays out of bounds!"));
        return;
    }

    int32 ExpectedDeviceIdx = DeviceOrder[ProgressIndex];
    EWireColor ExpectedColor = CorrectWireColors[ProgressIndex];

    if (ExpectedDeviceIdx == INDEX_NONE || !PuzzleDevices.IsValidIndex(ExpectedDeviceIdx))
    {
        UE_LOG(LogTemp, Error, TEXT("WirePuzzleManager: Sequence is broken! Device step points to INDEX_NONE or invalid device."));
        // Optionally: alarm or skip, but safest is to do nothing
        return;
    }

    // Evaluate correct wire sequence
    if (DeviceIdx == ExpectedDeviceIdx &&
        CutWire->WireColor == ExpectedColor)
    {
        // CORRECT cut: advance the puzzle
        ++ProgressIndex;
        //HighlightNextCorrectWire();

        if (ProgressIndex >= DeviceOrder.Num())
        {
            CompletePuzzle();
        }
    }
    else
    {
        // Incorrect wire/device: start alarm!
        ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr;
        if (GS) GS->StartAlarm();

        //HighlightNextCorrectWire();
    }
}

void AWirePuzzleManager::ResetPuzzle()
{
    //HighlightNextCorrectWire();
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
    ProgressIndex = 0;
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
    DOREPLIFETIME(AWirePuzzleManager, DeviceOrder);
    DOREPLIFETIME(AWirePuzzleManager, CorrectWireColors);
    DOREPLIFETIME(AWirePuzzleManager, ProgressIndex);
    DOREPLIFETIME(AWirePuzzleManager, bCompleted);
}