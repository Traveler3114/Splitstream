#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PresentBridge.h" // Required for ABridge forward access
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
	TSoftObjectPtr<class APresentBridge> PresentObject;

public:
	// Mirrors the tile layout from PresentObject
	void MirrorBridge(APresentBridge* SourceBridge);

	UFUNCTION()
	void DelayedMirror();

	UPROPERTY()
	TArray<UStaticMeshComponent*> BridgeTiles;
};
