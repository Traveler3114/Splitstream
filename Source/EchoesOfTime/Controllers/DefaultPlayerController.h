#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "DefaultPlayerController.generated.h"

UENUM(BlueprintType)
enum class EPlayerInputMode : uint8
{
    Gameplay,
    LockPicking
};

UCLASS()
class ECHOESOFTIME_API ADefaultPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaSeconds) override;

    // Lockpick minigame interface
    void StartLockPickMinigame(ULockPickComponent* LockComp);
    void EndLockPickMinigame(bool bSuccess);

    void SetPlayerInputMode(EPlayerInputMode NewMode);

protected:
    // Lockpick minigame state
    UPROPERTY()
    ULockPickComponent* ActiveLockPickComp = nullptr;
    bool bIsLockPicking = false;
    FVector2D LockPickInputVector = FVector2D::ZeroVector;
    float LockPickDialAngle = 0.0f;

    EPlayerInputMode CurrentInputMode = EPlayerInputMode::Gameplay;

    // Bindings for LockPick mode
    void OnLockPickInputX(float Axis);
    void OnLockPickInputY(float Axis);
    void OnLockPickConfirm();
    void OnLockPickCancel();

    // Helper
    void UpdateLockPickDebug();

    // For disabling/enabling character input
    void SetCharacterInputEnabled(bool bEnabled);

private:
    UPROPERTY()
    class ACharacterHUD* CharacterHUD;

    UPROPERTY()
    class UCharacterOverlay* CharacterOverlay;
};