#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IInteractable.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/IDetectable.h"
#include "DataAssets/InputMappingSet.h"
#include "DataAssets/AbilitySets/AbilityInputSet.h"
#include "DataAssets/AbilitySets/DefaultGASet.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "GameplayEffect.h"
#include "DefaultCharacter.generated.h"

class UCameraComponent;
class UInteractionComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
class UDefaultAbilitySystemComponent;
class UStaticMeshComponent;
class UInventoryComponent;
class UDetectionComponent;
class AItemPickup;


UCLASS()
class SPLITSTREAM_API ADefaultCharacter : public ACharacter, public IInteractable, public IAbilitySystemInterface, public IDetectable
{
    GENERATED_BODY()
public:
    ADefaultCharacter();

    void OnWalkSpeedChanged(const FOnAttributeChangeData& ChangeData);
    void OnRunSpeedChanged(const FOnAttributeChangeData& ChangeData);
    void OnCrouchSpeedChanged(const FOnAttributeChangeData& ChangeData);

    const UPlayerAttributeSet* GetPlayerAttributeSet() const;

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;

    // Inventory Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateEquippedItemMesh();

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Slots);

    void DropActiveItem();
    void SelectInventorySlot(int32 SlotNumber);
    UFUNCTION()
    void HandleNumberKey(FKey PressedKey);

    // Ability System
    void InitializeAbilitySystem();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ASC")
    UAbilitySystemComponent* AbilitySystemComponent;

    /** Typed accessor � returns the ASC cast to our custom subclass, or nullptr */
    UDefaultAbilitySystemComponent* GetDefaultASC() const;

    // Input-to-ability routing (thin wrappers that delegate to ASC)
    UFUNCTION()
    void HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag);
    UFUNCTION()
    void HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* FutureGASet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* SoloGASet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UDefaultGASet* DefaultGASet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

    // Input Actions
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

    // Movement Functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    virtual void Jump() override;
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

    // Camera (and Aim Camera Blueprint Events)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FVector CameraDefaultLocation;
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FRotator CameraDefaultRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    FVector CameraAimLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    FRotator CameraAimRotation;

    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StartAimCamera(FVector AimLocation, FRotator AimRotation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StopAimCamera(FVector ReturnLocation, FRotator ReturnRotation);

    UFUNCTION(Server, Reliable)
    void ServerCameraRotationUpdate(float NewPitch);
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Pitch)
    float Pitch = 0.0f;
    UFUNCTION()
    void OnRep_Pitch();

    // Inventory/Item Mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UStaticMeshComponent* EquippedItemMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    UDetectionComponent* DetectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    UInteractionComponent* InteractionComponent;

    UFUNCTION()
    void OnInstantInteract();

    UFUNCTION()
    void OnHoldInteractStart();

    UFUNCTION()
    void OnHoldInteractStop();

    UFUNCTION()
    void OnDropActiveItem();

    // Server-side implementation
    UFUNCTION(Server, Reliable)
    void ServerHandleInteract(AActor* TargetActor);


    UFUNCTION(BlueprintCallable)
    virtual void OnDetected_Implementation(AActor* Detector) override;
    UFUNCTION(BlueprintCallable)
    virtual void OnLost_Implementation(AActor* Detector) override;
    UFUNCTION(BlueprintCallable)
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;
    UFUNCTION(BlueprintCallable)
    virtual void OnFullyDetected_Implementation(AActor* Detector) override;

    UFUNCTION()
    void OnIllegalTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

    UFUNCTION(BlueprintPure, Category = "Detection")
    static float CalculateDetectionAngle(
        const FVector& CameraLocation,
        const FRotator& PlayerCameraRotation,
        const FVector& SelfLocation);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // REMOVED: GrantAbilitiesFromInputSet(), GrantAbilitiesFromDefaultSet(), GrantAbilitiesFromSet()
    // These now live on UDefaultAbilitySystemComponent

    FDelegateHandle IllegalTagDelegateHandle;
    FDelegateHandle WalkSpeedDelegateHandle;
    FDelegateHandle RunSpeedDelegateHandle;
    FDelegateHandle CrouchSpeedDelegateHandle;
};