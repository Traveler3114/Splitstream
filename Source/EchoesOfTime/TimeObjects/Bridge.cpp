#include "Bridge.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ABridge::ABridge()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABridge::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        TileFallStates.Empty();
        CreateBridge();
        CreateFutureBridge(); // <- Add this
    }
    else
    {
        CreateBridge();       // For replication
        CreateFutureBridge(); // <- Add this
    }
}

void ABridge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABridge, TileFallStates);
}

void ABridge::CreateBridge()
{
    BridgeTiles.SetNum(NumRows * NumColumns);

    for (int32 Row = 0; Row < NumRows; ++Row)
    {
        for (int32 Col = 0; Col < NumColumns; ++Col)
        {
            int32 Index = Row * NumColumns + Col;

            FString Name = FString::Printf(TEXT("Tile_%d"), Index);
            UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this, *Name);
            Tile->RegisterComponent();
            Tile->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            Tile->SetStaticMesh(TileMesh);
            Tile->SetRelativeLocation(FVector(Col * TileSpacing, Row * TileSpacing, 0.f));
            Tile->SetWorldScale3D(BoxScale);
            Tile->SetGenerateOverlapEvents(true);
            Tile->SetNotifyRigidBodyCollision(true);

            Tile->OnComponentBeginOverlap.AddDynamic(this, &ABridge::OnTileOverlap);

            bool bFall = false;
            if (HasAuthority())
            {
                bFall = FMath::RandBool();
                TileFallStates.Add(bFall);
            }
            else if (TileFallStates.IsValidIndex(Index))
            {
                bFall = TileFallStates[Index];
            }

            if (bFall)
            {
                Tile->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
                Tile->SetSimulatePhysics(false);
            }
            else
            {
                Tile->SetCollisionProfileName(TEXT("BlockAll"));
                Tile->SetSimulatePhysics(false);
            }

            BridgeTiles[Index].Mesh = Tile;
            BridgeTiles[Index].bCanFall = bFall;
        }
    }
}

void ABridge::CreateFutureBridge()
{
    FutureBridgeTiles.Empty(); // Clear existing if re-creating

    for (int32 Row = 0; Row < NumRows; ++Row)
    {
        for (int32 Col = 0; Col < NumColumns; ++Col)
        {
            int32 Index = Row * NumColumns + Col;

            if (!TileFallStates.IsValidIndex(Index) || TileFallStates[Index])
            {
                continue; // Skip falling tiles
            }

            FString Name = FString::Printf(TEXT("FutureTile_%d"), Index);
            UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this, *Name);

            if (!Tile) continue;

            Tile->RegisterComponent();
            Tile->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
            Tile->SetStaticMesh(TileMesh);
            Tile->SetWorldLocation(FutureStartLocation + FVector(Col * TileSpacing, Row * TileSpacing, 0.f));
            Tile->SetWorldScale3D(BoxScale);
            Tile->SetCollisionProfileName(TEXT("BlockAllDynamic"));
            Tile->SetSimulatePhysics(false);

            // Keep a reference so it's not garbage collected
            FutureBridgeTiles.Add(Tile);
        }
    }
}



void ABridge::OnTileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
        return;

    for (int32 i = 0; i < BridgeTiles.Num(); ++i)
    {
        if (BridgeTiles[i].Mesh == OverlappedComp && BridgeTiles[i].bCanFall)
        {
            if (HasAuthority())
            {
                Server_HandleTileFall(i);
            }
            break;
        }
    }
}

void ABridge::Server_HandleTileFall_Implementation(int32 TileIndex)
{
    Multicast_DropTile(TileIndex);
}

void ABridge::Multicast_DropTile_Implementation(int32 TileIndex)
{
    if (BridgeTiles.IsValidIndex(TileIndex))
    {
        DropTile(BridgeTiles[TileIndex].Mesh);
    }
}

void ABridge::DropTile(UStaticMeshComponent* Tile)
{
    if (Tile)
    {
        Tile->SetSimulatePhysics(true);
        Tile->SetCollisionProfileName(TEXT("PhysicsActor"));
    }
}
