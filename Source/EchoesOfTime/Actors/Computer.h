// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Computer.generated.h"

// Forward declaration
class UHackComponent;

UCLASS()
class ECHOESOFTIME_API AComputer : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AComputer();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class USceneComponent* DefaultSceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Computer")
    class UStaticMeshComponent* ComputerMesh;

    UHackComponent* HackComponent = nullptr;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};