#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FutureCube.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureCube : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFutureCube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Static mesh component for the Object
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ObjectMesh;

	// Reference to the PresentObject actor
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<class APresentCube> PresentObject;

	//FTransform CurrentPresentObjectTransform;
	FTransform PreviousPresentObjectTransform;

	double InitialXOffset;

};
