// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Components/RectLightComponent.h"
#include "DronePawn.generated.h"

UCLASS()
class ECHOESOFTIME_API ADronePawn : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ADronePawn();

    // Syncs RectLight cone with perception in Editor and at runtime
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    virtual void BeginPlay() override;
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    // Skeletal mesh to represent the drone
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone")
    USkeletalMeshComponent* DroneMesh;

    // The actor currently detected by the drone (set immediately when seen)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* DetectedActor = nullptr;

    // Drone spotlight (cone)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Drone|Visual")
    URectLightComponent* DroneRectLight;
};