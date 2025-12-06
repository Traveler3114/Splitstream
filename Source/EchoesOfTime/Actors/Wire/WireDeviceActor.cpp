#include "WireDeviceActor.h"
#include "WireActor.h"
#include "Engine/World.h"
#include "Components/ArrowComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AWireDeviceActor::AWireDeviceActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
    DeviceMesh->SetupAttachment(SceneRoot);

    SearchComponent = CreateDefaultSubobject<USearchComponent>(TEXT("SearchComponent"));
    SearchComponent->SetIsReplicated(true);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);

	bReplicates = true;
}

void AWireDeviceActor::BeginPlay()
{
    Super::BeginPlay();

    if (SearchComponent)
    {
        SearchComponent->OnSearchComplete.AddDynamic(this, &AWireDeviceActor::OnSearchComplete);
    }

    TArray<UChildActorComponent*> children;
    GetComponents(children);
    for (auto* Comp : children)
    {
        AWireActor* Wire = Cast<AWireActor>(Comp->GetChildActor());
        if (Wire)
        {
            Wire->OnWireCut.AddDynamic(this, &AWireDeviceActor::OnWireCut);
            WireActors.Add(Wire);
        }
    }
}

void AWireDeviceActor::Interact_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->Interact(Interactor);
}

void AWireDeviceActor::CancelInteract_Implementation(AActor* Interactor)
{
    if (SearchComponent)
        SearchComponent->CancelInteract(Interactor);
}

void AWireDeviceActor::SetHighlighted_Implementation(bool bHighlight)
{
    if (DeviceMesh)
    {
        DeviceMesh->SetRenderCustomDepth(bHighlight);
        DeviceMesh->CustomDepthStencilValue = bHighlight ? 1 : 0;
    }
}

void AWireDeviceActor::OnSearchComplete()
{
    if (HasAuthority()) MulticastPlayWireSound();
}

void AWireDeviceActor::MulticastPlayWireSound_Implementation()
{
    if (WireDeviceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WireDeviceSound, GetActorLocation());
    }
}

void AWireDeviceActor::OnWireCut(AWireActor* CutWire)
{
    UE_LOG(LogTemp, Log, TEXT("Standalone WireDeviceActor: Wire cut %s"), *UEnum::GetValueAsString(CutWire->WireColor));
}

void AWireDeviceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireDeviceActor, SpawnLocationName);
}