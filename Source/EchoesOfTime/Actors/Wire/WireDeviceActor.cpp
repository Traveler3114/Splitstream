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

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);

	bReplicates = true;
}

void AWireDeviceActor::BeginPlay()
{
    Super::BeginPlay();

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

void AWireDeviceActor::OnWireCut(AWireActor* CutWire)
{
}

void AWireDeviceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireDeviceActor, SpawnLocationName);
}