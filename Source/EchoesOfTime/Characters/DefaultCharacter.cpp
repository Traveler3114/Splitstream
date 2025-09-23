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
}

void ADefaultCharacter::UpdateInteractHighlight()
{
    // Only the local player should handle highlighting logic
    if (!IsLocallyControlled())
        return;

    FHitResult Hit;
    FVector TraceEnd;
    bool bHit = GetForwardTraceResult(300.f, Hit, TraceEnd);

    AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

    // Remove highlight from previous actor if it's different from the one we're looking at
    if (HighlightedActor && HighlightedActor != HitActor)
    {
        IInteractable::Execute_SetHighlighted(HighlightedActor, false);
        HighlightedActor = nullptr;
    }

    // Highlight the new actor if valid and implements the interactable interface
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
        PS->GiveAbilities();
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
    if (!CameraComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraComponent not found!"));
    }

    UpdateEquippedItemActor();
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
    UpdateEquippedItemActor();
}

void ADefaultCharacter::UpdateEquippedItemActor()
{
    if (!HasAuthority()) return; // Only the server should do this

    // Destroy previous equipped actor if any
    if (EquippedItemActor)
    {
        EquippedItemActor->Destroy();
        EquippedItemActor = nullptr;
    }

    if (!InventoryComponent) return;

    FInventorySlot ActiveSlot = InventoryComponent->GetActiveItem();
    UItemBase* ItemAsset = ActiveSlot.ItemAsset;

    if (ItemAsset)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        EquippedItemActor = World->SpawnActor<AItemPickup>(AItemPickup::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (EquippedItemActor)
        {
            EquippedItemActor->InitFromItemData(ItemAsset, ActiveSlot.ItemInstanceID);
            EquippedItemActor->SetActorEnableCollision(false); // No collision
            EquippedItemActor->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("HandGrip_R")
            );
        }

        // IMPORTANT: Mark EquippedItemActor for replication
        EquippedItemActor->SetReplicates(true);
    }
}

void ADefaultCharacter::OnRep_EquippedItemActor()
{
    if (EquippedItemActor)
    {
        EquippedItemActor->SetActorEnableCollision(false); // Ensure no collision on clients
        EquippedItemActor->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("HandGrip_R")
        );
    }
}


void ADefaultCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeAbilitySystem();
    if (!HasAuthority()) return;
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ACharacterHUD* HUD = Cast<ACharacterHUD>(PC->GetHUD()))
        {
            HUD->BindTags(this);
        }
    }
}

void ADefaultCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeAbilitySystem();
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ACharacterHUD* HUD = Cast<ACharacterHUD>(PC->GetHUD()))
        {
            HUD->BindTags(this);
        }
    }
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
        EnhancedInputComponent->BindAction(PastEchoAction, ETriggerEvent::Completed, this, &ADefaultCharacter::ActivateFutureGAPastEcho);
        EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Completed, this, &ADefaultCharacter::DropActiveItem);
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
    FVector End = Start + CameraComponent->GetForwardVector() * TraceDistance;
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

    // Add this: Downward trace to find the topmost surface
    FHitResult DownHit;
    FVector DownTraceStart = DropLocation + FVector(0, 0, 50); // 50 units above
    FVector DownTraceEnd = DropLocation - FVector(0, 0, 200); // 200 units below

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

        // If it requires an item, check for it using the interface method
        if (HitActor->GetClass()->ImplementsInterface(URequiresItem::StaticClass()))
        {
            UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
            UItemBase* ActiveItem = nullptr;
            if (Inventory)
            {
                FInventorySlot ActiveSlot = Inventory->GetActiveItem();
                ActiveItem = ActiveSlot.ItemAsset;
            }

            // Let the interactable actor decide if the item is correct!
            if (!IRequiresItem::Execute_IsCorrectItem(HitActor, ActiveItem))
            {
                // Optionally give feedback for missing/wrong item
                return;
            }

            // Optionally:
            if (ActiveItem) ActiveItem->OnUsed(this);
            // You can also let the interactable handle OnUsed if you prefer!
        }

        // Now call Interact (item requirement is fulfilled or not needed)
        if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_Interact(HitActor, this);
        }
    }
}

void ADefaultCharacter::ActivateFutureGAPastEcho()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (PS && PS->GetAbilitySystemComponent())
    {
        const FGameplayTag MyTag = TAG_Character_Ability_Future_PastEcho;
        PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(MyTag));
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
    if (!PS) return; // Fix: Guard against null PlayerState

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
    DOREPLIFETIME(ADefaultCharacter, EquippedItemActor);
}