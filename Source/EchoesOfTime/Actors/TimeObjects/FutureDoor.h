// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "FutureDoor.generated.h"

UCLASS()
class ECHOESOFTIME_API AFutureDoor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AFutureDoor();

	virtual void Interact_Implementation(AActor* Interactor) override;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "FutureDoor")
	void OpenDoor();

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category = "FutureDoor")
	void CloseDoor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* Door;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMesh3;

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Components")
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_IsOpen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PastDoor")
	TSoftObjectPtr<class APastDoor> PastDoor;

	UFUNCTION()
	void HandlePastDoorStateChanged(bool bPastIsOpen);
};
