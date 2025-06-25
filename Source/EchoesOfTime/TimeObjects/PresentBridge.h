#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PresentBridge.generated.h"

// Struct representing each tile of the bridge
USTRUCT()
struct FBridgeTile
{
    GENERATED_BODY()

    // Mesh component representing the tile visually and physically
    UPROPERTY()
    UStaticMeshComponent* Mesh = nullptr;

    // Flag indicating if this tile can fall when stepped on
    UPROPERTY()
    bool bCanFall = false;
};

UCLASS()
class ECHOESOFTIME_API APresentBridge : public AActor
{
    GENERATED_BODY()

public:
    // Constructor
    APresentBridge();

protected:
    // Called when the game starts or the actor is spawned
    virtual void BeginPlay() override;

    // Overlap event handler for when something overlaps a tile
    UFUNCTION()
    void OnTileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // Server RPC to handle tile falling logic securely on the server
    UFUNCTION(Server, Reliable)
    void Server_HandleTileFall(int32 TileIndex);
    void Server_HandleTileFall_Implementation(int32 TileIndex);

    // Multicast RPC to inform all clients that a tile should drop
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DropTile(int32 TileIndex);
    void Multicast_DropTile_Implementation(int32 TileIndex);

    // Helper function to apply physics and collision changes to a tile to make it fall
    void DropTile(UStaticMeshComponent* Tile);

    // Initializes and creates the bridge tiles grid
    void CreateBridge();

    // Setup properties that need to be replicated across network
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // Bridge configuration parameters editable in the editor

    // Number of rows in the bridge grid
    UPROPERTY(EditAnywhere, Category = "Bridge")
    int32 NumRows = 5;

    // Number of columns in the bridge grid
    UPROPERTY(EditAnywhere, Category = "Bridge")
    int32 NumColumns = 5;

    // Distance between tiles
    UPROPERTY(EditAnywhere, Category = "Bridge")
    float TileSpacingX = 150.f;

    // Distance between tiles
    UPROPERTY(EditAnywhere, Category = "Bridge")
    float TileSpacingY = 150.f;

    // Scale of each tile's collision box
    UPROPERTY(EditAnywhere, Category = "Bridge")
    FVector BoxScale = FVector(1.f, 1.f, 0.25f);

    // Mesh to use for each tile
    UPROPERTY(EditAnywhere, Category = "Bridge")
    UStaticMesh* TileMesh;

    UPROPERTY(EditAnywhere, Category = "Bridge")
    float FallForceMultiplier = 50000.f; // Expose to editor

    // Array replicated across network to keep track of which tiles can fall
    UPROPERTY(Replicated)
    TArray<bool> TileFallStates;

private:
    // Internal storage of bridge tiles with mesh and fall state
    UPROPERTY()
    TArray<FBridgeTile> BridgeTiles;

};
