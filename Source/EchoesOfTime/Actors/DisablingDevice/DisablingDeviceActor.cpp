#include "DisablingDeviceActor.h"
#include "Net/UnrealNetwork.h"

ADisablingDeviceActor::ADisablingDeviceActor()
{
    bReplicates = true;

    // Create and set root component
    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRootComp);

    // Create the device mesh and attach to root
    DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
    DeviceMesh->SetupAttachment(SceneRootComp);
}

void ADisablingDeviceActor::BeginPlay()
{
    Super::BeginPlay();
}

void ADisablingDeviceActor::DisableDevice()
{
    if (!bIsActive || !HasAuthority()) return;

    bIsActive = false;
    OnRep_DeviceState();
    OnDeviceStateChanged.Broadcast(this);

    // Solo completion logic:
    if (bIsSolo)
    {
        OnSoloDeviceDisabled.Broadcast();
        // You can add more solo logic here (VFX/SFX, Blueprint events)
    }
}

void ADisablingDeviceActor::Interact_Implementation(AActor* Interactor)
{
    DisableDevice();
}

void ADisablingDeviceActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (DeviceMesh)
    {
        DeviceMesh->SetRenderCustomDepth(bHighlight);
        DeviceMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void ADisablingDeviceActor::OnRep_DeviceState()
{
    // Play VFX/SFX as needed
}

void ADisablingDeviceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ADisablingDeviceActor, bIsActive);
    DOREPLIFETIME(ADisablingDeviceActor, bIsSolo);
}