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
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/AttributeSets/PlayerAttributeSet.h"
#include "AbilitySystem/EOTGameplayTags.h"
#include "Interfaces/IDetectable.h"
#include "EngineUtils.h"
#include "Controllers/DefaultPlayerController.h"
#include "ActorComponents/HackComponent.h"
#include "ActorComponents/SearchComponent.h"
#include "ActorComponents/LockPickComponent.h"
#include "TimerManager.h"


ADefaultCharacter::ADefaultCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetMesh()->SetOwnerNoSee(false);

    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->JumpZVelocity = 300.f;
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
    EquippedItemMeshComp->SetupAttachment(GetMesh(), TEXT("HandGrip_R"));
    EquippedItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

    // Start detection timer for performance
    GetWorldTimerManager().SetTimer(DetectionUpdateTimerHandle, this, &ADefaultCharacter::UpdateDetectionTimer, 0.05f, true);
}

void ADefaultCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(DetectionUpdateTimerHandle);
}

void ADefaultCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only run interact highlight every frame!
    if (IsLocallyControlled())
    {
        UpdateInteractHighlight();
    }
}

// --- PERFORMANCE: Detection logic moved to timer-based update
void ADefaultCharacter::UpdateDetectionTimer()
{
    float DeltaTime = 0.05f; // Matches timer interval
    TArray<AActor*> ToRemove;

    for (auto& Elem : DetectionProgressMap)
    {
        AActor* Detector = Elem.Key;
        float& Progress = Elem.Value;

        bool bInVision = Detector && Detector->GetClass()->ImplementsInterface(UDetectable::StaticClass())
            && IDetectable::Execute_IsActorAlreadyDetected(Detector, this);

        bool bIsIllegal = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(TAG_Character_Status_Illegal);

        float Rate = DeltaTime * 0.5f;

        if (bInVision && bIsIllegal)
        {
            Progress += Rate;
            if (Progress >= 1.f)
            {
                Progress = 1.f;
                if (Detector && Detector->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
                {
                    IDetectable::Execute_OnFullyDetected(Detector, this);
                }
                OnFullyDetected_Implementation(Detector);
            }
        }
        else
        {
            Progress -= Rate;
            if (Progress <= 0.f) Progress = 0.f;
        }

        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            if (ADefaultPlayerController* DefPC = Cast<ADefaultPlayerController>(PC))
            {
                float Angle = CalculateDetectionAngle(
                    Detector->GetActorLocation(),
                    PC->PlayerCameraManager->GetCameraRotation(),
                    GetActorLocation());
                DefPC->ClientUpdateDetectionWidget(
                    Detector,
                    Progress,
                    Progress >= 1.0f,
                    Angle
                );
            }
        }

        if (Progress == 0.f)
        {
            ToRemove.Add(Detector);
        }
    }

    for (AActor* Detector : ToRemove)
    {
        DetectionProgressMap.Remove(Detector);
    }
}

// ---------------- ABILITY SYSTEM ------------------
void ADefaultCharacter::InitializeAbilitySystem()
{
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (PS)
    {
        AbilitySystemComponent = PS->GetAbilitySystemComponent();
        if (AbilitySystemComponent)
        {
            AbilitySystemComponent->InitAbilityActorInfo(PS, this);
            AbilitySystemComponent->RegisterGameplayTagEvent(
                TAG_Character_Status_Illegal,
                EGameplayTagEventType::NewOrRemoved
            ).AddUObject(this, &ADefaultCharacter::OnIllegalTagChanged);
            
            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
                UPlayerAttributeSet::GetWalkSpeedAttribute()
            ).AddUObject(this, &ADefaultCharacter::OnWalkSpeedChanged);

            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
                UPlayerAttributeSet::GetRunSpeedAttribute()
            ).AddUObject(this, &ADefaultCharacter::OnRunSpeedChanged);

            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
                UPlayerAttributeSet::GetCrouchSpeedAttribute()
            ).AddUObject(this, &ADefaultCharacter::OnCrouchSpeedChanged);
        }
    }
    if (HasAuthority() && AttributeInitGE)
    {
        UAbilitySystemComponent* ServerASC = AbilitySystemComponent;
        if (ServerASC && ServerASC->GetOwnerRole() == ROLE_Authority)
        {
            FGameplayEffectContextHandle EffectContext = ServerASC->MakeEffectContext();
            FGameplayEffectSpecHandle SpecHandle = ServerASC->MakeOutgoingSpec(AttributeInitGE, 1.f, EffectContext);
            if (SpecHandle.IsValid())
            {
                ServerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
}

void ADefaultCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
    // Only update this if NOT sprinting/crouching
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
        
    GetCharacterMovement()->MaxWalkSpeedCrouched = ChangeData.NewValue; // Always update this property
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
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    if (!PS) return;
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    if (!ASC) return;

    // Grant future abilities ONLY if Team.Future tag
    if (FutureGASet && ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Team.Future")))
    {
        for (const FAbilityInputSetEntry& Entry : FutureGASet->Abilities)
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

    // Grant solo abilities ONLY if there is only one player
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Team.Solo")))
    {
        for (const FAbilityInputSetEntry& Entry : SoloGASet->Abilities)
        {
            if (!Entry.AbilityClass) continue;
            FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
            if (Entry.InputTag.IsValid())
            {
                Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
            }
            ASC->GiveAbility(Spec);
        }
        for (const FAbilityInputSetEntry& Entry : FutureGASet->Abilities)
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
}


UAbilitySystemComponent* ADefaultCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

// ---------------- DETECTION MECHANICS ------------------
void ADefaultCharacter::OnDetected_Implementation(AActor* Detector)
{
    if (!Detector) return;

    if (!AbilitySystemComponent || !AbilitySystemComponent->HasMatchingGameplayTag(TAG_Character_Status_Illegal))
    {
        return;
    }

    if (!DetectionProgressMap.Contains(Detector))
    {
        DetectionProgressMap.Add(Detector, 0.f);
    }
}

void ADefaultCharacter::OnLost_Implementation(AActor* Detector)
{
    // (Optional UI update for controller/overlay)
}

void ADefaultCharacter::OnFullyDetected_Implementation(AActor* Detector)
{
    if (!Detector) return;
    if (DetectionProgressMap.Contains(Detector))
    {
        DetectionProgressMap[Detector] = 1.f;
        // Alarm logic, UI etc
    }
}

void ADefaultCharacter::OnIllegalTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (NewCount > 0)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
            {
                if (IDetectable::Execute_IsActorAlreadyDetected(*ActorItr, this))
                {
                    IDetectable::Execute_OnDetected(this, *ActorItr);
                }
            }
        }
    }
    else
    {
        // Don't remove detectors instantly; handled in detection timer.
    }
}

// ---------------- INVENTORY HANDLING ------------------
void ADefaultCharacter::OnInventoryChanged(const TArray<FInventorySlot>& Slots)
{
    UpdateEquippedItemMesh();
}

void ADefaultCharacter::UpdateEquippedItemMesh()
{
    if (!InventoryComponent || !EquippedItemMeshComp)
        return;

    FInventorySlot ActiveSlot = InventoryComponent->GetActiveItem();
    UItemBase* ItemAsset = ActiveSlot.ItemAsset;
    UStaticMesh* NewMesh = (ItemAsset && ItemAsset->ItemMesh) ? ItemAsset->ItemMesh : nullptr;

    if (EquippedItemMeshComp->GetStaticMesh() != NewMesh)
    {
        EquippedItemMeshComp->SetStaticMesh(nullptr);
        EquippedItemMeshComp->SetStaticMesh(NewMesh);
    }

    if (NewMesh && ItemAsset)
    {
        EquippedItemMeshComp->SetWorldScale3D(ItemAsset->PickupMeshScale);
        EquippedItemMeshComp->SetRelativeRotation(ItemAsset->PickupMeshRotation);
        EquippedItemMeshComp->SetRelativeLocation(FVector(-0.000000, 0.500000, 2.208336));
        EquippedItemMeshComp->SetRelativeRotation(FRotator(0.528160, -3.449450, 8.694707));
    }

    // 2) ALL (server + clients): compute ADS socket -> aim coordinates
    //    (by the time this runs on clients, the mesh from the server should have replicated)
    if (EquippedItemMeshComp->GetStaticMesh()) // defend against nullptr mesh on client
    {
        static const FName ADSSocket(TEXT("ADS"));
        if (EquippedItemMeshComp->DoesSocketExist(ADSSocket))
        {
            const FTransform SocketTransform =
                EquippedItemMeshComp->GetSocketTransform(ADSSocket, ERelativeTransformSpace::RTS_Component);

            CameraAimLocation = SocketTransform.GetLocation();
            CameraAimRotation = SocketTransform.Rotator();
        }
    }
}

// ---------------- INPUT ------------------
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
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::HandleInteractHoldStart);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ADefaultCharacter::HandleInteractHoldStop);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADefaultCharacter::HandleInteractInstant);
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

// ------------- INPUT HANDLING ---------------
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
    FString KeyNameStr = PressedKey.GetFName().ToString();
    int32 SlotIndex = -1;

    if (KeyNameStr.Len() == 1 && FChar::IsDigit(KeyNameStr[0]))
    {
        if (KeyNameStr[0] == '0')
            SlotIndex = 9;
        else
            SlotIndex = KeyNameStr[0] - '1'; // '1' is 0, '2' is 1, ..., '9' is 8
    }
    else
    {
        static const TMap<FName, int32> KeyToSlot {
            {EKeys::One.GetFName(), 0},
            {EKeys::Two.GetFName(), 1},
            {EKeys::Three.GetFName(), 2},
            {EKeys::Four.GetFName(), 3},
            {EKeys::Five.GetFName(), 4},
            {EKeys::Six.GetFName(), 5},
            {EKeys::Seven.GetFName(), 6},
            {EKeys::Eight.GetFName(), 7},
            {EKeys::Nine.GetFName(), 8},
            {EKeys::Zero.GetFName(), 9}
        };
        const int32* Found = KeyToSlot.Find(PressedKey.GetFName());
        if (Found)
            SlotIndex = *Found;
    }

    if (SlotIndex >= 0 && SlotIndex < 10 && InventoryComponent)
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

// ----------- CONTROLLED TRACE ---------------
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

// ----------- INVENTORY ACTIONS --------------
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

// ------------ INTERACT MECHANICS -----------
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


void ADefaultCharacter::HandleInteractHoldStart()
{
    FHitResult Hit;
    FVector TraceEnd;
    if (!GetForwardTraceResult(300.f, Hit, TraceEnd)) return;
    AActor* HitActor = Hit.GetActor();
    if (!HitActor) return;

    if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()) &&
        IInteractable::Execute_IsProgressiveInteract(HitActor))
    {
        IInteractable::Execute_Interact(HitActor, this);
        ProgressiveActor = HitActor;
    }
}

void ADefaultCharacter::HandleInteractHoldStop()
{
    if (ProgressiveActor)
    {
        IInteractable::Execute_CancelInteract(ProgressiveActor, this);
        ProgressiveActor = nullptr;
    }
}

void ADefaultCharacter::HandleInteractInstant()
{
    if (HasAuthority() && IsLocallyControlled())
    {
        FHitResult Hit;
        FVector TraceEnd;
        if (GetForwardTraceResult(300.f, Hit, TraceEnd))
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor)
            {
                ServerHandleInteract(HitActor);
            }
        }
        return;
    }

    FHitResult Hit;
    FVector TraceEnd;
    if (GetForwardTraceResult(300.f, Hit, TraceEnd))
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor)
            return;

        if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            bool bRequiresItem = IInteractable::Execute_RequiresItem(HitActor);

            UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
            if (Inventory)
            {
                FInventorySlot ActiveSlot = Inventory->GetActiveItem();
                UItemBase* ActiveItem = ActiveSlot.ItemAsset;

                if (bRequiresItem)
                {
                    // If requires item, check for correct item
                    if (HasAuthority() && !IInteractable::Execute_IsCorrectItem(HitActor, ActiveItem))
                    {
                        return;
                    }
                    if (ActiveItem)
                    {
                        ActiveItem->OnUsed(this, ActiveSlot.ItemInstanceID);
                    }
                }
            }

            // If does NOT require item, just interact
            IInteractable::Execute_Interact(HitActor, this);

            // Always call server to handle authoritative interaction
            ServerHandleInteract(HitActor);
        }
    }
}
void ADefaultCharacter::ServerHandleInteract_Implementation(AActor* TargetActor)
{
    if (!TargetActor)
        return;

    if (TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        bool bRequiresItem = IInteractable::Execute_RequiresItem(TargetActor);

        UInventoryComponent* Inventory = FindComponentByClass<UInventoryComponent>();
        if (Inventory)
        {
            FInventorySlot ActiveSlot = Inventory->GetActiveItem();
            UItemBase* ActiveItem = ActiveSlot.ItemAsset;

            if (bRequiresItem)
            {
                if (!IInteractable::Execute_IsCorrectItem(TargetActor, ActiveItem))
                {
                    return;
                }
                if (ActiveItem)
                {
                    ActiveItem->OnUsed(this, ActiveSlot.ItemInstanceID);
                }
            }
        }

        // If does NOT require item, just interact
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
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
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

    // Set crouch speed from GAS attribute
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

    // Restore speed from GAS attribute
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

// ----------- SPRINT & CAMERA ROTATION -----------
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
    // Always update crouch speed property
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
    GrantAbilitiesFromInputSet();
    GrantAbilitiesFromDefaultSet();
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

// ------------- BlueprintPure (Detection Angle) -------------
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