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

//void AWirePuzzleManager::HighlightNextCorrectWire()
//{
//   // Remove highlight from all wires first
//   for (auto* Device : PuzzleDevices)
//   {
//       if (Device)
//       {
//           for (auto* Wire : Device->WireActors)
//           {
//               if (Wire)
//                   Wire->SetHighlighted_Implementation(false);
//           }
//       }
//   }
//   // Highlight the next correct wire (if not completed)
//   if (!bCompleted && DeviceOrder.IsValidIndex(ProgressIndex) && CorrectWireColors.IsValidIndex(DeviceOrder[ProgressIndex]))
//   {
//       int32 NextIdx = DeviceOrder[ProgressIndex];
//       AWireDeviceActor* NextDevice = PuzzleDevices.IsValidIndex(NextIdx) ? PuzzleDevices[NextIdx] : nullptr;
//       if (NextDevice)
//       {
//           for (AWireActor* Wire : NextDevice->WireActors)
//           {
//               if (Wire && Wire->WireColor == CorrectWireColors[NextIdx] && !Wire->bIsCut)
//               {
//                   Wire->SetHighlighted_Implementation(true);
//                   break; // Only highlight one wire
//               }
//           }
//       }
//   }
//}

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

        // Sync legacy arrays for existing code compatibility
        DeviceOrder.SetNum(DeviceSequence.Num());
        CorrectWireColors.SetNum(DeviceSequence.Num());
        for (int32 i = 0; i < DeviceSequence.Num(); ++i)
        {
            // Find index of device in PuzzleDevices by match of location name
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


void AWirePuzzleManager::OnWireCut(AWireActor* CutWire)
{
    if (!HasAuthority() || bCompleted || !CutWire) return;

    int32 DeviceIdx = GetDeviceIndexForWire(CutWire);
    int32 ExpectedDeviceIdx = DeviceOrder.IsValidIndex(ProgressIndex) ? DeviceOrder[ProgressIndex] : -1;

    // Evaluate correct wire sequence
    if (DeviceIdx == ExpectedDeviceIdx &&
        CorrectWireColors.IsValidIndex(DeviceIdx) &&
        CutWire->WireColor == CorrectWireColors[DeviceIdx])
    {
        /*if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("Wire puzzle: Correct wire cut on device %d -- %s"),
                    DeviceIdx + 1,
                    *UEnum::GetValueAsString(CutWire->WireColor)));*/
        ++ProgressIndex;
        if (ProgressIndex >= DeviceOrder.Num())
        {
            //if (GEngine)
            //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Wire puzzle COMPLETED!"));
            CompletePuzzle();
        }
    }
    else
    {
        //FString debugMsg;
        //if (DeviceIdx != ExpectedDeviceIdx)
        //    debugMsg = FString::Printf(TEXT("Wire puzzle: Wrong device! Expected Device %d"), ExpectedDeviceIdx + 1);
        //else
        //    debugMsg = FString::Printf(TEXT("Wire puzzle: Wrong wire on device %d! Expected: %s"),
        //        DeviceIdx + 1, *UEnum::GetValueAsString(CorrectWireColors[DeviceIdx]));
        //if (GEngine)
        //    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, debugMsg);

        // Trigger alarm
        ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr;
        if (GS) GS->StartAlarm();

        // Optionally reset puzzle, comment if you don't want auto-reset:
        // ResetPuzzle();
    }
    //HighlightNextCorrectWire();
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