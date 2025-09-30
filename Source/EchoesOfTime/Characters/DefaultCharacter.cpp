#include "DefaultCharacter.h"
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
#include "Widgets/HUD/CharacterHUD.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Actors/ItemPickup.h"
#include "Controllers/DefaultPlayerController.h"

ADefaultCharacter::ADefaultCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetMesh()->SetOwnerNoSee(false);
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
    bReplicates = true;
    SetReplicateMovement(true);
    bIsSprinting = false;
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

    EquippedItemMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedItemMeshComp"));
    EquippedItemMeshComp->SetupAttachment(GetMesh(), TEXT("HandGrip_R")); // Attach to hand socket
    EquippedItemMeshComp->SetIsReplicated(true); // Replicate mesh location if needed
    EquippedItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//EquippedItemMeshComp->SetOwnerNoSee(true);
    AimCameraTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimCameraTimeline"));
    // If doing FP/TP separation, set up both and use OwnerNoSee/OnlyOwnerSee
}

void ADefaultCharacter::UpdateInteractHighlight()
{
    if (!IsLocallyControlled())
        return;

    FHitResult Hit;
    FVector TraceEnd;
    bool bHit = GetForwardTraceResult(300.f, Hit, TraceEnd);

    AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

    if (HighlightedActor && HighlightedActor != HitActor)
    {
        IInteractable::Execute_SetHighlighted(HighlightedActor, false);
        HighlightedActor = nullptr;
    }

    if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        if (HitActor != HighlightedActor)
        {
            IInteractable::Execute_SetHighlighted(HitActor, true);
            HighlightedActor = HitActor;
        }
    }
}

void ADefaultCharacter::InitializeAbilitySystem()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (PS)
    {
        AbilitySystemComponent = PS->GetAbilitySystemComponent();
        if (AbilitySystemComponent)
        {
            AbilitySystemComponent->InitAbilityActorInfo(PS, this);
        }
    }
}

void ADefaultCharacter::GrantAbilitiesFromDefaultSet()
{
    if (!DefaultGASet) return;

    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();

    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultGASet->GrantedAbilities)
    {
        if (!AbilityClass) continue;
        FGameplayAbilitySpec Spec(AbilityClass, 1, 0);
        ASC->GiveAbility(Spec);
    }
}


void ADefaultCharacter::GrantAbilitiesFromInputSet()
{
    if (!AbilityInputSet) return;
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (!ASC) return;
    for (const FAbilityInputSetEntry& Entry : AbilityInputSet->Abilities)
    {
        if (!Entry.AbilityClass) continue;
        FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
        if (Entry.InputTag.IsValid())
        {
            Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
        }
        ASC->GiveAbility(Spec);

    }
}

UAbilitySystemComponent* ADefaultCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ADefaultCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &ADefaultCharacter::OnInventoryChanged);
    }
}

void ADefaultCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    CameraComponent = FindComponentByClass<UCameraComponent>();
    if (CameraComponent)
    {
        CameraDefaultLocation = CameraComponent->GetRelativeLocation();
        CameraDefaultRotation = CameraComponent->GetRelativeRotation();
    }

    UpdateEquippedItemMesh();
    if (AimCameraTimeline && AimCameraCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("OnAimCameraTimelineUpdate"));
        AimCameraTimeline->AddInterpFloat(AimCameraCurve, ProgressFunction);

        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnAimCameraTimelineFinished"));
        AimCameraTimeline->SetTimelineFinishedFunc(FinishedFunction);

        AimCameraTimeline->SetLooping(false);
    }
}

void ADefaultCharacter::OnAimCameraTimelineUpdate(float Value)
{
    if (CameraComponent)
    {
        FVector NewLocation = FMath::Lerp(CameraDefaultLocation, CameraAimLocation, Value);
        FRotator NewRotation = FMath::Lerp(CameraDefaultRotation, CameraAimRotation, Value);
        CameraComponent->SetRelativeLocation(NewLocation);
        CameraComponent->SetRelativeRotation(NewRotation);

    }
}

void ADefaultCharacter::OnAimCameraTimelineFinished()
{
}


void ADefaultCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (IsLocallyControlled())
    {
        UpdateInteractHighlight();
    }
}

void ADefaultCharacter::OnInventoryChanged(const TArray<FInventorySlot>& Slots)
{
    UpdateEquippedItemMesh();
}

void ADefaultCharacter::UpdateEquippedItemMesh()
{
    if (!InventoryComponent || !EquippedItemMeshComp || !HasAuthority()) return;

    FInventorySlot ActiveSlot = InventoryComponent->GetActiveItem();
    UItemBase* ItemAsset = ActiveSlot.ItemAsset;

    if (ItemAsset && ItemAsset->ItemMesh)
    {
        EquippedItemMeshComp->SetStaticMesh(ItemAsset->ItemMesh);
        EquippedItemMeshComp->SetWorldScale3D(ItemAsset->PickupMeshScale);
        EquippedItemMeshComp->SetRelativeRotation(ItemAsset->PickupMeshRotation);
		EquippedItemMeshComp->SetRelativeLocation(FVector(-0.000000,0.500000,2.208336));
        EquippedItemMeshComp->SetRelativeRotation(FRotator(0.528160,-3.449450,8.694707));
        //EquippedItemMeshComp->SetOwnerNoSee(true);
    }
    else
    {
        // Clear the mesh if no item equipped!
        EquippedItemMeshComp->SetStaticMesh(nullptr);
    }
}


void ADefaultCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
    if (!HasAuthority()) return;
    GrantAbilitiesFromInputSet();
    GrantAbilitiesFromDefaultSet();
}

void ADefaultCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
}

void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Look);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADefaultCharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADefaultCharacter::StopJumping);
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ADefaultCharacter::StartCrouch);
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ADefaultCharacter::StopCrouching);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADefaultCharacter::ServerStartSprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADefaultCharacter::ServerStopSprint);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ADefaultCharacter::ServerHandleInteract);
        EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Completed, this, &ADefaultCharacter::DropActiveItem);

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

void ADefaultCharacter::HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag)
{

    if (AbilitySystemComponent)
    {
        for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
        {

            if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
            {

                AbilitySystemComponent->TryActivateAbility(Spec.Handle);
            }
        }
    }
}

void ADefaultCharacter::HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag)
{
    if (AbilitySystemComponent)
    {
        for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
        {
            if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && Spec.IsActive())
            {
                AbilitySystemComponent->CancelAbilityHandle(Spec.Handle);
            }
        }
    }
}
void ADefaultCharacter::HandleNumberKey(FKey PressedKey)
{
    int32 SlotIndex = -1;
    if (PressedKey == EKeys::One) SlotIndex = 0;
    else if (PressedKey == EKeys::Two) SlotIndex = 1;
    else if (PressedKey == EKeys::Three) SlotIndex = 2;
    else if (PressedKey == EKeys::Four) SlotIndex = 3;
    else if (PressedKey == EKeys::Five) SlotIndex = 4;
    else if (PressedKey == EKeys::Six) SlotIndex = 5;
    else if (PressedKey == EKeys::Seven) SlotIndex = 6;
    else if (PressedKey == EKeys::Eight) SlotIndex = 7;
    else if (PressedKey == EKeys::Nine) SlotIndex = 8;
    else if (PressedKey == EKeys::Zero) SlotIndex = 9;

    if (SlotIndex != -1 && InventoryComponent)
    {
        InventoryComponent->ServerSetActiveSlot(SlotIndex);
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

bool ADefaultCharacter::GetForwardTraceResult(float TraceDistance, FHitResult& OutHit, FVector& OutTraceEnd) const
{
    if (!CameraComponent) return false;

    FVector Start = CameraComponent->GetComponentLocation();
    FRotator ControlRot = Controller ? Controller->GetControlRotation() : CameraComponent->GetComponentRotation();
    FVector End = Start + ControlRot.Vector() * TraceDistance;
    OutTraceEnd = End;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void ADefaultCharacter::DropActiveItem()
{
    if (!InventoryComponent) return;

    FHitResult Hit;
    FVector TraceEnd;
    FVector DropLocation;

    if (GetForwardTraceResult(300.f, Hit, TraceEnd))
    {
        DropLocation = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
    }
    else
    {
        DropLocation = TraceEnd;
    }

    FHitResult DownHit;
    FVector DownTraceStart = DropLocation + FVector(0, 0, 50);
    FVector DownTraceEnd = DropLocation - FVector(0, 0, 200);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(DownHit, DownTraceStart, DownTraceEnd, ECC_Visibility, Params))
    {
        DropLocation = DownHit.Location;
    }

    InventoryComponent->ServerDropActiveItem(DropLocation);
}

void ADefaultCharacter::ServerHandleInteract_Implementation()
{
    FHitResult Hit;
    FVector TraceEnd;
    if (GetForwardTraceResult(300.f, Hit, TraceEnd))
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor)
            return;

        if (HitActor->GetClass()->ImplementsInterface(URequiresItem::StaticClass()))
        {
            UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
            UItemBase* ActiveItem = nullptr;
            if (Inventory)
            {
                FInventorySlot ActiveSlot = Inventory->GetActiveItem();
                ActiveItem = ActiveSlot.ItemAsset;
            }

            if (!IRequiresItem::Execute_IsCorrectItem(HitActor, ActiveItem))
            {
                return;
            }

            if (ActiveItem) ActiveItem->OnUsed(this);
        }

        if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_Interact(HitActor, this);
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

void ADefaultCharacter::StartCrouch()
{
    Crouch();
}

void ADefaultCharacter::StopCrouching()
{
    UnCrouch();
}

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

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
        if (CameraComponent) {
            ServerCameraRotationUpdate(CameraComponent->GetComponentRotation().Pitch);
        }
    }
}

void ADefaultCharacter::ServerCameraRotationUpdate_Implementation(float NewPitch) {
    Pitch = NewPitch;
    OnRep_Pitch();
}

void ADefaultCharacter::OnRep_Pitch() {
    if (CameraComponent) {
        FRotator NewRotation = FRotator(Pitch, CameraComponent->GetComponentRotation().Yaw, CameraComponent->GetComponentRotation().Roll);
        CameraComponent->SetWorldRotation(NewRotation);
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
    if (bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = 500.f;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = 250.f;
    }
}

void ADefaultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADefaultCharacter, bIsSprinting);
    DOREPLIFETIME(ADefaultCharacter, Pitch);
}