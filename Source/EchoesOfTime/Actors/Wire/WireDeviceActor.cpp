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
        int32 NumWires = WireRelativeLocations.Num();
        for (int32 i = 0; i < NumWires; ++i)
        {
            if (!WireClass) continue;

            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            FVector SpawnLoc = GetActorLocation();
            FRotator SpawnRot = GetActorRotation();
            AWireActor* NewWire = GetWorld()->SpawnActor<AWireActor>(WireClass, SpawnLoc, SpawnRot, Params);

            if (NewWire)
            {
                NewWire->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepRelativeTransform);

                if (WireRelativeLocations.IsValidIndex(i))
                    NewWire->SetActorRelativeLocation(WireRelativeLocations[i]);
                else
                    NewWire->SetActorRelativeLocation(FVector::ZeroVector);

                NewWire->SetActorRelativeScale3D(WireRelativeScale);
                NewWire->SetActorRelativeRotation(WireRelativeRotation);

                WireActors.Add(NewWire);
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