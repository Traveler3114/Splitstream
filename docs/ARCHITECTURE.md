# Splitstream - Architecture Documentation

This document provides an in-depth technical overview of the Splitstream codebase architecture, design patterns, and implementation details.

---

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Module Structure](#module-structure)
- [Core Systems](#core-systems)
- [Design Patterns](#design-patterns)
- [Code Organization](#code-organization)
- [Networking Architecture](#networking-architecture)
- [Data Flow](#data-flow)
- [Extension Points](#extension-points)

---

## Architecture Overview

Splitstream follows a component-based architecture using Unreal Engine 5.7's framework. The project is structured around several key pillars:

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Game Instance Layer                      │
│                  (DefaultGameInstance)                       │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      Game Mode Layer                         │
│     (BaseGameMode, DefaultGameMode, LobbyGameMode)          │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────────┐
│  Game State  │  │ Player State │  │    Characters    │
│              │  │              │  │  & Controllers   │
└──────────────┘  └──────────────┘  └──────────────────┘
        │                   │                   │
        └───────────────────┼───────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│               Component-Based Systems                        │
│  (Inventory, Detection, Abilities, Input, etc.)             │
└─────────────────────────────────────────────────────────────┘
```

---

## Module Structure

### Main Game Module: Splitstream

**Module Type**: Runtime  
**Location**: `Source/Splitstream/`

#### Module Dependencies

```cpp
PublicDependencyModuleNames:
- Core, CoreUObject, Engine
- InputCore, EnhancedInput
- AIModule, NavigationSystem
- StateTreeModule, GameplayStateTreeModule
- Niagara
- UMG, RHI, RenderCore
- GameplayAbilities, GameplayTags, GameplayTasks

PrivateDependencyModuleNames:
- Slate, SlateCore
- OnlineSubsystem, OnlineSubsystemUtils
- NetCore
```

#### Build Configuration

**File**: `Splitstream.Build.cs`

```cpp
PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
```

Uses explicit precompiled headers for faster compilation.

---

## Core Systems

### 1. Gameplay Ability System (GAS)

#### Architecture

```
UAbilitySystemComponent
    │
    ├── UAttributeSet (PlayerAttributeSet)
    │   ├── Health
    │   ├── Stamina
    │   ├── WalkSpeed
    │   ├── RunSpeed
    │   └── CrouchSpeed
    │
    ├── UGameplayAbility
    │   ├── DefaultGAHack
    │   ├── DefaultGALockPick
    │   └── FutureGAPastEcho
    │
    └── UGameplayEffect
        └── Attribute modifications
```

#### Implementation Details

**DefaultAbilitySystemComponent**
- Custom ASC implementation
- Handles ability granting and activation
- Manages gameplay tags and effects

**PlayerAttributeSet**
- Contains character stats
- Replicates for multiplayer
- Callbacks for attribute changes

**Ability Input System**
- Uses Enhanced Input Actions
- Data-driven through `AbilityInputSet`
- Binds input to ability activation

#### Usage Example

```cpp
// Grant ability
ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass));

// Activate ability by tag
ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

// Listen to attribute changes
ASC->GetGameplayAttributeValueChangeDelegate(
    Attribute).AddUObject(this, &AMyClass::OnAttributeChanged);
```

### 2. Timeline System

#### Concept

The game features two parallel timelines that players can switch between:
- **Past**: Historical version of the world
- **Future**: Futuristic version of the world

#### Implementation

**Enum Definition**
```cpp
UENUM(BlueprintType)
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};
```

**Timeline-Aware Actors**
- Actors can have different states in different timelines
- Level streaming loads/unloads timeline-specific content
- State is synchronized across multiplayer

### 3. Character System

#### Class Hierarchy

```
ACharacter (Unreal Base)
    │
    ├── ADefaultCharacter (Player Character)
    │   ├── IInteractable
    │   ├── IAbilitySystemInterface
    │   └── IDetectable
    │
    └── AAICharacter (AI Base)
        ├── AGuardCharacter (Past Guards)
        ├── ARobotGuardCharacter (Future Guards)
        └── ACivilianCharacter (NPCs)
```

#### ADefaultCharacter

**Core Features:**
- Enhanced Input System integration
- Gameplay Ability System
- Inventory management
- Timeline switching
- Multiplayer replication

**Key Components:**
```cpp
UCameraComponent* FirstPersonCamera;
UStaticMeshComponent* EquippedItemMesh;
UInventoryComponent* InventoryComponent;
UDetectionComponent* DetectionComponent;
UAbilitySystemComponent* AbilitySystemComponent;
```

**Input Handling:**
Uses Enhanced Input with Input Mapping Contexts
- Data-driven input configuration
- Context-aware input stacking
- Remappable controls

#### DronePawn

Special pawn for drone gameplay:
- Alternative camera perspective
- Flight mechanics
- Limited interaction capabilities
- Can switch back to character

### 4. AI System

#### State Tree Implementation

Uses Unreal's State Tree for AI behaviors:

**State Structure:**
```
Root State
├── Idle State
│   └── Transition: On Player Detected → Investigate
├── Investigate State
│   ├── Move to Last Known Position
│   └── Transition: Player Found → Alert
├── Alert State
│   ├── Chase Player
│   └── Call for Backup
└── Search State
    └── Search Area
```

**Components:**
- **DetectionComponent**: Vision cone, hearing
- **SearchComponent**: Search patterns
- State Tree asset for behavior

#### Detection System

**Line of Sight:**
- Raycasts for vision checks
- Occlusion testing
- Distance-based detection
- Sound detection radius

**Alert Levels:**
1. **Unaware**: Normal patrol
2. **Suspicious**: Investigating sound/movement
3. **Alert**: Player detected
4. **Search**: Lost sight, actively searching

### 5. Component Systems

#### InventoryComponent

**Features:**
- Slot-based inventory
- Item management
- Equipment system
- Multiplayer replication

**Data Structure:**
```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    UPROPERTY()
    UItemDataAsset* Item;
    
    UPROPERTY()
    int32 Quantity;
    
    UPROPERTY()
    bool bIsEquipped;
};
```

#### DetectionComponent

**Responsibilities:**
- Manages detection state
- Tracks detected actors
- Alert level management
- Visual debugging

**Integration:**
- Used by AI characters
- Interfaces with IDetectable actors
- Network replicated

#### HackComponent / ProximityHackComponent

**Hacking System:**
- Hackable objects implement hacking interface
- Mini-game integration
- Progress tracking
- Success/failure callbacks

**Types:**
- **HackComponent**: Direct interaction hacking
- **ProximityHackComponent**: Proximity-based hacking

#### LockPickComponent

**Lockpicking System:**
- Mini-game based
- Difficulty levels
- Tool requirements
- Failure consequences

### 6. Interface System

#### Design Philosophy

Uses C++ interfaces for polymorphism without tight coupling.

#### Core Interfaces

**IInteractable**
```cpp
class IInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(APawn* InteractingPawn);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FText GetInteractionText() const;
};
```

**IDetectable**
- Allows actors to be detected by AI
- Provides detection modifiers
- Team identification

**IRepairable**
- Future timeline objects
- Repair mini-game
- State restoration

**IKeycardUnlockable**
- Keycard-locked doors/containers
- Access level requirements

**IPuzzleCompletionReceiver**
- Callback for puzzle completion
- Rewards/progression

---

## Design Patterns

### 1. Component Pattern

**Usage:** Extensive use of actor components for reusable functionality

**Benefits:**
- Modularity and reusability
- Composition over inheritance
- Easy to add/remove features

**Example:**
```cpp
// Add inventory to any actor
UInventoryComponent* Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
```

### 2. Interface Pattern

**Usage:** C++ interfaces for polymorphic behavior

**Benefits:**
- Decoupling
- Multiple interface implementation
- Blueprint-friendly

**Example:**
```cpp
// Check if actor is interactable
if (Actor->Implements<UInteractable>())
{
    IInteractable::Execute_Interact(Actor, PlayerPawn);
}
```

### 3. Data-Driven Design

**Usage:** Data Assets for configuration

**Benefits:**
- Designer-friendly
- No recompilation needed
- Easy balancing

**Examples:**
- `InputMappingSet`: Input configuration
- `AbilityInputSet`: Ability input bindings
- `DefaultGASet`: Gameplay ability sets
- Item data assets

### 4. Observer Pattern

**Usage:** Delegates and events for loose coupling

**Examples:**
- Attribute change callbacks
- Inventory change notifications
- Detection state changes

```cpp
OnInventoryChanged.AddDynamic(this, &ADefaultCharacter::OnInventoryChanged);
```

### 5. State Pattern

**Usage:** State Tree for AI behaviors

**Benefits:**
- Visual state machine
- Designer-friendly
- Efficient execution

---

## Code Organization

### Directory Structure Philosophy

```
Source/Splitstream/
├── AbilitySystem/           # GAS implementation
│   ├── Abilities/           # Individual abilities
│   ├── AttributeSets/       # Attribute definitions
│   └── EOTGameplayTags.cpp  # Tag registration
│
├── ActorComponents/         # Reusable components
│   └── [Component].h/cpp    # One component per file pair
│
├── Actors/                  # Actor classes
│   └── [Actor].h/cpp        # One actor per file pair
│
├── Characters/              # Character implementations
│   └── [Character].h/cpp    # One character per file pair
│
├── Controllers/             # Player and AI controllers
│
├── DataAssets/              # Data asset definitions
│
├── GameModes/               # Game mode classes
│   ├── BaseGameMode         # Base class
│   ├── DefaultGameMode      # Main game mode
│   └── LobbyGameMode        # Multiplayer lobby
│
├── GameStates/              # Game state management
│
├── Interfaces/              # C++ interfaces
│   └── I[Interface].h/cpp   # Interface files
│
├── Minigames/               # Mini-game systems
│
├── Saving/                  # Save/load system
│
└── Widgets/                 # UI widget classes
```

### Naming Conventions

**Classes:**
```cpp
AMyActor          // Actor class
UMyObject         // UObject class
UMyComponent      // Component class
FMyStruct         // Struct
EMyEnum           // Enum
IMyInterface      // Interface
TMyTemplate       // Template class
```

**Variables:**
```cpp
int32 MyVariable           // Local variable
int32 MemberVariable       // Member variable (can use b prefix for bool)
bIsActive                  // Boolean member
UPROPERTY() int32 MyProp   // Property variable
```

**Functions:**
```cpp
void DoSomething()                    // Public function
void DoSomethingInternal()            // Protected/Private function
void OnSomethingHappened()            // Event callback
UFUNCTION() void BlueprintFunc()      // Blueprint-callable function
```

---

## Networking Architecture

### Replication Model

**Authority Model:**
- Server authoritative
- Client prediction for movement
- Server validation for actions

### Replicated Classes

**GameState (ABaseGameState):**
- Global game state
- Timeline information
- Match progress

**PlayerState (ADefaultPlayerState):**
- Player-specific data
- Score/stats
- Ability unlocks

**Characters:**
- Position and rotation
- Ability activation
- Equipment state
- Detection state

### RPC Usage

**Client → Server:**
```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerInteract(AActor* Target);
```

**Server → Client:**
```cpp
UFUNCTION(Client, Reliable)
void ClientNotifyDetection();
```

**Multicast:**
```cpp
UFUNCTION(NetMulticast, Reliable)
void MulticastPlayEffect();
```

### Advanced Sessions Integration

**Features:**
- Lobby system
- Server browser
- Steam integration
- Epic Online Services

**Usage:**
```cpp
// Create session
AdvancedSessions->CreateSession(...)

// Find sessions
AdvancedSessions->FindSessions(...)

// Join session
AdvancedSessions->JoinSession(...)
```

---

## Data Flow

### Input Flow

```
Player Input
    ↓
Enhanced Input System
    ↓
Input Action Triggered
    ↓
Character Input Handler
    ↓
┌─────────────┬─────────────────┐
│   Movement  │  Ability Input  │
│   Component │                 │
└─────────────┴─────────────────┘
        ↓              ↓
    Character      Ability
    Movement       Activation
```

### Ability Activation Flow

```
Input Trigger
    ↓
Input Action → Gameplay Tag
    ↓
AbilitySystemComponent::TryActivateAbility
    ↓
Check: Cost, Cooldown, Tags
    ↓
UGameplayAbility::ActivateAbility
    ↓
Execute Ability Logic
    ↓
Apply Gameplay Effects
    ↓
UGameplayAbility::EndAbility
```

### Interaction Flow

```
Player Presses Interact Key
    ↓
Raycast from Camera
    ↓
Hit Actor Implements IInteractable?
    ↓ Yes
Display Interaction Prompt
    ↓
Player Confirms Interaction
    ↓
Call IInteractable::Execute_Interact
    ↓
Server RPC (if multiplayer)
    ↓
Execute Interaction Logic
    ↓
Client Response / Feedback
```

### AI Detection Flow

```
AI Tick
    ↓
DetectionComponent::PerformDetection
    ↓
Get Potential Targets in Range
    ↓
For Each Target:
    ├─ Line of Sight Check
    ├─ Distance Check
    ├─ Angle Check
    └─ Occlusion Check
    ↓
Target Detected?
    ↓ Yes
Update Detection Level
    ↓
Trigger State Tree Transition
    ↓
Execute Alert Behavior
```

---

## Extension Points

### Adding New Abilities

1. **Create C++ Class:**
   ```cpp
   UCLASS()
   class UMyGameplayAbility : public UGameplayAbility
   {
       virtual void ActivateAbility(...) override;
       virtual void EndAbility(...) override;
   };
   ```

2. **Create Data Asset:**
   - Create Blueprint child in editor
   - Configure ability properties

3. **Add to Ability Set:**
   - Add to character's `DefaultGASet`
   - Define input binding

### Adding New Components

1. **Create Component Class:**
   ```cpp
   UCLASS()
   class UMyComponent : public UActorComponent
   {
       virtual void BeginPlay() override;
       virtual void TickComponent(...) override;
   };
   ```

2. **Add to Actor:**
   ```cpp
   MyComponent = CreateDefaultSubobject<UMyComponent>("MyComponent");
   ```

3. **Configure in Blueprint:**
   - Expose properties with `UPROPERTY()`
   - Make functions callable with `UFUNCTION()`

### Adding New Character Types

1. **Inherit from Base:**
   ```cpp
   UCLASS()
   class AMyCharacter : public ADefaultCharacter
   {
       // Override behaviors
   };
   ```

2. **Or Create from AAICharacter:**
   ```cpp
   UCLASS()
   class AMyAICharacter : public AAICharacter
   {
       // AI-specific implementation
   };
   ```

### Adding New Interfaces

1. **Create Interface:**
   ```cpp
   UINTERFACE(MinimalAPI, Blueprintable)
   class UMyInterface : public UInterface
   {
       GENERATED_BODY()
   };

   class IMyInterface
   {
       GENERATED_BODY()
   public:
       UFUNCTION(BlueprintNativeEvent)
       void MyFunction();
   };
   ```

2. **Implement in Class:**
   ```cpp
   class AMyActor : public AActor, public IMyInterface
   {
       virtual void MyFunction_Implementation() override;
   };
   ```

---

## Performance Considerations

### Optimization Strategies

**1. Tick Management**
- Disable tick when not needed
- Use less frequent tick intervals
- Tick only when on screen

**2. Replication**
- Minimize replicated properties
- Use relevancy and priorities
- Batch RPC calls

**3. AI**
- LOD for AI complexity
- Throttle expensive checks
- Use Environment Query System (EQS) sparingly

**4. Rendering**
- LOD models for meshes
- Occlusion culling
- GPU Lightmass for static lighting

---

## Security Considerations

### Multiplayer Security

**Server Authority:**
- Validate all client input
- Server-side checks for abilities
- Anti-cheat considerations

**Input Validation:**
```cpp
bool ADefaultCharacter::ServerInteract_Validate(AActor* Target)
{
    // Check distance
    // Check line of sight
    // Check cooldowns
    return true;
}
```

---

## Future Extensions

### Planned Systems

- Advanced dialogue system
- Dynamic weather/time of day
- Procedural level generation
- Skill tree progression
- Achievement system
- Analytics integration

### Modding Support

Potential modding hooks:
- Custom abilities
- Custom items
- Custom levels
- Custom game modes

---

## Conclusion

This architecture document provides a comprehensive overview of the Splitstream codebase. For specific implementation details, refer to the inline code documentation and comments.

**Related Documents:**
- [README.md](../README.md) - Setup and getting started
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines (if exists)

---

**Last Updated**: January 2025  
**Maintainer**: Traveler3114
