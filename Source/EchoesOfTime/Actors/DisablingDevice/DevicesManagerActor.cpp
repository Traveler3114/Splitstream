#include "DevicesManagerActor.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
#include "Net/UnrealNetwork.h"

ADevicesManagerActor::ADevicesManagerActor()
{
    bReplicates = true;
}

void ADevicesManagerActor::BeginPlay()
{
    Super::BeginPlay();

    for (ADisablingDeviceActor* Device : Devices)
    {
        if (Device)
			RegisterDevice(Device);
    }
}

void ADevicesManagerActor::RegisterDevice(ADisablingDeviceActor* Device)
{
    if (!Device) return;

    Devices.Add(Device);
    // Bind on server only
    if (HasAuthority())
    {
        Device->OnDeviceStateChanged.RemoveDynamic(this, &ADevicesManagerActor::OnDeviceStateChanged);
        Device->OnDeviceStateChanged.AddDynamic(this, &ADevicesManagerActor::OnDeviceStateChanged);
    }
}

void ADevicesManagerActor::OnDeviceStateChanged(ADisablingDeviceActor* ChangedDevice)
{
    if (!HasAuthority() || bPuzzleCompleted)
        return;

    CheckPuzzleState();
}

void ADevicesManagerActor::CheckPuzzleState()
{
    for (ADisablingDeviceActor* Device : Devices)
    {
        if (Device && Device->bIsActive)
            return; // Puzzle not complete
    }

    bPuzzleCompleted = true;
    OnRep_PuzzleCompleted();
    OnPuzzleCompleted.Broadcast();

    if (CompletionTarget && CompletionTarget->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
    {
        IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(CompletionTarget);
    }
}

void ADevicesManagerActor::OnRep_PuzzleCompleted()
{
    // Play VFX/SFX or notify Blueprints
}

void ADevicesManagerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADevicesManagerActor, Devices);
    DOREPLIFETIME(ADevicesManagerActor, bPuzzleCompleted);
}