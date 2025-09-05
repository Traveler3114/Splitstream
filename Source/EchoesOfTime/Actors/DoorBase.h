#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "DoorBase.generated.h"

UCLASS(Abstract)
class ECHOESOFTIME_API ADoorBase : public AActor, public IInteractable, public IKeycardUnlockable
{
    GENERATED_BODY()

public:
    ADoorBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bRequiresKeycard = false;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = false;

    class ULockPickComponent* LockPickComponent = nullptr;

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;

    virtual void SetHighlighted_Implementation(bool bHighlight) override;

    // Keycard unlockable interface
    virtual void UnlockWithKeycard_Implementation(AActor* Interactor) override;
    virtual bool RequiresKeycard_Implementation() const override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    virtual void OnLockUnlocked();

    UFUNCTION()
    virtual void OnRep_IsOpen();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void OpenDoor();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void CloseDoor();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};