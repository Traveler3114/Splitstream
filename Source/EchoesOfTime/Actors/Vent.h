// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "TimelineEra.h"
#include "Vent.generated.h"

UCLASS()
class ECHOESOFTIME_API AVent : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* VentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
	class USearchComponent* SearchComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
	void OpenVent();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
	void CloseVent();


	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;

	UFUNCTION()
	virtual void OnSearchComplete();

	UPROPERTY(ReplicatedUsing = OnRep_OpenState)
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_OpenState();

};
