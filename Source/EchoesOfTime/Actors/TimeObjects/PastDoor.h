#pragma once

#include "CoreMinimal.h"
#include "Actors/DoorBase.h"
#include "PastDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorStateChanged, bool, bIsOpen);

UCLASS()
class ECHOESOFTIME_API APastDoor : public ADoorBase
{
    GENERATED_BODY()

public:
    APastDoor();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

    UPROPERTY(BlueprintAssignable, Category = "PastDoor")
    FOnDoorStateChanged OnDoorStateChanged;

    void OnRep_IsOpen() override;

    // Only PastDoor has these:
    UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockPick")
    class ULockPickComponent* LockPickComponent;

    UFUNCTION()
    void OnLockUnlocked();
};