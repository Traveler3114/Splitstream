#include "WirePuzzleManager.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "Engine/Engine.h"
#include "WireActor.h"
#include "WireDeviceActor.h"
#include "GameStates/DefaultGameState.h"

AWirePuzzleManager::AWirePuzzleManager()
{
    bReplicates = true;
}

void AWirePuzzleManager::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
        SetupPuzzle();
}

void AWirePuzzleManager::SetupPuzzle()
{
    if (!HasAuthority()) return;
    ProgressIndex = 0;
    bCompleted = false;

    DeviceOrder.Empty();
    for (int32 i = 0; i < PuzzleDevices.Num(); ++i)
        DeviceOrder.Add(i);
    for (int32 i = DeviceOrder.Num() - 1; i > 0; --i)
        DeviceOrder.Swap(i, FMath::RandRange(0, i));

    CorrectWireColors.SetNum(PuzzleDevices.Num());
    for (int32 i = 0; i < PuzzleDevices.Num(); ++i)
    {
        auto* Device = PuzzleDevices[i];
        if (Device && Device->WireActors.Num() > 0)
        {
            int32 WireIdx = FMath::RandRange(0, Device->WireActors.Num() - 1);
            CorrectWireColors[i] = Device->WireActors[WireIdx]->WireColor;
        }
        else
        {
            CorrectWireColors[i] = EWireColor::Red;
        }
    }

    for (int32 i = 0; i < PuzzleDevices.Num(); ++i)
    {
        auto* Device = PuzzleDevices[i];
        if (Device)
        {
            for (AWireActor* Wire : Device->WireActors)
            {
                if (Wire)
                {
                    Wire->OnWireCut.AddDynamic(this, &AWirePuzzleManager::OnWireCut);
                }
            }
        }
    }
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
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
                FString::Printf(TEXT("Wire puzzle: Correct wire cut on device %d -- %s"),
                    DeviceIdx + 1,
                    *UEnum::GetValueAsString(CutWire->WireColor)));
        ++ProgressIndex;
        if (ProgressIndex >= DeviceOrder.Num())
        {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Wire puzzle COMPLETED!"));
            CompletePuzzle();
        }
    }
    else
    {
        FString debugMsg;
        if (DeviceIdx != ExpectedDeviceIdx)
            debugMsg = FString::Printf(TEXT("Wire puzzle: Wrong device! Expected Device %d"), ExpectedDeviceIdx + 1);
        else
            debugMsg = FString::Printf(TEXT("Wire puzzle: Wrong wire on device %d! Expected: %s"),
                DeviceIdx + 1, *UEnum::GetValueAsString(CorrectWireColors[DeviceIdx]));
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, debugMsg);

        // Trigger alarm
        ADefaultGameState* GS = GetWorld() ? GetWorld()->GetGameState<ADefaultGameState>() : nullptr;
        if (GS) GS->StartAlarm();

        // Optionally reset puzzle, comment if you don't want auto-reset:
        // ResetPuzzle();
    }
}

void AWirePuzzleManager::ResetPuzzle()
{
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
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Wire puzzle RESET!"));
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
    // Optionally play VFX or sound
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, TEXT("Wire puzzle completed [client]!"));
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