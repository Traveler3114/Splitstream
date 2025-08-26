#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockPickComponent.generated.h"

// Delegate for when the lock is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockUnlocked);

USTRUCT(BlueprintType)
struct FLockPinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float SweetSpotAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float Tolerance = 10.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API ULockPickComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULockPickComponent();

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockUnlocked OnUnlock;

    // --- Configurable Pins (set in BP/Editor) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    TArray<FLockPinData> Pins;

    // --- Replicated State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    int32 CurrentPinIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    TArray<bool> PinSetStates;

    UPROPERTY(ReplicatedUsing = OnRep_Unlocked)
    bool bUnlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    bool bPickingInProgress = false;

    // --- Blueprint API ---
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    int32 GetPinCount() const { return Pins.Num(); }

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    bool GetCurrentPinData(float& OutSweetSpotAngle, float& OutTolerance) const;

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    bool TrySetCurrentPin(float InputAngle);

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    bool AdvancePin();

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void ResetLock();

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    bool IsUnlocked() const { return bUnlocked; }

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void StartLockPicking();

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void EndLockPicking();

    // --- RPCs ---
    UFUNCTION(Server, Reliable)
    void ServerTrySetPin(float InputAngle);

    UFUNCTION()
    void OnRep_Unlocked();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    float NormalizeAngle(float Angle) const;
};