#include "DefaultCharacter.h"
#include "Splitstream.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DefaultPlayerState.h"
#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Actors/ItemPickup.h"
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "DefaultGameInstance.h"
#include "AbilitySystem/SplitstreamGameplayTags.h"
#include "Interfaces/IDetectable.h"
#include "Controllers/DefaultPlayerController.h"
#include "ActorComponents/HackComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/InteractionComponent.h"
#include "AbilitySystem/DefaultAbilitySystemComponent.h"
#include "ActorComponents/LockPickComponent.h"
#include "ActorComponents/DetectionComponent.h"
#include "Subsystems/DetectorRegistry.h"

ADefaultCharacter::ADefaultCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetMesh()->SetOwnerNoSee(false);

    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    bReplicates = true;
    SetReplicateMovement(true);

    bIsSprinting = false;

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

    DetectionComponent = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));

    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

    EquippedItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedItemMeshComp"));
    EquippedItemMeshComp->SetupAttachment(GetMesh(), TEXT("Hand_R"));
    EquippedItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InventoryComponent->EquippedItemMeshComp = EquippedItemMeshComp;
}

void ADefaultCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    //if (InventoryComponent)
    //{
    //    InventoryComponent->OnInventoryChanged.AddDynamic(this, &ADefaultCharacter::OnInventoryChanged);
    //}
}

void ADefaultCharacter::BeginPlay()
{
    Super::BeginPlay();

    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            // Remove any old mapping contexts to prevent duplicates/conflicts
            Subsystem->ClearAllMappings();

            // Always use the runtime mapping context from GameInstance if available
            UInputMappingContext* MappingContext = GI ? GI->GetCurrentInputMappingContext() : DefaultMappingContext;
            if (MappingContext)
            {
                Subsystem->AddMappingContext(MappingContext, 0);
            }
        }
    }

    CameraComponent = FindComponentByClass<UCameraComponent>();
    if (CameraComponent)
    {
        CameraDefaultLocation = CameraComponent->GetRelativeLocation();
        CameraDefaultRotation = CameraComponent->GetRelativeRotation();
    }

    InventoryComponent->UpdateEquippedItemMesh(InventoryComponent->Slots);
}

void ADefaultCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up GAS delegates to prevent dangling references.
    // The ASC lives on PlayerState and can outlive this pawn.
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RegisterGameplayTagEvent(
            TAG_Character_Status_Illegal,
            EGameplayTagEventType::NewOrRemoved
        ).Remove(IllegalTagDelegateHandle);

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UPlayerAttributeSet::GetWalkSpeedAttribute()
        ).Remove(WalkSpeedDelegateHandle);

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UPlayerAttributeSet::GetRunSpeedAttribute()
        ).Remove(RunSpeedDelegateHandle);

        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UPlayerAttributeSet::GetCrouchSpeedAttribute()
        ).Remove(CrouchSpeedDelegateHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void ADefaultCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsLocallyControlled() && InteractionComponent)
    {
        InteractionComponent->InteractHighlightTimer += DeltaTime;
        if (InteractionComponent->InteractHighlightTimer >= InteractionComponent->InteractHighlightInterval)
        {
            InteractionComponent->InteractHighlightTimer = 0.f;
            FVector Start = CameraComponent->GetComponentLocation();
            FRotator Rot = Controller ? Controller->GetControlRotation() : CameraComponent->GetComponentRotation();
            InteractionComponent->UpdateInteractHighlight(Start, Rot);
        }
    }
}

// ---------------- ABILITY SYSTEM ------------------
void ADefaultCharacter::InitializeAbilitySystem()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS)
    {
        UE_LOG(LogSplitstream, Warning, TEXT("%s: InitializeAbilitySystem - PlayerState is null"), *GetName());
        return;
    }

    AbilitySystemComponent = PS->GetAbilitySystemComponent();
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogSplitstream, Warning, TEXT("%s: InitializeAbilitySystem - AbilitySystemComponent is null"), *GetName());
        return;
    }

    AbilitySystemComponent->InitAbilityActorInfo(PS, this);

    // Store delegate handle for cleanup
    IllegalTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(
        TAG_Character_Status_Illegal,
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &ADefaultCharacter::OnIllegalTagChanged);

    WalkSpeedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UPlayerAttributeSet::GetWalkSpeedAttribute()
    ).AddUObject(this, &ADefaultCharacter::OnWalkSpeedChanged);

    RunSpeedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UPlayerAttributeSet::GetRunSpeedAttribute()
    ).AddUObject(this, &ADefaultCharacter::OnRunSpeedChanged);

    CrouchSpeedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UPlayerAttributeSet::GetCrouchSpeedAttribute()
    ).AddUObject(this, &ADefaultCharacter::OnCrouchSpeedChanged);

    // Delegate attribute init to the ASC
    if (HasAuthority())
    {
        if (UDefaultAbilitySystemComponent* ASC = GetDefaultASC())
        {
            ASC->InitializeAttributes(AttributeInitGE);
        }
    }
}

void ADefaultCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
    if (!bIsSprinting && !GetCharacterMovement()->IsCrouching())
    {
        GetCharacterMovement()->MaxWalkSpeed = ChangeData.NewValue;
    }
}

void ADefaultCharacter::OnRunSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
    if (bIsSprinting)
        GetCharacterMovement()->MaxWalkSpeed = ChangeData.NewValue;
}

void ADefaultCharacter::OnCrouchSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
    if (GetCharacterMovement()->IsCrouching())
        GetCharacterMovement()->MaxWalkSpeed = ChangeData.NewValue;
    GetCharacterMovement()->MaxWalkSpeedCrouched = ChangeData.NewValue;
}



UAbilitySystemComponent* ADefaultCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

UDefaultAbilitySystemComponent* ADefaultCharacter::GetDefaultASC() const
{
    return Cast<UDefaultAbilitySystemComponent>(AbilitySystemComponent);
}

// ------------------ DETECTION SYSTEM EVENT-DRIVEN --------------------
void ADefaultCharacter::OnDetected_Implementation(AActor* Detector)
{
    if (!DetectionComponent || !AbilitySystemComponent)
        return;

    // Assuming TAG_Character_Status_Illegal is somewhere in scope:
    if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Character_Status_Illegal))
    {
        DetectionComponent->StartDetection(Detector);
    }
}

void ADefaultCharacter::OnLost_Implementation(AActor* Detector)
{
    if(DetectionComponent)
        DetectionComponent->StopDetection(Detector);
}

void ADefaultCharacter::OnForceDetectionEnd_Implementation(AActor* Detector)
{
    if (DetectionComponent) DetectionComponent->ForceImmediateDetectionEnd(Detector);
}


void ADefaultCharacter::OnFullyDetected_Implementation(AActor* Detector)
{

}

// -- Called on illegal tag state change; triggers OnDetected for all legitimate detectors
void ADefaultCharacter::OnIllegalTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    UWorld* World = GetWorld();
    if (!World) return;

    UDetectorRegistry* Registry = World->GetSubsystem<UDetectorRegistry>();
    if (!Registry) return;

    // Copy to array to safely iterate (prevents iterator invalidation if callbacks modify the registry)
    TArray<AActor*> Detectors = Registry->GetValidDetectors();

    if (NewCount > 0)
    {
        for (AActor* Detector : Detectors)
        {
            if (!IsValid(Detector)) continue;
            if (!Detector->GetClass()->ImplementsInterface(UDetectable::StaticClass())) continue;
            if (IDetectable::Execute_IsActorAlreadyDetected(Detector, this))
            {
                IDetectable::Execute_OnDetected(this, Detector);
            }
        }
    }
    else
    {
        for (AActor* Detector : Detectors)
        {
            if (!IsValid(Detector)) continue;
            if (!Detector->GetClass()->ImplementsInterface(UDetectable::StaticClass())) continue;
            if (IDetectable::Execute_IsActorAlreadyDetected(Detector, this))
            {
                IDetectable::Execute_OnLost(this, Detector);
            }
        }
    }
}



// ---------------- INPUT ------------------
void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogSplitstream, Error, TEXT("ADefaultCharacter::SetupPlayerInputComponent - "
            "PlayerInputComponent is not a UEnhancedInputComponent! "
            "Ensure DefaultInputComponentClass is set to EnhancedInputComponent in project settings."));
        return;
    }

    EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Move);
    EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Look);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADefaultCharacter::Jump);
    EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADefaultCharacter::StopJumping);
    EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ADefaultCharacter::StartCrouch);
    EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ADefaultCharacter::StopCrouching);
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADefaultCharacter::ServerStartSprint);
    EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADefaultCharacter::ServerStopSprint);
    EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADefaultCharacter::OnInstantInteract);
    EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::OnHoldInteractStart);
    EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ADefaultCharacter::OnHoldInteractStop);
    EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Completed, this, &ADefaultCharacter::OnDropActiveItem);

    if (InputMappingSet)
    {
        for (const FInputActionTagMapping& MapEntry : InputMappingSet->Mappings)
        {
            if (MapEntry.InputAction)
            {
                EnhancedInputComponent->BindAction(MapEntry.InputAction, ETriggerEvent::Started, this, &ADefaultCharacter::HandleAbilityInput, MapEntry.InputTag);
                EnhancedInputComponent->BindAction(MapEntry.InputAction, ETriggerEvent::Completed, this, &ADefaultCharacter::HandleAbilityInputReleased, MapEntry.InputTag);
            }
        }
    }

    PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
    PlayerInputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &ADefaultCharacter::HandleNumberKey);
}


void ADefaultCharacter::OnInstantInteract()
{
    if (!InteractionComponent || !CameraComponent) return;
    FVector Start = CameraComponent->GetComponentLocation();
    FRotator Rot = Controller ? Controller->GetControlRotation() : CameraComponent->GetComponentRotation();
    InteractionComponent->HandleInstantInteract(
        this, Start, Rot, [this](AActor* Target){
            ServerHandleInteract(Target); // Client always forwards instant to server
        }
    );
}

void ADefaultCharacter::OnHoldInteractStart()
{
    if (!InteractionComponent || !CameraComponent) return;
    FVector Start = CameraComponent->GetComponentLocation();
    FRotator Rot = Controller ? Controller->GetControlRotation() : CameraComponent->GetComponentRotation();
    InteractionComponent->HandleHoldInteractStart(this, Start, Rot);
}

void ADefaultCharacter::OnHoldInteractStop()
{
    if (!InteractionComponent) return;
    InteractionComponent->HandleHoldInteractStop(this);
}

void ADefaultCharacter::HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag)
{
    if (UDefaultAbilitySystemComponent* ASC = GetDefaultASC())
    {
        ASC->HandleAbilityInputPressed(InputTag);
    }
}

void ADefaultCharacter::HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag)
{
    if (UDefaultAbilitySystemComponent* ASC = GetDefaultASC())
    {
        ASC->HandleAbilityInputReleased(InputTag);
    }
}

void ADefaultCharacter::HandleNumberKey(FKey PressedKey)
{
    static const TMap<FName, int32> KeyToSlot{
        {EKeys::One.GetFName(),   0},
        {EKeys::Two.GetFName(),   1},
        {EKeys::Three.GetFName(), 2},
        {EKeys::Four.GetFName(),  3},
        {EKeys::Five.GetFName(),  4},
        {EKeys::Six.GetFName(),   5},
        {EKeys::Seven.GetFName(), 6},
        {EKeys::Eight.GetFName(), 7},
        {EKeys::Nine.GetFName(),  8},
        {EKeys::Zero.GetFName(),  9}
    };

    if (const int32* Found = KeyToSlot.Find(PressedKey.GetFName()))
    {
        if (InventoryComponent)
        {
            InventoryComponent->ServerSetActiveSlot(*Found);
        }
    }
}

void ADefaultCharacter::SelectInventorySlot(int32 SlotNumber)
{
    int32 SlotIndex = SlotNumber - 1;
    if (InventoryComponent)
    {
        InventoryComponent->ServerSetActiveSlot(SlotIndex);
    }
}


void ADefaultCharacter::OnDropActiveItem()
{
    if (!InteractionComponent || !InventoryComponent || !CameraComponent) return;
    InteractionComponent->DropEquippedItem(InventoryComponent, CameraComponent);
}



void ADefaultCharacter::ServerHandleInteract_Implementation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogSplitstream, Warning, TEXT("%s: ServerHandleInteract - TargetActor is null"), *GetName());
        return;
    }

    if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        bool bRequiresItem = IInteractable::Execute_RequiresItem(TargetActor);

        UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
        if (Inventory)
        {
            FInventorySlot ActiveSlot = Inventory->GetActiveItem();

            if (bRequiresItem)
            {
                if (!IInteractable::Execute_IsCorrectItem(TargetActor, ActiveSlot))
                {
                    return;
                }
                if (ActiveSlot.ItemAsset)
                {
                    ActiveSlot.ItemAsset->OnUsed(this, ActiveSlot.ItemInstanceID);
                }
            }
        }
        IInteractable::Execute_Interact(TargetActor, this);
    }
}

// ---------------- MOVEMENT --------------
void ADefaultCharacter::Move(const FInputActionValue& Value)
{
    if (!IsValid(this)) return;

    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (ASC && ASC->HasMatchingGameplayTag(TAG_Character_Status_Block_Movement))
        return;
    if (!Controller) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void ADefaultCharacter::Look(const FInputActionValue& Value)
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (ASC && ASC->HasMatchingGameplayTag(TAG_Character_Status_Block_Look))
        return;

    FVector2D LookAxisVector = Value.Get<FVector2D>();

    float Sensitivity = 1.0f;
    UDefaultGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance<UDefaultGameInstance>() : nullptr;
    if (GI)
        Sensitivity = GI->GetMouseSensitivity();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X * Sensitivity);
        AddControllerPitchInput(LookAxisVector.Y * Sensitivity);
        if (CameraComponent) {
            ServerCameraRotationUpdate(CameraComponent->GetComponentRotation().Pitch);
        }
    }
}


void ADefaultCharacter::Jump()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (ASC && ASC->HasMatchingGameplayTag(TAG_Character_Status_Block_Movement))
        return;
    Super::Jump();
}

const UPlayerAttributeSet* ADefaultCharacter::GetPlayerAttributeSet() const
{
    return AbilitySystemComponent
        ? Cast<const UPlayerAttributeSet>(AbilitySystemComponent->GetAttributeSet(UPlayerAttributeSet::StaticClass()))
        : nullptr;
}

void ADefaultCharacter::StartCrouch()
{
    if (!GetCharacterMovement()->IsFalling())
        Crouch();

    const UPlayerAttributeSet* AttrSet = GetPlayerAttributeSet();
    if (AttrSet && GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetCrouchSpeed();
        GetCharacterMovement()->MaxWalkSpeedCrouched = AttrSet->GetCrouchSpeed();
    }
}

void ADefaultCharacter::StopCrouching()
{
    UnCrouch();

    const UPlayerAttributeSet* AttrSet = GetPlayerAttributeSet();
    if (AttrSet && GetCharacterMovement())
    {
        if (bIsSprinting)
            GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetRunSpeed();
        else
            GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetWalkSpeed();

        GetCharacterMovement()->MaxWalkSpeedCrouched = AttrSet->GetCrouchSpeed();
    }
}

void ADefaultCharacter::StartSprint()
{
    if (!GetCharacterMovement()->IsCrouching() && GetCharacterMovement()->Velocity.Size() > 0)
    {
        bIsSprinting = true;
        OnRep_SprintState();
    }
}

void ADefaultCharacter::StopSprint()
{
    bIsSprinting = false;
    OnRep_SprintState();
}

void ADefaultCharacter::ServerStartSprint_Implementation()
{
    StartSprint();
}

void ADefaultCharacter::ServerStopSprint_Implementation()
{
    StopSprint();
}

void ADefaultCharacter::OnRep_SprintState()
{
    const UPlayerAttributeSet* AttrSet = GetPlayerAttributeSet();
    if (!GetCharacterMovement() || !AttrSet)
        return;

    if (bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetRunSpeed();
    }
    else if (GetCharacterMovement()->IsCrouching())
    {
        GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetCrouchSpeed();
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = AttrSet->GetWalkSpeed();
    }
    GetCharacterMovement()->MaxWalkSpeedCrouched = AttrSet->GetCrouchSpeed();
}

void ADefaultCharacter::ServerCameraRotationUpdate_Implementation(float NewPitch)
{
    Pitch = NewPitch;
    OnRep_Pitch();
}

void ADefaultCharacter::OnRep_Pitch()
{
    if (CameraComponent)
    {
        FRotator NewRotation = FRotator(Pitch, CameraComponent->GetComponentRotation().Yaw, CameraComponent->GetComponentRotation().Roll);
        CameraComponent->SetWorldRotation(NewRotation);
    }
}

// ---------------- REPLICATION ------------------
void ADefaultCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
    if (!HasAuthority()) return;

    UDefaultAbilitySystemComponent* ASC = GetDefaultASC();
    if (!ASC) return;

    // Grant team-based input abilities
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Team.Future")))
    {
        ASC->GrantAbilitiesFromSet(FutureGASet);
    }
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Team.Solo")))
    {
        ASC->GrantAbilitiesFromSet(SoloGASet);
        ASC->GrantAbilitiesFromSet(FutureGASet);
    }

    // Grant default abilities
    ASC->GrantAbilitiesFromDefaultSet(DefaultGASet);
}

void ADefaultCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
}

void ADefaultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADefaultCharacter, bIsSprinting);
    DOREPLIFETIME(ADefaultCharacter, Pitch);
}

float ADefaultCharacter::CalculateDetectionAngle(const FVector& CameraLocation, const FRotator& PlayerCameraRotation, const FVector& SelfLocation)
{
    FVector ToDetector = CameraLocation - SelfLocation;

    FVector CameraForward = PlayerCameraRotation.Vector();
    FVector CameraRight = FRotationMatrix(PlayerCameraRotation).GetUnitAxis(EAxis::Y);

    CameraForward.Z = 0.f;
    CameraRight.Z = 0.f;
    ToDetector.Z = 0.f;

    CameraForward.Normalize();
    CameraRight.Normalize();
    ToDetector.Normalize();

    float Dot = FVector::DotProduct(CameraForward, ToDetector);
    Dot = FMath::Clamp(Dot, -1.f, 1.f);

    float AngleRad = FMath::Acos(Dot);
    float AngleDeg = FMath::RadiansToDegrees(AngleRad);

    float Sign = FVector::DotProduct(CameraRight, ToDetector) > 0.f ? 1.f : -1.f;
    AngleDeg *= Sign;

    return AngleDeg;
}