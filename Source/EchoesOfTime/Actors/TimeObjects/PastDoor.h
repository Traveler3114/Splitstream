#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "PastDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsOpen);

UCLASS()
class ECHOESOFTIME_API APastDoor : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    APastDoor();

    virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable, Category = "PastDoor")
    FOnDoorStateChanged OnDoorStateChanged;

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "PastDoor")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "PastDoor")
    void CloseDoor();

    // Components
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
};