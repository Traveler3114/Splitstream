#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bridge.generated.h"

USTRUCT()
struct FBridgeTile
{
    GENERATED_BODY()

    UPROPERTY()
    UStaticMeshComponent* Mesh = nullptr;

    UPROPERTY()
    bool bCanFall = false;
};

UCLASS()
class ECHOESOFTIME_API ABridge : public AActor
{
    GENERATED_BODY()

public:
    ABridge();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(Server, Reliable)
    void Server_HandleTileFall(int32 TileIndex);
    void Server_HandleTileFall_Implementation(int32 TileIndex);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DropTile(int32 TileIndex);
    void Multicast_DropTile_Implementation(int32 TileIndex);

    void DropTile(UStaticMeshComponent* Tile);

    void CreateBridge();
    void CreateFutureBridge();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UPROPERTY(EditAnywhere, Category = "Bridge")
    int32 NumRows = 5;

    UPROPERTY(EditAnywhere, Category = "Bridge")
    int32 NumColumns = 5;

    UPROPERTY(EditAnywhere, Category = "Bridge")
    float TileSpacing = 150.f;

    UPROPERTY(EditAnywhere, Category = "Bridge")
    FVector BoxScale = FVector(1.f, 1.f, 0.25f);

    UPROPERTY(EditAnywhere, Category = "Bridge")
    UStaticMesh* TileMesh;

    // Replicated array storing which tiles fall
    UPROPERTY(Replicated)
    TArray<bool> TileFallStates;

    UPROPERTY(EditAnywhere, Category = "Bridge")
    FVector FutureStartLocation = FVector(-20260.000000, 2300.000000, 400.000000);

private:
    UPROPERTY()
    TArray<FBridgeTile> BridgeTiles;

    UPROPERTY()
    TArray<UStaticMeshComponent*> FutureBridgeTiles;
};
