#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockPickComponent.generated.h"

// Delegate for when the lock is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLockUnlocked);

UENUM(BlueprintType)
enum class ELockDifficulty : uint8
{
    Easy UMETA(DisplayName = "Easy"),
    Medium UMETA(DisplayName = "Medium"),
    Hard UMETA(DisplayName = "Hard")
};

USTRUCT(BlueprintType)
struct FLockPinData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float SweetSpotAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPin")
    float Tolerance = 10.f;
};

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ECHOESOFTIME_API ULockPickComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULockPickComponent();

    // --- Editor Setup ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick|Setup", meta = (ClampMin = "1", ClampMax = "5"))
    int32 PinCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick|Setup")
    ELockDifficulty LockDifficulty = ELockDifficulty::Medium;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockUnlocked OnUnlock;

    // --- Pins (auto-generated, not editable) ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockPick")
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

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    int32 GetActivePinIndex() const { return CurrentPinIndex; }

    UFUNCTION(BlueprintCallable, Category = "LockPick")
    float GetPinAngleProximity(int32 PinIndex, float InputAngle) const;

    // Add to public section:
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void Interact(AActor* Interactor);
protected:
    virtual void BeginPlay() override;
    virtual void OnComponentCreated() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void GeneratePins();

    float NormalizeAngle(float Angle) const;
};