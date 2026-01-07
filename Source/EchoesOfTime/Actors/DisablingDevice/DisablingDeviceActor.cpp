#include "DisablingDeviceActor.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"
#include "Interfaces/IPuzzleCompletionReceiver.h"
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

void ADisablingDeviceActor::SetIsSolo(bool bSolo)
{
    bIsSolo = bSolo;

    if (DeviceMesh)
    {
        // --- Slot 0: IconBackgroundColour ---
        UMaterialInstanceDynamic* IconMatInstance = DeviceMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (IconMatInstance)
        {
            FLinearColor UseIconBGColor = bIsSolo ? SoloBaseColor : ManagerBaseColor;
            IconMatInstance->SetVectorParameterValue(FName("IconBackgroundColour"), UseIconBGColor);
        }

        // --- Slot 1: Base Color ---
        UMaterialInstanceDynamic* BoxMatInstance = DeviceMesh->CreateAndSetMaterialInstanceDynamic(1);
        if (BoxMatInstance)
        {
            FLinearColor UseBaseColor = bIsSolo ? SoloBaseColor : ManagerBaseColor;
            BoxMatInstance->SetVectorParameterValue(FName("Base Color"), UseBaseColor);
        }
    }
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
    SetHighlighted_Implementation(false);
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
        for (AActor* Target : CompletionTargets)
        {
            if (Target && Target->GetClass()->ImplementsInterface(UPuzzleCompletionReceiver::StaticClass()))
            {
                IPuzzleCompletionReceiver::Execute_OnPuzzleCompleted(Target);
            }
        }
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

bool ADisablingDeviceActor::IsProgressiveInteract_Implementation()
{
    return true;
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