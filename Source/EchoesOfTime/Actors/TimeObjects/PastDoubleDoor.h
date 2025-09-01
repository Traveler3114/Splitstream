#pragma once

#include "CoreMinimal.h"
#include "Actors/DoubleDoorBase.h"
#include "PastDoubleDoor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoubleDoorStateChanged, bool, bIsOpen);

UCLASS()
class ECHOESOFTIME_API APastDoubleDoor : public ADoubleDoorBase
{
    GENERATED_BODY()

public:
    APastDoubleDoor();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintAssignable, Category = "PastDoubleDoor")
    FOnDoubleDoorStateChanged OnDoubleDoorStateChanged;

    void OnRep_IsOpen() override;

    // Only PastDoubleDoor has these:
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockPick")
    class ULockPickComponent* LockPickComponent;

    UFUNCTION()
    void OnLockUnlocked();
};