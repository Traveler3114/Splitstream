#include "PresentBridge.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

APresentBridge::APresentBridge()
{
    PrimaryActorTick.bCanEverTick = false; // No need to tick every frame
    bReplicates = true;                    // Enable replication for multiplayer

    // Create a root component to attach tiles to
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void APresentBridge::BeginPlay()
{
    Super::BeginPlay();

    // On server: clear the fall states and create the bridge, also create future bridge (not defined here)
    if (HasAuthority())
    {
        TileFallStates.Empty();
        CreateBridge();
    }
    else {
        CreateBridge();
    }
}

void APresentBridge::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Register TileFallStates for replication
    DOREPLIFETIME(APresentBridge, TileFallStates);
}

void APresentBridge::CreateBridge()
{
    // Set the size of BridgeTiles to total number of tiles
    BridgeTiles.SetNum(NumRows * NumColumns);

    // Loop through each grid coordinate to create tiles
    for (int32 Row = 0; Row < NumRows; ++Row)
    {
        for (int32 Col = 0; Col < NumColumns; ++Col)
        {
            int32 Index = Row * NumColumns + Col;

            // Create a unique name for each tile mesh component
            FString Name = FString::Printf(TEXT("Tile_%d"), Index);

            // Instantiate a new StaticMeshComponent for this tile
            UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this, *Name);
            Tile->RegisterComponent(); // Register so it becomes part of the actor
            Tile->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            Tile->SetStaticMesh(TileMesh); // Set the mesh to your chosen tile mesh

            // Position the tile according to row and column with spacing
            Tile->SetRelativeLocation(FVector(Col * TileSpacing, Row * TileSpacing, 0.f));
            Tile->SetWorldScale3D(BoxScale); // Set collision box scale

            // Enable overlap events and notify on collisions for detecting player stepping
            Tile->SetGenerateOverlapEvents(true);
            Tile->SetNotifyRigidBodyCollision(true);

            // Bind the overlap event to your handler function
            Tile->OnComponentBeginOverlap.AddDynamic(this, &APresentBridge::OnTileOverlap);

            bool bFall = false;

            if (HasAuthority())
            {
                // Server randomly decides if tile can fall
                bFall = FMath::RandBool();

                // Save fall state for replication to clients
                TileFallStates.Add(bFall);
            }
            else if (TileFallStates.IsValidIndex(Index))
            {
                // Clients read the replicated fall state from server
                bFall = TileFallStates[Index];
            }

            // Configure collision and physics based on whether tile can fall
            if (bFall)
            {
                Tile->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
                Tile->SetSimulatePhysics(false); // Initially no physics simulation
            }
            else
            {
                Tile->SetCollisionProfileName(TEXT("BlockAll"));
                Tile->SetSimulatePhysics(false); // Static tile, no physics
            }

            // Store the tile component and fall flag in the bridge tile struct
            BridgeTiles[Index].Mesh = Tile;
            BridgeTiles[Index].bCanFall = bFall;
        }
    }
}

// Called when a tile overlaps with another component (e.g., player steps on it)
void APresentBridge::OnTileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Only proceed if overlapping actor is valid and is a Character (player)
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
        return;

    // Find which tile triggered the overlap
    for (int32 i = 0; i < BridgeTiles.Num(); ++i)
    {
        // Check if this is the tile overlapped and if it can fall
        if (BridgeTiles[i].Mesh == OverlappedComp && BridgeTiles[i].bCanFall)
        {
            if (HasAuthority())
            {
                // Only the server handles fall logic and multicasts to clients
                Server_HandleTileFall(i);
            }
            break;
        }
    }
}

// Server RPC to handle tile fall event
void APresentBridge::Server_HandleTileFall_Implementation(int32 TileIndex)
{
    // Call multicast RPC to update all clients
    Multicast_DropTile(TileIndex);
}

// Multicast RPC implementation to drop the tile on all clients
void APresentBridge::Multicast_DropTile_Implementation(int32 TileIndex)
{
    if (BridgeTiles.IsValidIndex(TileIndex))
    {
        // Make the tile fall by enabling physics and updating collision
        DropTile(BridgeTiles[TileIndex].Mesh);
    }
}

// Drops a tile by enabling physics and setting its collision profile
void APresentBridge::DropTile(UStaticMeshComponent* Tile)
{
    if (Tile)
    {
        Tile->SetSimulatePhysics(true);                // Enable physics simulation so it falls
        Tile->SetCollisionProfileName(TEXT("PhysicsActor")); // Change collision to physics actor
    }
}
