# Code Reference - Echoes of Time

**Complete API and Implementation Reference for Source/EchoesOfTime**

---

## Table of Contents

1. [Overview](#overview)
2. [Module Structure](#module-structure)
3. [Core Systems](#core-systems)
4. [Ability System](#ability-system)
5. [Actor Components](#actor-components)
6. [Actors](#actors)
7. [Characters](#characters)
8. [Controllers](#controllers)
9. [Data Assets](#data-assets)
10. [Game Modes & States](#game-modes--states)
11. [Interfaces](#interfaces)
12. [Minigames](#minigames)
13. [Widgets](#widgets)
14. [Utilities](#utilities)

---

## Overview

This document provides extensive technical documentation for all code in the `Source/EchoesOfTime` directory. The codebase consists of **274 C++ source files** organized into a modular architecture.

### Project Structure

```
Source/EchoesOfTime/
├── AbilitySystem/           # Gameplay Ability System implementation
├── ActorComponents/         # Reusable actor components
├── Actors/                  # Interactive world actors
├── Characters/              # Player and AI characters
├── Controllers/             # Player controllers
├── DataAssets/              # Data-driven configuration
├── GameModes/               # Game mode logic
├── GameStates/              # Game state management
├── Interfaces/              # C++ interfaces for polymorphism
├── Minigames/               # Mini-game implementations
├── Saving/                  # Save game system
├── Widgets/                 # UI components
└── *.{h,cpp}               # Core module files
```

### Key Technologies

- **Unreal Engine 5.7**: Latest engine features
- **Gameplay Ability System (GAS)**: For abilities, attributes, and effects
- **Enhanced Input System**: Modern input handling
- **Replication**: Full multiplayer support
- **State Tree**: AI behavior management
- **Timeline System**: Past/Future era mechanics


## Module Structure

This section documents the core module files that define the foundational architecture.

### EchoesOfTime.h / .cpp
**Main module definition**

Primary module header that defines the ECHOESOFTIME_API macro and initializes the game module.

```cpp
// Core module initialization
class ECHOESOFTIME_API FEchoesOfTimeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
```

**Purpose**: Defines module boundaries and API exports for the project.

---

### TimelineEra.h / .cpp
**Timeline era enumeration**

Defines the two time periods in the game: Past and Future.

```cpp
UENUM(BlueprintType)
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};
```

**Usage**:
- Character spawning based on timeline era
- Object visibility in different timelines
- Timeline-specific gameplay mechanics
- Level design and puzzle systems
- Ability restrictions per era

**Key Systems Using This**:
- `FutureGAPastEcho` - Timeline switching ability
- All "Future*" and "Past*" actor classes
- `GhostCharacterActor` - Shows past/future echoes
- Procedural level generation

---

### DefaultGameInstance.h / .cpp
**Game instance management**

Extends `UAdvancedFriendsGameInstance` to manage session lifecycle, input mappings, and user settings.

```cpp
class ECHOESOFTIME_API UDefaultGameInstance : public UAdvancedFriendsGameInstance
{
public:
    // Session management
    UFUNCTION(BlueprintImplementableEvent)
    void CreateSession(const FString& LevelName, const TSoftObjectPtr<UWorld>& Level);
    
    UFUNCTION()
    void HostLeaveToMainMenu(const FString& MainMenuMapPath = TEXT("/Game/Maps/MainMenuMap"));
    
    // Input system
    UPROPERTY()
    UInputMappingContext* RuntimeInputMappingContext;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TArray<UInputAction*> AllInputActions;
    
    // Settings
    UPROPERTY()
    float MouseSensitivity;
    
    void LoadUserSettings();
    void SaveUserSettings(const TArray<FSavedKeybind>& Keybinds, float NewMouseSensitivity);
    UInputAction* FindInputActionByName(const FName& ActionName) const;
    
protected:
    void RequestDestroySessionAndCleanup(bool bTravelAfterDestroy, const FString& InPendingMenuURL);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void CleanupNetDriver();
    void CleanupOSSDelegates();
};
```

**Key Responsibilities**:
- Multiplayer session creation and destruction
- Input mapping context management
- User settings persistence (keybinds, mouse sensitivity)
- Network driver cleanup
- Session state management

**Methods**:
- `Init()`: Loads user settings on startup
- `LoadUserSettings()`: Reads saved keybindings and mouse sensitivity from save game
- `SaveUserSettings()`: Persists user preferences to disk
- `FindInputActionByName()`: Retrieves input actions by name for remapping
- `RequestDestroySessionAndCleanup()`: Handles multiplayer session cleanup
- `CleanupNetDriver()`: Ensures proper network resource cleanup

---

### DefaultPlayerState.h / .cpp
**Player state replication**

Manages player-specific data that needs to be replicated across the network.

```cpp
class ECHOESOFTIME_API ADefaultPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    // Replicated player data
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Purpose**: Stores persistent player data that survives respawns and pawn changes.

---

### NavUtilityLibrary.h / .cpp
**Navigation utilities**

Blueprint function library for navigation and pathfinding operations.

```cpp
class ECHOESOFTIME_API UNavUtilityLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    // Navigation helper functions for AI and level design
};
```

**Usage**: AI pathfinding, spawn point validation, procedural level generation.

---

## Ability System

The project uses Unreal's **Gameplay Ability System (GAS)** for abilities, attributes, and gameplay effects.

### DefaultAbilitySystemComponent.h / .cpp

**Custom ability system component**

```cpp
class ECHOESOFTIME_API UDefaultAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
    // Custom extensions to UE's ability system
};
```

**Purpose**: Extends Epic's `UAbilitySystemComponent` with project-specific functionality. Currently a base implementation that can be extended with custom ability granting logic, input binding, or attribute initialization.

---

### EOTGameplayTags.h / .cpp

**Gameplay tag definitions**

Defines all gameplay tags used throughout the project for abilities, statuses, and events. These tags are declared with `UE_DECLARE_GAMEPLAY_TAG_EXTERN` and defined in the `.cpp` file.

**Character Tags**:
```cpp
TAG_Character                           // Base character tag
TAG_Character_Status_Block              // General blocking status
TAG_Character_Status_Block_Movement     // Blocks movement
TAG_Character_Status_Block_Look         // Blocks camera/look input
```

**Ability Tags**:
```cpp
TAG_Character_Ability_Future_PastEcho   // Past Echo ability (Future timeline)
TAG_Character_Ability_LockPick          // Lock picking ability
TAG_Character_Ability_Hack              // Hacking ability
TAG_Character_Ability_Search            // Searching ability
TAG_Character_Ability_Solo_Teleport     // Teleport ability (Solo mode)
```

**Status Tags**:
```cpp
TAG_Character_Status_PastEcho           // Currently viewing past
TAG_Character_Status_LockPicking        // Currently lock picking
TAG_Character_Status_Hacking            // Currently hacking
TAG_Character_Status_ProximityHacking   // Proximity hack active
TAG_Character_Status_Searching          // Currently searching
TAG_Character_Status_Aiming             // Aiming weapon
TAG_Character_Status_Firing             // Firing weapon
TAG_Character_Status_Illegal            // In illegal state
TAG_Character_Status_Illegal_Action     // Performing illegal action
TAG_Character_Status_Illegal_Area       // In restricted area
TAG_Character_Status_Teleporting        // Teleporting
```

**Weapon Tags**:
```cpp
TAG_Weapon_Ability_Pistol_Fire          // Fire pistol
TAG_Weapon_Ability_Pistol_Aim           // Aim pistol
```

**GameplayCue Tags**:
```cpp
TAG_GameplayCue_PastEcho_Activated      // Visual/audio for past echo activation
TAG_GameplayCue_PastEcho_Deactivated    // Visual/audio for past echo deactivation
```

**AI/StateTree Tags**:
```cpp
TAG_StateTree_Event_FullyDetected_Pawn  // AI detected a pawn
TAG_StateTree_Event_FullyDetected_Actor // AI detected an actor
TAG_StateTree_Event_RepairNeeded        // Device needs repair
TAG_Guard_Status_Repair                 // Guard is repairing
```

**Usage**: These tags are used throughout the system for:
- Ability activation/blocking conditions
- Character state management
- UI updates and indicators
- AI behavior triggers
- Gameplay effect application/removal
- Event-driven architecture

---


### Gameplay Abilities

Located in `AbilitySystem/Abilities/` - These implement the core player abilities using Unreal's Gameplay Ability System.

#### DefaultGAHack.h / .cpp

**Hacking ability for terminals and electronic devices**

Allows players to hack computers, terminals, security systems, and electronic devices through a mini-game interface.

```cpp
class ECHOESOFTIME_API UDefaultGAHack : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData) override;
    
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
                           const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo,
                           bool bReplicateEndAbility,
                           bool bWasCancelled) override;
};
```

**Features**:
- Triggers hacking mini-game (Firewall or similar)
- Blocks movement and look input during hack
- Network replicated for multiplayer
- Integrates with `HackComponent` on target actors
- Applies `TAG_Character_Status_Hacking` tag
- Grants access to secure systems on success

**Activation**:
1. Player interacts with hackable object (computer, terminal, security panel)
2. Ability checks if target has `HackComponent`
3. Applies blocking tags (`TAG_Character_Status_Block_Movement`)
4. Launches hacking mini-game widget
5. On completion, grants/denies access based on mini-game result

**Related Classes**:
- `HackComponent` - Component on hackable objects
- `HackAbilityTask` - Async task for hack progress
- `HackWidget` - UI for hacking mini-game
- `FirewallMiniGame` - Specific mini-game implementation

---

#### DefaultGALockPick.h / .cpp

**Lock picking ability for doors and containers**

Interactive lock picking mechanic with mini-game for bypassing locks on doors, lockers, and containers.

```cpp
class ECHOESOFTIME_API UDefaultGALockPick : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    virtual void EndAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "LockPick")
    float BaseLockPickDuration = 3.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "LockPick")
    float DifficultyMultiplier = 1.5f;
};
```

**Features**:
- Mini-game based lock picking system
- Difficulty scales with lock complexity
- Can fail and require retry
- Alerts nearby guards if detected during picking
- Unlocks doors, lockers, and containers
- Applies `TAG_Character_Status_LockPicking` tag
- Progressive interaction (hold to continue)

**Lock Pick Mechanics**:
- **Easy Locks**: 2-3 second pick time
- **Medium Locks**: 4-5 second pick time
- **Hard Locks**: 6-8 second pick time
- Can be interrupted by:
  - Player movement
  - Detection by guards
  - Player cancellation

**Related Classes**:
- `LockPickComponent` - Component on lockable objects
- `LockPickAbilityTask` - Async task for lock pick progress
- `LockPickWidget` - UI showing pick progress
- `DoorBase` - Doors with locks
- `LockerActor` - Lockable storage

---

#### DefaultGASearch.h / .cpp

**Search ability for finding items in containers and on characters**

Allows players to search AI characters, containers, desks, and other searchable objects to find items and collectibles.

```cpp
class ECHOESOFTIME_API UDefaultGASearch : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    virtual void EndAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Search")
    float BaseSearchDuration = 2.0f;
    
    void OnSearchComplete();
    void GrantSearchRewards(AActor* SearchedActor);
};
```

**Features**:
- Progressive interaction (hold to search)
- Finds items in containers and on characters
- Can be interrupted
- Time-based completion
- Applies `TAG_Character_Status_Searching` tag
- Grants items directly to inventory
- Illegal when searching characters

**Search Types**:
1. **Character Search**: Search unconscious/dead AI characters
2. **Container Search**: Desks, cabinets, boxes
3. **Locker Search**: After lock picking
4. **Actor Search**: Generic searchable actors

**Related Classes**:
- `SearchComponent` - Component on searchable objects
- `SearchAbilityTask` - Async task for search progress
- `SearchWidget` - UI showing search progress
- `SearchableActor` - Generic searchable object
- `AICharacter` - Searchable AI characters
- `DeskActor` - Searchable desk

---

#### FutureGAPastEcho.h / .cpp

**Past Echo ability (Future timeline exclusive)**

The signature ability for Future timeline characters. Allows players to see "echoes" of past events, revealing hidden information and ghost characters from the past timeline.

```cpp
class ECHOESOFTIME_API UFutureGAPastEcho : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    virtual void EndAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "PastEcho")
    float EchoDuration = 10.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "PastEcho")
    float Cooldown = 15.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "PastEcho")
    float StaminaCost = 30.0f;
    
    void ActivatePastVision();
    void DeactivatePastVision();
    void RevealGhostCharacters();
    void HideGhostCharacters();
};
```

**Features**:
- Reveals past timeline events and objects
- Shows ghost characters (echoes of past timeline)
- Time-limited duration (10 seconds default)
- Visual effects via Gameplay Cues
- Post-process effects for visual distinction
- Cooldown period after use
- Stamina/resource cost
- Applies `TAG_Character_Status_PastEcho` tag

**Visual Effects**:
- Screen tint/color grading
- Ghostly particle effects
- Past objects become visible
- Temporal distortion effects

**Gameplay Applications**:
- **Puzzle Solving**: See past object positions
- **Navigation**: Reveal hidden paths
- **Story**: Witness past events
- **Information**: Read past documents/terminals

**Related Classes**:
- `GhostCharacterActor` - Ghost representations of past characters
- `GCN_PastEchoActivated` - Visual/audio cue on activation
- `GCN_PastEchoDeactivated` - Visual/audio cue on deactivation
- `IGhostRevealable` - Interface for revealable objects
- Past/Future actor pairs (PastDoor/FutureDoor, etc.)

---

#### SoloGATeleport.h / .cpp

**Teleport ability (Solo mode)**

Single-player teleportation ability for quick traversal. Not available in multiplayer modes.

```cpp
class ECHOESOFTIME_API USoloGATeleport : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Teleport")
    float MaxTeleportDistance = 2000.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Teleport")
    float TeleportCooldown = 5.0f;
    
    bool IsLocationValid(const FVector& Location) const;
    void PerformTeleport(const FVector& TargetLocation);
    void SpawnTeleportEffects(const FVector& SourceLocation, const FVector& TargetLocation);
};
```

**Features**:
- Instant teleportation to line-of-sight location
- Line-of-sight validation
- Navigation mesh validation
- Cooldown system
- VFX at source and destination
- Applies `TAG_Character_Status_Teleporting` tag
- Can be blocked by walls/obstacles

**Teleport Validation**:
- Must have line of sight to target
- Target must be on navigation mesh
- Cannot teleport through walls
- Must be within max distance
- Target location must have valid ground

**Related Classes**:
- `Teleporter` - Teleporter pad actor (different system)
- Navigation validation utilities

---

### Weapon Abilities

Located in `AbilitySystem/Abilities/Weapons/` - Combat abilities for weapons.

#### PistolGAAim.h / .cpp

**Pistol aim ability**

Handles pistol aiming mechanics with camera adjustments and movement modifications.

```cpp
class ECHOESOFTIME_API UPistolGAAim : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    virtual void EndAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Aim")
    FVector CameraAimOffset = FVector(100, 50, -20);
    
    UPROPERTY(EditDefaultsOnly, Category = "Aim")
    float AimFOV = 60.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Aim")
    float AimMovementSpeedMultiplier = 0.5f;
    
    void EnterAimMode();
    void ExitAimMode();
    void AdjustCamera();
    void RestoreCamera();
};
```

**Features**:
- Camera zoom and shoulder offset
- FOV reduction for precision aiming
- Movement speed reduction (50% by default)
- Replicated aiming state across network
- Applies `TAG_Character_Status_Aiming` tag
- Smooth camera transitions (Blueprint)

**Aiming Effects**:
- Camera moves over shoulder
- FOV narrows for better targeting
- Walk speed reduced
- Crosshair changes (UI)
- Higher accuracy when firing

**Related Classes**:
- `DefaultCharacter` - Character with camera component
- `PistolGAFire` - Fire ability
- Weapon mesh and animations

---

#### PistolGAFire.h / .cpp

**Pistol fire ability**

Handles pistol firing mechanics including projectile spawning, ammo consumption, and hit detection.

```cpp
class ECHOESOFTIME_API UPistolGAFire : public UGameplayAbility
{
    GENERATED_BODY()
public:
    virtual void ActivateAbility(...) override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    TSubclassOf<class ABullet> ProjectileClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    float FireRate = 0.25f; // Seconds between shots
    
    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    float Damage = 20.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    float BaseAccuracy = 0.95f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Fire")
    int32 MagazineSize = 12;
    
    void FireWeapon();
    void SpawnProjectile();
    void ApplyRecoil();
    void ConsumeAmmo();
    bool CanFire() const;
};
```

**Features**:
- Hitscan or projectile-based firing
- Ammo consumption and tracking
- Recoil application
- Muzzle flash effects
- Impact effects and sound
- Accuracy system (affected by aiming)
- Fire rate limiting
- Applies `TAG_Character_Status_Firing` tag

**Firing Mechanics**:
- **Aimed Fire**: Higher accuracy, lower fire rate
- **Hip Fire**: Lower accuracy, faster fire rate
- **Recoil**: Camera kick per shot
- **Spread**: Bullet dispersion based on accuracy

**Ammo System**:
- Magazine-based (12 rounds default)
- Reload required when empty
- Infinite magazines (or limited based on game mode)

**Related Classes**:
- `Bullet` - Projectile actor
- `PistolGAAim` - Aim ability
- `DefaultCharacter` - Shooter character
- Damage system and health attributes

---


### Ability Tasks

Located in `AbilitySystem/AbilityTasks/` - Asynchronous operations that run during ability execution.

Ability tasks are used to handle time-based or event-driven operations within gameplay abilities without blocking the main thread.

#### HackAbilityTask.h / .cpp

**Asynchronous hacking task**

Manages the progression of hacking operations, including timing, success/failure conditions, and event broadcasting.

```cpp
class ECHOESOFTIME_API UHackAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", 
              meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static UHackAbilityTask* CreateHackTask(
        UGameplayAbility* OwningAbility,
        AActor* TargetActor,
        float Duration);
    
    // Delegates
    UPROPERTY(BlueprintAssignable)
    FHackTaskDelegate OnComplete;
    
    UPROPERTY(BlueprintAssignable)
    FHackTaskDelegate OnFailed;
    
    UPROPERTY(BlueprintAssignable)
    FHackTaskDelegate OnCancelled;
    
protected:
    virtual void Activate() override;
    virtual void OnDestroy(bool AbilityEnded) override;
    
    void TickHackProgress();
    
    FTimerHandle ProgressTimerHandle;
    float HackDuration;
    float CurrentProgress;
};
```

**Purpose**: Manages asynchronous hacking operations with progress tracking and cancellation support.

**Features**:
- Time-based progress tracking
- Cancellable operation
- Multiple outcome delegates (success, failure, cancel)
- Network-aware (can be replicated)

---

#### LockPickAbilityTask.h / .cpp

**Asynchronous lock picking task**

Handles the duration and outcome of lock picking attempts.

```cpp
class ECHOESOFTIME_API ULockPickAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
              meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static ULockPickAbilityTask* CreateLockPickTask(
        UGameplayAbility* OwningAbility,
        AActor* TargetActor,
        float Duration,
        float Difficulty);
    
    UPROPERTY(BlueprintAssignable)
    FLockPickTaskDelegate OnSuccess;
    
    UPROPERTY(BlueprintAssignable)
    FLockPickTaskDelegate OnFailure;
    
    UPROPERTY(BlueprintAssignable)
    FLockPickTaskDelegate OnInterrupted;
    
protected:
    virtual void Activate() override;
    void CheckForInterruption();
    void CompleteLockPick(bool bSuccess);
};
```

**Purpose**: Manages lock picking duration, difficulty checks, and interruption detection.

**Features**:
- Difficulty-based success probability
- Interruption detection (movement, detection)
- Success/failure outcomes
- Progress visualization support

---

#### SearchAbilityTask.h / .cpp

**Asynchronous search task**

Manages the progressive search interaction with objects and characters.

```cpp
class ECHOESOFTIME_API USearchAbilityTask : public UAbilityTask
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
              meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
    static USearchAbilityTask* CreateSearchTask(
        UGameplayAbility* OwningAbility,
        AActor* TargetActor,
        float Duration);
    
    UPROPERTY(BlueprintAssignable)
    FSearchTaskDelegate OnSearchComplete;
    
    UPROPERTY(BlueprintAssignable)
    FSearchTaskDelegate OnSearchInterrupted;
    
protected:
    virtual void Activate() override;
    void TickSearchProgress();
    void GrantSearchResults();
};
```

**Purpose**: Handles progressive search interaction with cancellation support.

---

### Attribute Sets

Located in `AbilitySystem/AttributeSets/` - Defines gameplay attributes managed by GAS.

#### PlayerAttributeSet.h / .cpp

**Player attributes for Gameplay Ability System**

Defines all player stats and attributes managed by the Gameplay Ability System, including health, stamina, and movement speeds.

```cpp
class ECHOESOFTIME_API UPlayerAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    // Health Attributes
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Health)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxHealth)
    
    // Stamina Attributes
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Stamina)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxStamina)
    
    // Movement Speed Attributes
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_WalkSpeed)
    FGameplayAttributeData WalkSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, WalkSpeed)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_RunSpeed)
    FGameplayAttributeData RunSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, RunSpeed)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CrouchSpeed)
    FGameplayAttributeData CrouchSpeed;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CrouchSpeed)
    
    // Replication Notifies
    UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
    
    UFUNCTION()
    virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
    
    UFUNCTION()
    virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
    
    UFUNCTION()
    virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
    
    UFUNCTION()
    virtual void OnRep_WalkSpeed(const FGameplayAttributeData& OldWalkSpeed);
    
    UFUNCTION()
    virtual void OnRep_RunSpeed(const FGameplayAttributeData& OldRunSpeed);
    
    UFUNCTION()
    virtual void OnRep_CrouchSpeed(const FGameplayAttributeData& OldCrouchSpeed);
    
    // Attribute clamping and handling
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Attributes Explained**:

1. **Health**: Current health points (0 = dead)
2. **MaxHealth**: Maximum health capacity
3. **Stamina**: Current stamina/energy (used for sprinting, abilities)
4. **MaxStamina**: Maximum stamina capacity
5. **WalkSpeed**: Base walking speed (cm/s)
6. **RunSpeed**: Sprinting speed (cm/s)
7. **CrouchSpeed**: Crouched movement speed (cm/s)

**Usage**:
- Modified by Gameplay Effects
- Monitored for death/damage events
- UI displays current values
- Character movement speed updates automatically
- Network replicated for multiplayer

**Attribute Modification**:
- **Instant**: Direct value changes (damage, healing)
- **Duration**: Temporary buffs/debuffs
- **Infinite**: Permanent stat changes

---

### Gameplay Cues

Located in `AbilitySystem/GameplayCues/` - Handle visual and audio feedback for gameplay events.

Gameplay Cues provide cosmetic feedback for gameplay events without affecting game logic. They're network-optimized and can be executed, added, or removed.

#### GCN_PastEchoActivated.h / .cpp

**Past Echo activation visual/audio cue**

Triggers visual and audio effects when the Past Echo ability is activated.

```cpp
class ECHOESOFTIME_API UGCN_PastEchoActivated : public UGameplayCueNotify_Static
{
    GENERATED_BODY()
public:
    virtual bool OnExecute_Implementation(
        AActor* MyTarget,
        const FGameplayCueParameters& Parameters) const override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* ActivationSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    UParticleSystem* ActivationParticles;
    
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UCameraShakeBase> CameraShake;
    
    UPROPERTY(EditDefaultsOnly, Category = "PostProcess")
    FPostProcessSettings PastEchoPostProcess;
};
```

**Effects**:
- Screen tint/color grading (desaturated, bluish)
- Ghostly particle effects around player
- Temporal distortion sound effects
- Camera shake on activation
- Post-process volume adjustments

**Visual Style**:
- Blue/cyan color tint
- Slightly desaturated
- Blur/distortion edges
- Particle wisps

---

#### GCN_PastEchoDeactivated.h / .cpp

**Past Echo deactivation visual/audio cue**

Removes or fades out Past Echo effects when the ability ends.

```cpp
class ECHOESOFTIME_API UGCN_PastEchoDeactivated : public UGameplayCueNotify_Static
{
    GENERATED_BODY()
public:
    virtual bool OnExecute_Implementation(
        AActor* MyTarget,
        const FGameplayCueParameters& Parameters) const override;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* DeactivationSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    float FadeOutDuration = 0.5f;
};
```

**Effects**:
- Smooth fade out of post-process effects
- Deactivation sound
- Particle system fade out
- Return to normal vision

---

## Actor Components

Located in `ActorComponents/` - Reusable components that add functionality to actors.

These components implement specific gameplay systems and can be attached to any actor to provide that functionality.

### DetectionComponent.h / .cpp

**Player detection and stealth system**

Manages AI detection of the player character, handling detection levels, decay, and alert states.

```cpp
class ECHOESOFTIME_API UDetectionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UDetectionComponent();
    
    // Detection State
    UPROPERTY(BlueprintReadOnly, Category = "Detection")
    float CurrentDetectionLevel = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionThreshold = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRate = 10.0f; // Points per second when in sight
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDecayRate = 5.0f; // Points per second when not in sight
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SuspicionDecayDelay = 2.0f; // Seconds before decay starts
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void IncreaseDetection(float Amount, AActor* Detector);
    
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void DecreaseDetection(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void ResetDetection();
    
    UFUNCTION(BlueprintCallable, Category = "Detection")
    void ForceFullDetection(AActor* Detector);
    
    UFUNCTION(BlueprintPure, Category = "Detection")
    float GetDetectionPercentage() const;
    
    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsDetected() const;
    
    UFUNCTION(BlueprintPure, Category = "Detection")
    bool IsInSuspicion() const;
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Detection")
    FOnDetectionChanged OnDetectionChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Detection")
    FOnFullyDetected OnFullyDetected;
    
    UPROPERTY(BlueprintAssignable, Category = "Detection")
    FOnDetectionLost OnDetectionLost;
    
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;
    
private:
    TSet<TWeakObjectPtr<AActor>> CurrentDetectors;
    float TimeSinceLastIncrease = 0.0f;
    bool bWasDetected = false;
};
```

**Features**:
- **Progressive Detection**: Builds up over time when in sight
- **Decay System**: Decreases when not in sight
- **Multiple Detectors**: Tracks multiple AI/cameras detecting player
- **Detection States**: Clear → Suspicious → Detected → Alerted
- **Event Broadcasting**: Notifies when detection state changes
- **Visual Feedback**: UI indicator shows detection level

**Detection States**:
1. **Clear** (0-25%): Not detected, safe
2. **Suspicious** (25-75%): Guard is investigating
3. **Detected** (75-99%): Guard knows player position
4. **Alerted** (100%): Full alarm, guard chasing

**Usage**: Attached to player character, monitored by guards, cameras, and other detection sources.

---


### HackComponent.h / .cpp

**Makes objects hackable**

Component that makes an actor hackable by players, managing hack state, difficulty, and completion callbacks.

```cpp
class ECHOESOFTIME_API UHackComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UHackComponent();
    
    // Hack Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hack")
    float HackDifficulty = 1.0f; // 0.5 = Easy, 1.0 = Normal, 2.0 = Hard
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hack")
    float HackDuration = 5.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Hack", ReplicatedUsing = OnRep_IsHacked)
    bool bIsHacked = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hack")
    bool bCanBeHackedMultipleTimes = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hack")
    TSubclassOf<class UFirewallMiniGame> MiniGameClass;
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void StartHack(AActor* Hacker);
    
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void CompleteHack();
    
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void FailHack();
    
    UFUNCTION(BlueprintCallable, Category = "Hack")
    void ResetHack();
    
    UFUNCTION(BlueprintPure, Category = "Hack")
    bool CanBeHacked() const;
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Hack")
    FOnHackComplete OnHackComplete;
    
    UPROPERTY(BlueprintAssignable, Category = "Hack")
    FOnHackFailed OnHackFailed;
    
    UPROPERTY(BlueprintAssignable, Category = "Hack")
    FOnHackStarted OnHackStarted;
    
protected:
    UFUNCTION()
    void OnRep_IsHacked();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Usage**: Attach to computers, terminals, doors, security systems, cameras, drones, etc.

**Features**:
- Configurable difficulty levels
- Mini-game integration
- One-time or repeatable hacks
- Network replicated
- Event broadcasting

---

### InventoryComponent.h / .cpp

**Player inventory management**

Manages player inventory with fixed slots, active item tracking, and ability/effect granting from items.

```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UItemBase* ItemAsset = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FGuid ItemInstanceID;
    
    // Runtime-only (not saved/replicated)
    TArray<FActiveGameplayEffectHandle> GrantedGameplayEffectHandles;
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FInventorySlot>&, Items);

class ECHOESOFTIME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UInventoryComponent();
    
    // Inventory Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 SlotCount = 10;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<UItemBase*> DefaultItemAssets;
    
    UPROPERTY(ReplicatedUsing = OnRep_Slots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;
    
    UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 ActiveSlotIndex = 0;
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItemBase* ItemAsset, FGuid ItemInstanceID);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItem(int32 Index);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveItemByInstanceID(FGuid ItemInstanceID);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetActiveSlot(int32 Index);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropActiveItem(FVector DropLocation);
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventorySlot GetActiveItem() const;
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventorySlot> GetSlots() const { return Slots; }
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FGameplayTag GetTeamTag() const;
    
    // Server RPCs
    UFUNCTION(Server, Reliable)
    void ServerSetActiveSlot(int32 Index);
    
    UFUNCTION(Server, Reliable)
    void ServerDropActiveItem(FVector DropLocation);
    
    UFUNCTION(Server, Reliable)
    void ServerAddItem(UItemBase* ItemAsset);
    
    // Rep Notifies
    UFUNCTION()
    void OnRep_Slots();
    
    UFUNCTION()
    void OnRep_ActiveSlotIndex();
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;
    
    // Timeline system integration
    void RegisterFutureInstance(FGuid ItemInstanceID);
    
protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
private:
    void HandleFutureItemInvalidated(FGuid InvalidID);
    TSet<FGuid> RegisteredFutureInstances;
};
```

**Features**:
- **Fixed Slot Count**: Configurable number of inventory slots
- **Active Slot System**: One item equipped at a time
- **Item Abilities**: Items grant gameplay abilities when equipped
- **Item Effects**: Items apply passive gameplay effects
- **Network Replication**: Full multiplayer support
- **Timeline Integration**: Future items invalidate when past changes
- **Default Items**: Can spawn with initial items
- **Item Dropping**: Drop items into the world

**Usage**: Attached to player character.

**Slot Management**:
- Items added to first empty slot
- Number keys (1-0) select slots
- Scroll wheel cycles through slots
- Drop key removes active item

---

### LockPickComponent.h / .cpp

**Makes objects lockable and pickable**

Component for objects that can be locked and unlocked through lock picking.

```cpp
class ECHOESOFTIME_API ULockPickComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    ULockPickComponent();
    
    // Lock Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick", ReplicatedUsing = OnRep_IsLocked)
    bool bIsLocked = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    float PickDifficulty = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    float PickDuration = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    bool bRequiresLockpickItem = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockPick")
    int32 LockComplexity = 3; // Number of pins/tumblers
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void StartLockPick(AActor* Picker);
    
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void UnlockSuccess();
    
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void UnlockFailed();
    
    UFUNCTION(BlueprintCallable, Category = "LockPick")
    void Lock();
    
    UFUNCTION(BlueprintPure, Category = "LockPick")
    bool IsLocked() const { return bIsLocked; }
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockUnlocked OnLockUnlocked;
    
    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockPickStarted OnLockPickStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "LockPick")
    FOnLockPickFailed OnLockPickFailed;
    
protected:
    UFUNCTION()
    void OnRep_IsLocked();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
```

**Usage**: Attach to doors, lockers, containers, safes, etc.

**Features**:
- Difficulty-based lock picking
- Optional lockpick item requirement
- Can be re-locked
- Network replicated
- Lock complexity (visual mini-game)

---

### ProximityHackComponent.h / .cpp

**Proximity-based hacking**

Allows hacking devices by being near them without direct interaction (e.g., hacking cameras or drones).

```cpp
class ECHOESOFTIME_API UProximityHackComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UProximityHackComponent();
    
    // Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProximityHack")
    float HackRange = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProximityHack")
    float HackDuration = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProximityHack")
    bool bRequiresLineOfSight = true;
    
    UPROPERTY(BlueprintReadOnly, Category = "ProximityHack", ReplicatedUsing = OnRep_IsHacked)
    bool bIsHacked = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "ProximityHack")
    bool bIsHacking = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "ProximityHack")
    float HackProgress = 0.0f;
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "ProximityHack")
    void StartProximityHack(AActor* Hacker);
    
    UFUNCTION(BlueprintCallable, Category = "ProximityHack")
    void StopProximityHack();
    
    UFUNCTION(BlueprintCallable, Category = "ProximityHack")
    void CompleteHack();
    
    UFUNCTION(BlueprintPure, Category = "ProximityHack")
    bool IsInRange(AActor* Hacker) const;
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "ProximityHack")
    FOnProximityHackComplete OnProximityHackComplete;
    
    UPROPERTY(BlueprintAssignable, Category = "ProximityHack")
    FOnProximityHackInterrupted OnProximityHackInterrupted;
    
protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                              FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION()
    void OnRep_IsHacked();
    
private:
    TWeakObjectPtr<AActor> CurrentHacker;
    void TickHackProgress(float DeltaTime);
    bool ValidateHackConditions();
};
```

**Usage**: Attach to security cameras, drones, automated turrets, security systems.

**Features**:
- Range-based activation
- Progressive hack (fills over time)
- Line of sight requirement (optional)
- Can be interrupted if player moves away
- UI indicator shows hack progress

---

### SearchComponent.h / .cpp

**Makes actors searchable**

Component that makes actors searchable for loot, items, and collectibles.

```cpp
class ECHOESOFTIME_API USearchComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    USearchComponent();
    
    // Search Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    TArray<UItemBase*> SearchableItems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    float SearchDuration = 2.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Search", ReplicatedUsing = OnRep_IsSearched)
    bool bIsSearched = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    bool bCanBeSearchedMultipleTimes = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    bool bRequiresProgressiveInteraction = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search")
    bool bIsIllegalAction = false; // Searching people is illegal
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "Search")
    void StartSearch(AActor* Searcher);
    
    UFUNCTION(BlueprintCallable, Category = "Search")
    void CompleteSearch();
    
    UFUNCTION(BlueprintCallable, Category = "Search")
    void InterruptSearch();
    
    UFUNCTION(BlueprintCallable, Category = "Search")
    void ResetSearch();
    
    UFUNCTION(BlueprintPure, Category = "Search")
    bool CanBeSearched() const;
    
    UFUNCTION(BlueprintPure, Category = "Search")
    TArray<UItemBase*> GetSearchableItems() const { return SearchableItems; }
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Search")
    FOnSearchComplete OnSearchComplete;
    
    UPROPERTY(BlueprintAssignable, Category = "Search")
    FOnSearchStarted OnSearchStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "Search")
    FOnSearchInterrupted OnSearchInterrupted;
    
protected:
    UFUNCTION()
    void OnRep_IsSearched();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
private:
    void GrantItemsToSearcher(AActor* Searcher);
};
```

**Usage**: Attach to AI characters, desks, cabinets, containers, searchable actors.

**Features**:
- Configurable loot tables
- Progressive or instant search
- One-time or repeatable
- Illegal action flag (for NPCs)
- Network replicated

**Search Targets**:
- Dead/unconscious AI characters
- Desks and office furniture
- Containers and boxes
- Lockers (after unlocking)
- Generic searchable props

---

