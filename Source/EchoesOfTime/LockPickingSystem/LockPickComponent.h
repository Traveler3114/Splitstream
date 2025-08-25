#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockPickComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockUnlocked);
// --- Pin data struct ---
USTRUCT(BlueprintType)
struct FLockPinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float SweetSpotAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float Tolerance = 10.f;
};

// --- LockPickComponent ---
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API ULockPickComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULockPickComponent();


    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockUnlocked OnUnlock;
    // Pins: set up in BP/Editor (add as many as you like)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    TArray<FLockPinData> Pins;

    // Replicated state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    int32 CurrentPinIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    TArray<bool> PinSetStates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    bool bUnlocked = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "LockPick")
    bool bPickingInProgress = false;

    // Blueprint API
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

    // Actual RPC
    UFUNCTION(Server, Reliable)
    void ServerTrySetPin(float InputAngle);

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    float NormalizeAngle(float Angle) const;
};