#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserSensor.generated.h"

UCLASS()
class ECHOESOFTIME_API ALaserSensor : public AActor
{
    GENERATED_BODY()

public:
    ALaserSensor();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* StartMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* EndMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* LaserBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UNiagaraComponent* LaserBeamNiagara;

    UFUNCTION()
    void OnLaserOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};