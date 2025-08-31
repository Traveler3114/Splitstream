#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "DoubleDoorBase.generated.h"

UCLASS(Abstract)
class ECHOESOFTIME_API ADoubleDoorBase : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    ADoubleDoorBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorLeftMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorRightMesh;

    UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsOpen = false;

    UFUNCTION()
    virtual void OnRep_IsOpen();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void CloseDoor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bRequiresKeycard = false;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};