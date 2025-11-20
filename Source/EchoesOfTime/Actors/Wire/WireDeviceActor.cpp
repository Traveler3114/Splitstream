#include "WireDeviceActor.h"
#include "WireActor.h"
#include "Engine/World.h"

AWireDeviceActor::AWireDeviceActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
    DeviceMesh->SetupAttachment(SceneRoot);
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
    UE_LOG(LogTemp, Log, TEXT("Standalone WireDeviceActor: Wire cut %s"), *UEnum::GetValueAsString(CutWire->WireColor));
}