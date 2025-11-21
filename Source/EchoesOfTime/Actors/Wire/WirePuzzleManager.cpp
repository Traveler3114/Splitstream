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
        FString WireOrderString;
        FString WireColorString;
        // Obtain order string and color string from generator
        for (TActorIterator<AProceduralLevelGenerator> It(GetWorld()); It; ++It)
        {
            WireOrderString = It->PastWireDeviceOrderString;
            WireColorString = It->PastWireCorrectColorString;
            break; // Only need one generator
        }
        TArray<FString> OrderStrs;
        WireOrderString.ParseIntoArray(OrderStrs, TEXT(","));
        DeviceOrder.SetNum(OrderStrs.Num());
        for (int32 i = 0; i < OrderStrs.Num(); ++i)
        {
            DeviceOrder[i] = FCString::Atoi(*OrderStrs[i]);
        }

        // Parse colors
        TArray<FString> ColorStrs;
        WireColorString.ParseIntoArray(ColorStrs, TEXT(","));
        CorrectWireColors.SetNum(ColorStrs.Num());
        for (int32 i = 0; i < ColorStrs.Num(); ++i)
        {
            FString ColorStr = ColorStrs[i].Replace(TEXT("EWireColor::"), TEXT(""));
            if (ColorStr == TEXT("Red")) CorrectWireColors[i] = EWireColor::Red;
            else if (ColorStr == TEXT("Green")) CorrectWireColors[i] = EWireColor::Green;
            else if (ColorStr == TEXT("Blue")) CorrectWireColors[i] = EWireColor::Blue;
            else if (ColorStr == TEXT("Yellow")) CorrectWireColors[i] = EWireColor::Yellow;
            else CorrectWireColors[i] = EWireColor::Red; // fallback
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