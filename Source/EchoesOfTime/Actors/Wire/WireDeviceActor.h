#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "TimelineEra.h"
#include "WireDeviceActor.generated.h"

UCLASS()
class ECHOESOFTIME_API AWireDeviceActor : public AActor
{
    GENERATED_BODY()

public:
    AWireDeviceActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* DeviceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Device")
    TSubclassOf<AWireActor> WireClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<AWireActor*> WireActors;

    UFUNCTION()
    void OnWireCut(AWireActor* CutWire);

protected:
    virtual void BeginPlay() override;
};