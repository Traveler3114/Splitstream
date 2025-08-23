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
#include "InventorySystem/InventoryComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"



ADefaultCharacter::ADefaultCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set default character collision size
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	GetMesh()->SetOwnerNoSee(false); // Make sure the mesh is visible in first-person

	// Prevent character rotation from affecting the camera
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Character movement settings
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	bReplicates = true;
	SetReplicateMovement(true);

	// Initialize sprint state
	bIsSprinting = false;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void ADefaultCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


void ADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();


	// Setup input mapping context
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
}

void ADefaultCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server init
	if (!HasAuthority()) return;

	ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->GetAbilitySystemComponent())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		PS->GiveAbilities();
	}
}

void ADefaultCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client init
	ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->GetAbilitySystemComponent())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
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

	// Bind number keys 1-9 to select inventory slots 0-8
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
	int32 SlotIndex = SlotNumber - 1; // Convert player-facing 1-9 to 0-8 index
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
	FRotator DropRotation = FRotator::ZeroRotator;

	if (GetForwardTraceResult(300.f, Hit, TraceEnd))
	{
		DropLocation = Hit.bBlockingHit ? Hit.ImpactPoint : TraceEnd;
	}
	else
	{
		DropLocation = TraceEnd;
	}

	InventoryComponent->ServerDropActiveItem(DropLocation);
}

void ADefaultCharacter::ServerHandleInteract_Implementation()
{
	FHitResult Hit;
	FVector TraceEnd;
	if (GetForwardTraceResult(300.f, Hit, TraceEnd))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
			{
				IInteractable::Execute_Interact(HitActor, this);
			}
		}
	}
}

void ADefaultCharacter::ActivateFutureGAPastEcho()
{
	ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
	if (PS && PS->GetAbilitySystemComponent())
	{
		const FGameplayTag MyTag = FGameplayTag::RequestGameplayTag(FName("Character.Ability.Future.PastEcho"));
		PS->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(MyTag));
	}
}

void ADefaultCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		// Stop sprinting if moving sideways or backward
		if (MovementVector.Y <= 0 || MovementVector.X != 0)
		{
			ServerStopSprint();
		}
	}
}

void ADefaultCharacter::Look(const FInputActionValue& Value)
{
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
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
	}
}

void ADefaultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADefaultCharacter, bIsSprinting);
	DOREPLIFETIME(ADefaultCharacter, Pitch);
}