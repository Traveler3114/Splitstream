#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IKeycardUnlockable.h"
#include "DoubleDoorBase.generated.h"

UCLASS(Abstract)
class ECHOESOFTIME_API ADoubleDoorBase : public AActor, public IInteractable, public IKeycardUnlockable
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bRequiresKeycard = false;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bAutoOpenForGuards = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    class UBoxComponent* GuardOpenTrigger;

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

    // Guard auto open/close
    UFUNCTION()
    void OnGuardOpenBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void ForceOpenDoorForGuard();

    UFUNCTION()
    void ForceCloseDoorForGuard();
};