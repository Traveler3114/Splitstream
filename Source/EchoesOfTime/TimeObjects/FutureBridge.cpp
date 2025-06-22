#include "FutureBridge.h"
#include "PresentBridge.h"
#include "Components/StaticMeshComponent.h"

AFutureBridge::AFutureBridge()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFutureBridge::BeginPlay()
{
	Super::BeginPlay();

	if (PresentObject.IsValid())
	{
		// Delay MirrorBridge to allow PresentObject to finish initialization
		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, this, &AFutureBridge::DelayedMirror, 0.2f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FutureBridge: PresentObject is not set or not loaded."));
	}
}

void AFutureBridge::DelayedMirror()
{
	if (PresentObject.IsValid())
	{
		MirrorBridge(PresentObject.Get());
	}
}


void AFutureBridge::MirrorBridge(APresentBridge* SourceBridge)
{
	if (!SourceBridge) return;

	NumRows = SourceBridge->NumRows;
	NumColumns = SourceBridge->NumColumns;
	TileSpacing = SourceBridge->TileSpacing;
	TileMesh = SourceBridge->TileMesh;
	BoxScale = SourceBridge->BoxScale;

	const TArray<bool>& FallStates = SourceBridge->TileFallStates;

	for (int32 Row = 0; Row < NumRows; ++Row)
	{
		for (int32 Col = 0; Col < NumColumns; ++Col)
		{
			int32 Index = Row * NumColumns + Col;

			// Skip falling tiles
			if (!FallStates.IsValidIndex(Index) || FallStates[Index])
				continue;

			FString Name = FString::Printf(TEXT("FutureTile_%d"), Index);

			UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this, *Name);
			if (Tile)
			{
				Tile->RegisterComponent();
				Tile->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				Tile->SetStaticMesh(TileMesh);
				Tile->SetRelativeLocation(FVector(Col * TileSpacing, Row * TileSpacing, 5.f)); // Slight Z offset
				Tile->SetWorldScale3D(BoxScale);
				Tile->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // Visual only
			}
			BridgeTiles.Add(Tile);
		}
	}
}
