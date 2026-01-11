// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IDetectable.h"
#include "VentBase.generated.h"

UCLASS()
class SPLITSTREAM_API AVentBase : public AActor, public IInteractable, public IDetectable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVentBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* VentBaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Device")
	class USearchComponent* SearchComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Detection")
    class UDetectionComponent* DetectionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Vent")
	void OpenVent();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Vent")
	void CloseVent();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual bool IsProgressiveInteract_Implementation() override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;

	virtual void OnDetected_Implementation(AActor* Detector) override;
	virtual void OnLost_Implementation(AActor* Detector) override;
	virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;

	UFUNCTION()
	virtual void OnSearchComplete();

	UPROPERTY(ReplicatedUsing = OnRep_OpenState)
	bool bIsOpen = false;

	UFUNCTION()
	virtual void OnRep_OpenState();

};
