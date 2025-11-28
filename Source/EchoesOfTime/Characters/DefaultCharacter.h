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
class AItemPickup;

UCLASS()
class ECHOESOFTIME_API ADefaultCharacter : public ACharacter, public IInteractable, public IRequiresItem, public IAbilitySystemInterface, public IDetectable
{
    GENERATED_BODY()
public:
    UPROPERTY()
    UTimelineComponent* AimCameraTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    UCurveFloat* AimCameraCurve;

    // Store start/end locations/rotations
    FVector CameraDefaultLocation;
    FRotator CameraDefaultRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    FVector CameraAimLocation = FVector(9.960482f, 15.432522f, 1.7f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
    FRotator CameraAimRotation = FRotator(-14.932470f, 62.527103f, -102.804844f);


    UFUNCTION()
    void OnAimCameraTimelineUpdate(float Value);

    UFUNCTION()
    void OnAimCameraTimelineFinished();

public:
    ADefaultCharacter();
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;

    UFUNCTION(BlueprintCallable)
    virtual void OnDetected_Implementation(AActor* Detector) override;
    UFUNCTION(BlueprintCallable)
    virtual void OnLost_Implementation(AActor* Detector) override;
    UFUNCTION(BlueprintCallable)
    virtual void OnFullyDetected_Implementation(AActor* Detector) override;

    UPROPERTY(BlueprintReadOnly)
    TMap<AActor*, float> DetectionProgressMap;

	void OnIllegalTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

    void InitializeAbilitySystem();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UFUNCTION()
    void OnInventoryChanged(const TArray<FInventorySlot>& Slots);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UStaticMeshComponent* EquippedItemMeshComp;

    void UpdateEquippedItemMesh();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingSet* InputMappingSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* AbilityInputSet;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UDefaultGASet* DefaultGASet;

    UFUNCTION()
    void HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag);

    UFUNCTION()
    void HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;

protected:
    UPROPERTY()
    AActor* HighlightedActor = nullptr;

    void UpdateInteractHighlight();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ASC")
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
    UInputAction* DropItemAction;

    // Movement and looking functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartCrouch();
    void StopCrouching();

    virtual void Jump() override;

    void StartSprint();
    void StopSprint();

	void HandleInteract();

    // In DefaultCharacter.h
    UFUNCTION(Server, Reliable)
    void ServerHandleInteract(AActor* TargetActor);

    void SelectInventorySlot(int32 SlotNumber);

    UFUNCTION()
    void HandleNumberKey(FKey PressedKey);

    void DropActiveItem();

    UFUNCTION(Server, Reliable)
    void ServerStartSprint();
    UFUNCTION(Server, Reliable)
    void ServerStopSprint();

    UPROPERTY(ReplicatedUsing = OnRep_SprintState)
    bool bIsSprinting;

    UFUNCTION()
    void OnRep_SprintState();

    UFUNCTION(Server, Reliable)
    void ServerCameraRotationUpdate(float NewPitch);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Pitch)
    float Pitch = 0.0f;

    UFUNCTION()
    void OnRep_Pitch();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    bool GetForwardTraceResult(float TraceDistance, FHitResult& OutHit, FVector& OutTraceEnd) const;

    UFUNCTION(BlueprintPure, Category = "Detection")
    static float CalculateDetectionAngle(
        const FVector& CameraLocation,
        const FRotator& PlayerCameraRotation,
        const FVector& SelfLocation)
    {
        // Vector from player to detector
        FVector ToDetector = CameraLocation - SelfLocation;

        // Get Forward and Right vector from camera rotation
        FVector CameraForward = PlayerCameraRotation.Vector();
        FVector CameraRight = FRotationMatrix(PlayerCameraRotation).GetUnitAxis(EAxis::Y);

        // Flatten vectors (ignore Z)
        CameraForward.Z = 0.f;
        CameraRight.Z = 0.f;
        ToDetector.Z = 0.f;

        // Normalize
        CameraForward.Normalize();
        CameraRight.Normalize();
        ToDetector.Normalize();

        // Compute angle between camera forward and ToDetector
        float Dot = FVector::DotProduct(CameraForward, ToDetector);
        Dot = FMath::Clamp(Dot, -1.f, 1.f); // Prevent NaN

        float AngleRad = FMath::Acos(Dot);
        float AngleDeg = FMath::RadiansToDegrees(AngleRad);

        // Sign determines left/right side for UI
        float Sign = FVector::DotProduct(CameraRight, ToDetector) > 0.f ? 1.f : -1.f;
        AngleDeg *= Sign;

        return AngleDeg;
    }

private:
    void GrantAbilitiesFromInputSet();
    void GrantAbilitiesFromDefaultSet();
};