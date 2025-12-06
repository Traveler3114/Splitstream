#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInteractable.h"
#include "Interfaces/IRequiresItem.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/IDetectable.h"
#include "DataAssets/InputMappingSet.h"
#include "DataAssets/AbilitySets/AbilityInputSet.h"
#include "DataAssets/AbilitySets/DefaultGASet.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

#include "DefaultCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
class UStaticMeshComponent;
class UInventoryComponent;
class AItemPickup;

UCLASS()
class ECHOESOFTIME_API ADefaultCharacter : public ACharacter, public IInteractable, public IRequiresItem, public IAbilitySystemInterface, public IDetectable
{
    GENERATED_BODY()

public:
    ADefaultCharacter();

    // ============================================
    // Unreal Engine Overrides
    // ============================================
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Jump() override;

    // ============================================
    // Ability System
    // ============================================
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    void InitializeAbilitySystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ASC")
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* AbilityInputSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UDefaultGASet* DefaultGASet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    UFUNCTION()
    void HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag);

    UFUNCTION()
    void HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag);

    // ============================================
    // Input System
    // ============================================
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingSet* InputMappingSet;

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
    UInputAction* DropItemAction;

    UFUNCTION()
    void HandleNumberKey(FKey PressedKey);

    // ============================================
    // Movement System
    // ============================================
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartCrouch();
    void StopCrouching();
    void StartSprint();
    void StopSprint();

    UFUNCTION(Server, Reliable)
    void ServerStartSprint();

    UFUNCTION(Server, Reliable)
    void ServerStopSprint();

    UPROPERTY(ReplicatedUsing = OnRep_SprintState)
    bool bIsSprinting;

    UFUNCTION()
    void OnRep_SprintState();

    // ============================================
    // Camera System
    // ============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FVector CameraDefaultLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FRotator CameraDefaultRotation;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Pitch)
    float Pitch = 0.0f;

    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StartAimCamera(FVector AimLocation, FRotator AimRotation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StopAimCamera(FVector ReturnLocation, FRotator ReturnRotation);

    UFUNCTION(Server, Reliable)
    void ServerCameraRotationUpdate(float NewPitch);

    UFUNCTION()
    void OnRep_Pitch();

    // ============================================
    // Inventory System
    // ============================================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UStaticMeshComponent* EquippedItemMeshComp;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateEquippedItemMesh();

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Slots);

    void DropActiveItem();
    void SelectInventorySlot(int32 SlotNumber);

    // ============================================
    // Interaction System
    // ============================================
    AActor* ProgressiveActor = nullptr;

    UPROPERTY()
    AActor* HighlightedActor = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsProgressiveInteractActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FGameplayTag GetProgressiveInteractTag(AActor* Actor) const;

    UFUNCTION()
    void HandleInteractHoldStart();

    UFUNCTION()
    void HandleInteractHoldStop();

    UFUNCTION()
    void HandleInteractInstant();

    UFUNCTION(Server, Reliable)
    void ServerHandleInteract(AActor* TargetActor);
    virtual void ServerHandleInteract_Implementation(AActor* TargetActor);

    void UpdateInteractHighlight();

    // ============================================
    // Detection System
    // ============================================
    UPROPERTY(BlueprintReadOnly)
    TMap<AActor*, float> DetectionProgressMap;

    UFUNCTION(BlueprintCallable)
    virtual void OnDetected_Implementation(AActor* Detector) override;

    UFUNCTION(BlueprintCallable)
    virtual void OnLost_Implementation(AActor* Detector) override;

    UFUNCTION(BlueprintCallable)
    virtual void OnFullyDetected_Implementation(AActor* Detector) override;

    UFUNCTION()
    void OnIllegalTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

    UFUNCTION(BlueprintPure, Category = "Detection")
    static float CalculateDetectionAngle(
        const FVector& CameraLocation,
        const FRotator& PlayerCameraRotation,
        const FVector& SelfLocation);

    // ============================================
    // Utility Functions
    // ============================================
    bool GetForwardTraceResult(float TraceDistance, FHitResult& OutHit, FVector& OutTraceEnd) const;

private:
    void GrantAbilitiesFromInputSet();
    void GrantAbilitiesFromDefaultSet();
    
    // Performance optimization: Reduce update frequency
    float TimeSinceLastInteractCheck = 0.0f;
    float TimeSinceLastDetectionUpdate = 0.0f;
    static constexpr float InteractCheckInterval = 0.1f; // Check for interactions every 100ms
    static constexpr float DetectionUpdateInterval = 0.1f; // Update detection every 100ms
};