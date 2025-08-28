#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInteractable.h"
#include "InputActionValue.h"
#include "LockPickingSystem/LockPickComponent.h"
#include "AbilitySystemInterface.h"
#include "DefaultCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
struct FInputActionValue;

UCLASS()
class ECHOESOFTIME_API ADefaultCharacter : public ACharacter, public IInteractable, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    ADefaultCharacter();
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    //virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;

    void InitializeAbilitySystem();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Input actions


protected:
    UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="ASC")
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    class UInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PastEchoAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DropItemAction;

    // Movement and looking functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartCrouch();
    void StopCrouching();

	virtual void Jump() override;

    // Sprint functions
    void StartSprint();
    void StopSprint();

    UFUNCTION(Server, Reliable)
    void ServerHandleInteract();

    void ActivateFutureGAPastEcho();

    void SelectInventorySlot(int32 SlotNumber);

    UFUNCTION()
    void HandleNumberKey(FKey PressedKey);

    void DropActiveItem();

    // Server-side sprinting
    UFUNCTION(Server, Reliable)
    void ServerStartSprint();
    UFUNCTION(Server, Reliable)
    void ServerStopSprint();

    // Replicated sprint state
    UPROPERTY(ReplicatedUsing = OnRep_SprintState)
    bool bIsSprinting;

    UFUNCTION()
    void OnRep_SprintState();

    UFUNCTION(Server, Reliable)
    void ServerCameraRotationUpdate(float NewPitch);

    UPROPERTY(ReplicatedUsing = OnRep_Pitch)
    float Pitch = 0.0f;

    UFUNCTION()
    void OnRep_Pitch();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    bool GetForwardTraceResult(float TraceDistance, FHitResult& OutHit, FVector& OutTraceEnd) const;
};