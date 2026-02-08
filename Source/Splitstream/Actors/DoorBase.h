// DoorBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IUnlockable.h"
#include "DoorBase.generated.h"

UCLASS(Abstract)
class SPLITSTREAM_API ADoorBase : public AActor, public IInteractable, public IUnlockable
{
    GENERATED_BODY()

public:
    ADoorBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Components")
	class UArrowComponent* ArrowComp;

    UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsOpen = false;


    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bAutoOpenForGuards = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    class UBoxComponent* GuardOpenTrigger;

    UPROPERTY()
    class ULockPickComponent* LockPickComponent = nullptr;

    int32 ComputeOpenDirection(AActor* ReferenceActor) const;

    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override;

    // Keycard unlockable interface
    virtual void UnlockWithAccess_Implementation(AActor* Interactor) override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    virtual void OnLockUnlocked();

    UFUNCTION()
    virtual void OnRep_IsOpen();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void OpenDoor(int32 Direction);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void CloseDoor(int32 Direction);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Guard auto open/close
    UFUNCTION()
    void OnGuardOpenBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void ForceOpenDoorForGuard(AActor* GuardActor);

    UFUNCTION()
    void ForceCloseDoorForGuard(AActor* GuardActor);

protected:
    int32 OpenDirection = 1;
};