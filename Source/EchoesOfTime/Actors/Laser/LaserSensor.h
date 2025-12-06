#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "LaserSensor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS()
class ECHOESOFTIME_API ALaserSensor : public AActor
{
    GENERATED_BODY()

public:
    ALaserSensor();

    // Set active state (client -> call server, server -> immediate)
    UFUNCTION(BlueprintCallable, Category = "Laser")
    void SetActive(bool bNewActive);

    // Server RPC to set active state (authoritative)
    UFUNCTION(Server, Reliable)
    void ServerSetActive(bool bNewActive);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StartMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EndMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* LaserBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* LaserBeamNiagara;

    // Replicated active flag
    UPROPERTY(ReplicatedUsing = OnRep_IsActive, EditAnywhere, BlueprintReadWrite, Category = "Laser")
    bool bIsActive;

    UFUNCTION()
    void OnRep_IsActive();

    void ApplyActiveState();

    // replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnLaserOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};