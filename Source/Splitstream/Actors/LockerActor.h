// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimelineEra.h"
#include "Interfaces/IInteractable.h"
#include "LockerActor.generated.h"

UCLASS()
class ECHOESOFTIME_API ALockerActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALockerActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lever")
	USceneComponent* SceneRootComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locker")
	ETimelineEra TimelineEra = ETimelineEra::Past;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locker")
	UStaticMeshComponent* LockerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locker")
	UStaticMeshComponent* LockerDoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locker")
	class ULockPickComponent* LockPickComponent;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Locker")
	void OpenLocker();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Locker")
	void CloseLocker();

	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsOpen = false;

	UFUNCTION()
	void OnRep_IsOpen();


	UPROPERTY(ReplicatedUsing = OnRep_StaffName, VisibleAnywhere, BlueprintReadOnly, Category = "Staff")
	FString StaffName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locker")
	class UTextRenderComponent* NameText;

	UFUNCTION(BlueprintCallable, Category = "Locker")
	void SetStaffName(const FString& NewName);

	UFUNCTION()
	void OnRep_StaffName();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void CancelInteract_Implementation(AActor* Interactor) override;
	virtual bool IsProgressiveInteract_Implementation() override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
