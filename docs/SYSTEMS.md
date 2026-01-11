# Systems Overview

This document provides a detailed overview of the major systems in Echoes of Time and how they work together.

---

## Table of Contents

- [Gameplay Ability System](#gameplay-ability-system)
- [Character System](#character-system)
- [AI and Detection System](#ai-and-detection-system)
- [Timeline System](#timeline-system)
- [Inventory System](#inventory-system)
- [Networking System](#networking-system)
- [Input System](#input-system)
- [Mini-Game Systems](#mini-game-systems)
- [Save System](#save-system)

---

## Gameplay Ability System

### Overview

The Gameplay Ability System (GAS) is Unreal's framework for implementing character abilities, attributes, and gameplay effects.

### Components

#### 1. Ability System Component

**Class:** `UDefaultAbilitySystemComponent`

**Location:** `Source/EchoesOfTime/AbilitySystem/`

**Responsibilities:**
- Manages granted abilities
- Handles ability activation and cancellation
- Applies and manages gameplay effects
- Tracks gameplay tags
- Replicates ability state

**Key Functions:**
```cpp
// Grant an ability to the character
void GiveAbility(const FGameplayAbilitySpec& Spec);

// Try to activate ability by tag
bool TryActivateAbilitiesByTag(const FGameplayTagContainer& Tags);

// Apply gameplay effect
FActiveGameplayEffectHandle ApplyGameplayEffectToSelf(
    const UGameplayEffect* Effect);
```

#### 2. Attribute Sets

**Class:** `UPlayerAttributeSet`

**Attributes:**
- **Health**: Character health points
- **MaxHealth**: Maximum health
- **Stamina**: Resource for abilities
- **MaxStamina**: Maximum stamina
- **WalkSpeed**: Walking movement speed
- **RunSpeed**: Running movement speed
- **CrouchSpeed**: Crouching movement speed

**Replication:**
All attributes are replicated for multiplayer synchronization.

**Usage:**
```cpp
// Get current health
float Health = PlayerAttributeSet->GetHealth();

// Listen to attribute changes
ASC->GetGameplayAttributeValueChangeDelegate(
    UPlayerAttributeSet::GetHealthAttribute()
).AddUObject(this, &AMyClass::OnHealthChanged);
```

#### 3. Gameplay Abilities

**Base Class:** `UGameplayAbility`

**Existing Abilities:**
- `DefaultGAHack`: Hacking ability
- `DefaultGALockPick`: Lockpicking ability
- `FutureGAPastEcho`: Timeline echo ability

**Ability Lifecycle:**
```
Granted → Activated → Executing → Ended → Committed
```

**Creating New Ability:**
```cpp
UCLASS()
class UMyGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()
    
public:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override
    {
        // Check costs and cooldowns
        if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
        
        // Execute ability logic
        PerformAbilityAction();
        
        // End ability
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
    
protected:
    void PerformAbilityAction()
    {
        // Ability-specific logic
    }
};
```

#### 4. Gameplay Effects

**Purpose:** Modify attributes temporarily or permanently

**Types:**
- **Instant**: Immediate one-time change (e.g., damage)
- **Duration**: Temporary effect with duration (e.g., speed boost)
- **Infinite**: Permanent effect (e.g., stat upgrades)

**Example:**
```cpp
// Create instant damage effect
UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>();
DamageEffect->DurationPolicy = EGameplayEffectDurationType::Instant;
DamageEffect->Modifiers.Add(FGameplayModifierInfo(
    UPlayerAttributeSet::GetHealthAttribute(),
    EGameplayModOp::Additive,
    FGameplayEffectModifierMagnitude(-10.0f)
));

// Apply to target
ASC->ApplyGameplayEffectToSelf(DamageEffect, 1.0f, Context);
```

#### 5. Gameplay Tags

**Class:** `EOTGameplayTags`

**Purpose:** Tag-based identification and filtering

**Examples:**
```cpp
// Ability tags
GameplayTag.Ability.Hack
GameplayTag.Ability.LockPick
GameplayTag.Ability.TimelineSwitch

// State tags
GameplayTag.State.Detected
GameplayTag.State.Crouching
GameplayTag.State.Hacking

// Timeline tags
GameplayTag.Timeline.Past
GameplayTag.Timeline.Future
```

---

## Character System

### Player Character

**Class:** `ADefaultCharacter`

**Location:** `Source/EchoesOfTime/Characters/`

#### Core Components

```cpp
// Camera
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UCameraComponent* FirstPersonCamera;

// Equipped item visual
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UStaticMeshComponent* EquippedItemMesh;

// Gameplay systems
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UInventoryComponent* InventoryComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UAbilitySystemComponent* AbilitySystemComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UDetectionComponent* DetectionComponent;
```

#### Movement System

**Speed Attributes:**
Controlled by Gameplay Ability System attributes:
- Walk Speed
- Run Speed  
- Crouch Speed

**Implementation:**
```cpp
void ADefaultCharacter::OnWalkSpeedChanged(
    const FOnAttributeChangeData& ChangeData)
{
    GetCharacterMovement()->MaxWalkSpeed = ChangeData.NewValue;
}
```

#### Interaction System

**Interface:** `IInteractable`

**Process:**
1. Raycast from camera to detect interactive objects
2. Display interaction prompt if object implements `IInteractable`
3. On input, call `Interact()` on the object
4. Replicate to server if multiplayer

```cpp
void ADefaultCharacter::PerformInteraction()
{
    FHitResult Hit;
    FVector Start = FirstPersonCamera->GetComponentLocation();
    FVector End = Start + (FirstPersonCamera->GetForwardVector() * InteractionRange);
    
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
    {
        if (Hit.GetActor()->Implements<UInteractable>())
        {
            IInteractable::Execute_Interact(Hit.GetActor(), this);
        }
    }
}
```

### AI Characters

**Base Class:** `AAICharacter`

**Derived Classes:**
- `AGuardCharacter`: Past timeline guards
- `ARobotGuardCharacter`: Future timeline robot guards
- `ACivilianCharacter`: Non-hostile NPCs

#### AI Controller

Uses State Tree for behavior:
- Patrol routes
- Detection and investigation
- Alert and chase
- Search patterns

### Drone Pawn

**Class:** `ADronePawn`

**Features:**
- Alternative gameplay mode
- Flight mechanics
- Limited interaction capabilities
- Different camera perspective

---

## AI and Detection System

### Detection Component

**Class:** `UDetectionComponent`

**Location:** `Source/EchoesOfTime/ActorComponents/`

#### Detection Mechanics

**Vision Cone:**
```cpp
bool IsInVisionCone(const FVector& TargetLocation) const
{
    FVector DirectionToTarget = (TargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
    float DotProduct = FVector::DotProduct(GetOwner()->GetActorForwardVector(), DirectionToTarget);
    float Angle = FMath::Acos(DotProduct);
    return Angle <= VisionHalfAngle;
}
```

**Line of Sight:**
```cpp
bool HasLineOfSight(AActor* Target) const
{
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    
    return !GetWorld()->LineTraceSingleByChannel(
        Hit,
        GetOwner()->GetActorLocation(),
        Target->GetActorLocation(),
        ECC_Visibility,
        Params
    );
}
```

**Distance Check:**
- Close range: Immediate detection
- Medium range: Gradual detection
- Long range: Requires line of sight

#### Alert Levels

1. **Unaware (0%)**: Normal patrol
2. **Suspicious (25%)**: Investigating sound/movement
3. **Investigating (50%)**: Moving to last known position
4. **Alert (75%)**: Player detected, chasing
5. **Combat (100%)**: Active combat state

### Search Component

**Class:** `USearchComponent`

**Behavior:**
- Search pattern around last known position
- Gradually expand search radius
- Return to patrol if player not found
- Can be called for backup by other AI

### State Tree

**Visual AI Programming:**
- State-based behavior
- Transitions based on conditions
- Reusable states and tasks
- Designer-friendly

**Common States:**
- Idle/Patrol
- Investigate
- Chase
- Search
- Combat

---

## Timeline System

### Concept

The game features two parallel timelines that affect the world state:

**Past Timeline:**
- Historical setting
- Different architecture and props
- Past-era guards
- Certain objects accessible

**Future Timeline:**
- Futuristic setting
- Advanced technology
- Robot guards
- Different puzzle solutions

### Implementation

**Enum:**
```cpp
UENUM(BlueprintType)
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};
```

**Timeline Switching:**
- Triggered by player ability
- Level streaming for timeline-specific content
- State preservation across switches
- Visual effects for transition

**Timeline-Aware Actors:**
```cpp
UCLASS()
class ATimelineActor : public AActor
{
    UPROPERTY(EditAnywhere)
    ETimelineEra VisibleInTimeline;
    
    UPROPERTY(EditAnywhere)
    bool bActiveInPast;
    
    UPROPERTY(EditAnywhere)
    bool bActiveInFuture;
    
    void OnTimelineChanged(ETimelineEra NewTimeline)
    {
        bool bShouldBeVisible = (NewTimeline == VisibleInTimeline);
        SetActorHiddenInGame(!bShouldBeVisible);
        SetActorEnableCollision(bShouldBeVisible);
    }
};
```

---

## Inventory System

### Inventory Component

**Class:** `UInventoryComponent`

**Location:** `Source/EchoesOfTime/ActorComponents/`

#### Data Structure

```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UItemDataAsset* Item;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsEquipped;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SlotIndex;
};
```

#### Key Features

**Add Item:**
```cpp
bool UInventoryComponent::AddItem(UItemDataAsset* Item, int32 Quantity)
{
    // Check for existing stack
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.Item == Item && Slot.Item->bIsStackable)
        {
            Slot.Quantity += Quantity;
            OnInventoryChanged.Broadcast(InventorySlots);
            return true;
        }
    }
    
    // Find empty slot
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.Item == nullptr)
        {
            Slot.Item = Item;
            Slot.Quantity = Quantity;
            OnInventoryChanged.Broadcast(InventorySlots);
            return true;
        }
    }
    
    return false; // Inventory full
}
```

**Equipment System:**
```cpp
void UInventoryComponent::EquipItem(int32 SlotIndex)
{
    // Unequip current item
    if (EquippedSlotIndex != -1)
    {
        InventorySlots[EquippedSlotIndex].bIsEquipped = false;
    }
    
    // Equip new item
    InventorySlots[SlotIndex].bIsEquipped = true;
    EquippedSlotIndex = SlotIndex;
    
    OnItemEquipped.Broadcast(InventorySlots[SlotIndex].Item);
}
```

**Replication:**
```cpp
void UInventoryComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UInventoryComponent, InventorySlots);
    DOREPLIFETIME(UInventoryComponent, EquippedSlotIndex);
}
```

---

## Networking System

### Architecture

**Model:** Server-authoritative with client prediction

**Components:**
- Game Instance: Persistent data across maps
- Game Mode: Server-only game rules
- Game State: Replicated game state
- Player State: Replicated player data
- Player Controller: Player input and camera
- Pawn/Character: Player representation

### Replication

#### Property Replication

```cpp
// Mark property for replication
UPROPERTY(Replicated, BlueprintReadOnly)
int32 PlayerScore;

// Implement GetLifetimeReplicatedProps
void AMyActor::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMyActor, PlayerScore);
    DOREPLIFETIME_CONDITION(AMyActor, PrivateData, COND_OwnerOnly);
}
```

#### RPC (Remote Procedure Calls)

**Server RPC:**
```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerInteract(AActor* Target);

void ADefaultCharacter::ServerInteract_Implementation(AActor* Target)
{
    // Server-side logic
    if (Target && Target->Implements<UInteractable>())
    {
        IInteractable::Execute_Interact(Target, this);
    }
}

bool ADefaultCharacter::ServerInteract_Validate(AActor* Target)
{
    // Validate input
    return Target != nullptr && 
           FVector::Distance(GetActorLocation(), Target->GetActorLocation()) < InteractionRange;
}
```

**Client RPC:**
```cpp
UFUNCTION(Client, Reliable)
void ClientPlayEffect();

void ADefaultCharacter::ClientPlayEffect_Implementation()
{
    // Client-side effect
    PlaySound(EffectSound);
    SpawnParticles(EffectParticles);
}
```

**Multicast RPC:**
```cpp
UFUNCTION(NetMulticast, Reliable)
void MulticastExplosion(FVector Location);

void AMyActor::MulticastExplosion_Implementation(FVector Location)
{
    // Plays on all clients
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Location);
}
```

### Session Management

**Advanced Sessions Plugin:**

```cpp
// Create session
UAdvancedSessionsLibrary::CreateSession(
    PlayerController,
    PublicConnections,
    bUseLAN,
    AdditionalSettings
);

// Find sessions
UAdvancedSessionsLibrary::FindSessions(
    PlayerController,
    MaxResults,
    bUseLAN
);

// Join session
UAdvancedSessionsLibrary::JoinSession(
    PlayerController,
    SessionResult
);
```

---

## Input System

### Enhanced Input

**Components:**
- Input Actions: Define what actions exist
- Input Mapping Contexts: Map inputs to actions
- Input Modifiers: Modify input values
- Input Triggers: Define when action triggers

#### Input Actions

Defined in `Content/Input/`:
- `IA_Move`: Movement input (2D vector)
- `IA_Look`: Camera look (2D vector)
- `IA_Jump`: Jump action
- `IA_Interact`: Interaction
- `IA_Ability1`: Primary ability
- `IA_Ability2`: Secondary ability

#### Binding Input

```cpp
void ADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind move action
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Move);
        
        // Bind look action
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADefaultCharacter::Look);
        
        // Bind jump action
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
    }
}
```

---

## Mini-Game Systems

### Firewall Mini-Game

**Class:** `AFirewallMiniGame`

**Gameplay:**
- Grid-based puzzle
- Navigate through nodes
- Avoid detection
- Time-limited

### Neon Runner Mini-Game

**Class:** `ANeonRunnerMiniGame`

**Gameplay:**
- Timing-based challenge
- Rhythm mechanics
- Score-based

---

## Save System

**Location:** `Source/EchoesOfTime/Saving/`

### Save Game Data

```cpp
UCLASS()
class USaveGame : public UGameSaveGame
{
    GENERATED_BODY()
    
public:
    UPROPERTY()
    FVector PlayerLocation;
    
    UPROPERTY()
    FRotator PlayerRotation;
    
    UPROPERTY()
    TArray<FInventorySlot> Inventory;
    
    UPROPERTY()
    ETimelineEra CurrentTimeline;
    
    UPROPERTY()
    TMap<FString, bool> CompletedObjectives;
};
```

### Save/Load Operations

```cpp
// Save game
void USaveGameSubsystem::SaveGame()
{
    USaveGame* SaveGameInstance = Cast<USaveGame>(
        UGameplayStatics::CreateSaveGameObject(USaveGame::StaticClass())
    );
    
    // Populate save data
    SaveGameInstance->PlayerLocation = Player->GetActorLocation();
    // ... more data
    
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, "SaveSlot1", 0);
}

// Load game
void USaveGameSubsystem::LoadGame()
{
    USaveGame* LoadedGame = Cast<USaveGame>(
        UGameplayStatics::LoadGameFromSlot("SaveSlot1", 0)
    );
    
    if (LoadedGame)
    {
        // Restore game state
        Player->SetActorLocation(LoadedGame->PlayerLocation);
        // ... restore more data
    }
}
```

---

## System Integration

### How Systems Work Together

```
Player Input
    ↓
Enhanced Input System
    ↓
Character Movement Component
    ↓
Gameplay Ability System
    ↓
Character Actions (Abilities)
    ↓
Replicated via Network System
    ↓
AI Detection System (if detected)
    ↓
State Tree AI (reaction)
    ↓
Game State Updates
    ↓
Timeline System (if timeline switch)
    ↓
Save System (checkpoint)
```

---

## Performance Considerations

### Optimization Strategies

**Tick Management:**
- Disable tick when not needed
- Use variable tick rates
- Tick only when on screen

**Replication:**
- Minimize replicated properties
- Use relevancy and priorities
- Batch updates

**AI:**
- LOD for AI complexity
- Throttle expensive checks
- Use EQS sparingly

---

## Conclusion

This systems overview provides a comprehensive look at how Echoes of Time is structured. For implementation details, refer to the source code and inline documentation.

---

**Last Updated**: January 2025  
**Maintainer**: Traveler3114
