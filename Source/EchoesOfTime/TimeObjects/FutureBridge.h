#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bridge.h" // Required for ABridge forward access
#include "FutureBridge.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureBridge : public AActor
{
	GENERATED_BODY()

public:
	AFutureBridge();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Bridge")
	TSoftObjectPtr<class ABridge> PresentObject;

	UPROPERTY(EditAnywhere, Category = "Bridge")
	UStaticMesh* TileMesh;

	UPROPERTY(EditAnywhere, Category = "Bridge")
	FVector BoxScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Bridge")
	float TileSpacing = 100.f;

	UPROPERTY(EditAnywhere, Category = "Bridge")
	int32 NumRows = 5;

	UPROPERTY(EditAnywhere, Category = "Bridge")
	int32 NumColumns = 5;

public:
	// Mirrors the tile layout from PresentObject
	void MirrorBridge(ABridge* SourceBridge);

	UFUNCTION()
	void DelayedMirror();

	UPROPERTY()
	TArray<UStaticMeshComponent*> BridgeTiles;
};
