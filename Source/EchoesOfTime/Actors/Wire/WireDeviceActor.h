// WireDeviceActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WireActor.h"
#include "TimelineEra.h"
#include "WireDeviceActor.generated.h"

USTRUCT(BlueprintType)
struct FWireDeviceColorConfig
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadWrite)
    EWireColor WireColorA = EWireColor::None;
    UPROPERTY(BlueprintReadWrite)
    EWireColor WireColorB = EWireColor::None;
    UPROPERTY(BlueprintReadWrite)
    EWireColor RequiredWireColor = EWireColor::None;
};

UCLASS()
class ECHOESOFTIME_API AWireDeviceActor : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wires")
    TArray<FVector> WireRelativeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wires")
    FVector WireRelativeScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wires")
    FRotator WireRelativeRotation = FRotator::ZeroRotator;

    AWireDeviceActor();

    UFUNCTION(BlueprintCallable)
    void ApplyColorConfiguration(const FWireDeviceColorConfig& Config);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* DeviceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AWireActor> WireClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    TArray<AWireActor*> WireActors;

    UPROPERTY(BlueprintReadOnly, Replicated)
    FString SpawnLocationName;

    UFUNCTION()
    void OnWireCut(AWireActor* CutWire);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};