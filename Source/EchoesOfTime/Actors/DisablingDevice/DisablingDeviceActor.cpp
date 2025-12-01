#include "DisablingDeviceActor.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ArrowComponent.h"

ADisablingDeviceActor::ADisablingDeviceActor()
{
    bReplicates = true;

    // Create and set root component
    SceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRootComp);

    // Create the device mesh and attach to root
    DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
    DeviceMesh->SetupAttachment(SceneRootComp);

	SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
	SearchComponent->SetIsReplicated(true);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);
}

void ADisablingDeviceActor::BeginPlay()
{
    Super::BeginPlay();

    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &ADisablingDeviceActor::OnSearchComplete);
    }
}

void ADisablingDeviceActor::OnSearchComplete()
{
    DisableDevice();
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
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void ADisablingDeviceActor::CancelInteract_Implementation(AActor* Interactor)
{
    if(SearchComponent)
		SearchComponent->CancelInteract(Interactor);
}

void ADisablingDeviceActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (DeviceMesh && SearchComponent)
    {
        if (SearchComponent->bSearched)
        {
            DeviceMesh->SetRenderCustomDepth(false);
            DeviceMesh->CustomDepthStencilValue = 0;
            return;
        }
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
    DOREPLIFETIME(ADisablingDeviceActor, SpawnLocationName);
}