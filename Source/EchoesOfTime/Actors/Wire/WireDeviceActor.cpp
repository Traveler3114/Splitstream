// WireDeviceActor.cpp
#include "WireDeviceActor.h"
#include "WireActor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AWireDeviceActor::AWireDeviceActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DeviceMesh"));
    DeviceMesh->SetupAttachment(SceneRoot);

    bReplicates = true;
}

// WireDeviceActor.cpp

void AWireDeviceActor::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        WireActors.Empty();
        TArray<UChildActorComponent*> ChildActorComponents;
        GetComponents(ChildActorComponents);

        for (UChildActorComponent* CAC : ChildActorComponents)
        {
            if (AWireActor* Wire = Cast<AWireActor>(CAC->GetChildActor()))
            {
                WireActors.Add(Wire);
            }
        }
    }
}



void AWireDeviceActor::ApplyColorConfiguration(const FWireDeviceColorConfig& Config)
{
    if (HasAuthority() && WireActors.Num() >= 2)
    {
        // Force update trick
        if (WireActors[0]->WireColor == Config.WireColorA)
            WireActors[0]->WireColor = EWireColor::None;
        WireActors[0]->WireColor = Config.WireColorA;
        WireActors[0]->ApplyWireColor(); // <--- Add this

        if (WireActors[1]->WireColor == Config.WireColorB)
            WireActors[1]->WireColor = EWireColor::None;
        WireActors[1]->WireColor = Config.WireColorB;
        WireActors[1]->ApplyWireColor(); // <--- Add this
    }
}

void AWireDeviceActor::OnWireCut(AWireActor* CutWire)
{
    // implement logic if needed
}

void AWireDeviceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWireDeviceActor, WireActors);
    DOREPLIFETIME(AWireDeviceActor, SpawnLocationName);
}