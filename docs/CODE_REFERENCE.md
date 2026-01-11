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


## Actors

Located in `Actors/` - Interactive world objects and environmental elements. This is the largest module with 50+ actor types organized into several categories.

### Base Actor Classes

These provide common functionality for derived actor types.

#### DoorBase.h / .cpp

**Abstract base class for all door types**

Provides core door functionality including open/close, locking, keycard access, and auto-open for AI.

```cpp
UCLASS(Abstract)
class ECHOESOFTIME_API ADoorBase : public AActor, 
                                    public IInteractable, 
                                    public IKeycardUnlockable
{
    GENERATED_BODY()
public:
    // Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USceneComponent* SceneRoot;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DoorMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UArrowComponent* ArrowComp; // Shows door forward direction
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    class UBoxComponent* GuardOpenTrigger; // Auto-open trigger for AI
    
    // Door State
    UPROPERTY(ReplicatedUsing = OnRep_IsOpen, EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bIsOpen = false;
    
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Lock")
    bool bIsLocked = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bRequiresKeycard = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bAutoOpenForGuards = true;
    
    // Lock Pick Component
    UPROPERTY()
    class ULockPickComponent* LockPickComponent = nullptr;
    
    // Methods
    int32 ComputeOpenDirection(AActor* ReferenceActor) const;
    
    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override;
    
    // IKeycardUnlockable
    virtual void UnlockWithKeycard_Implementation(AActor* Interactor) override;
    virtual bool RequiresKeycard_Implementation() const override;
    
    // Blueprint Events (animation in BP)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void OpenDoor(int32 Direction);
    
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Door")
    void CloseDoor(int32 Direction);
    
    // Lock System
    UFUNCTION()
    virtual void OnLockUnlocked();
    
    UFUNCTION()
    virtual void OnRep_IsOpen();
    
    // Guard Auto-Open System
    UFUNCTION()
    void OnGuardOpenBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, ...);
    
    UFUNCTION()
    void OnGuardOpenEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, ...);
    
    UFUNCTION()
    void ForceOpenDoorForGuard(AActor* GuardActor);
    
    UFUNCTION()
    void ForceCloseDoorForGuard(AActor* GuardActor);
    
protected:
    int32 OpenDirection = 1; // 1 = away from player, -1 = toward player
};
```

**Features**:
- **Smart Opening**: Computes door swing direction based on player position
- **Lock System**: Supports lock picking via LockPickComponent
- **Keycard Access**: Can require keycard for access
- **AI Auto-Open**: Guards can walk through without interaction
- **Network Replication**: Full multiplayer support
- **Blueprint Animation**: Door movement handled in Blueprint

**Opening Directions**:
- Door opens away from player (push) if player is on hinge side
- Door opens toward player (pull) if player is opposite hinge
- Direction: 1 = forward, -1 = backward

**Derived Classes**:
- `PastDoor` - Door in past timeline
- `FutureDoor` - Door in future timeline

---

#### DoubleDoorBase.h / .cpp

**Double door with two panels**

Similar to DoorBase but manages two door panels that open symmetrically.

```cpp
UCLASS(Abstract)
class ECHOESOFTIME_API ADoubleDoorBase : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* LeftDoorMesh;
    
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* RightDoorMesh;
    
    UPROPERTY(ReplicatedUsing = OnRep_IsOpen)
    bool bIsOpen = false;
    
    UPROPERTY(Replicated)
    bool bIsLocked = false;
    
    // Similar interface to DoorBase
    UFUNCTION(BlueprintImplementableEvent)
    void OpenDoubleDoor(int32 Direction);
    
    UFUNCTION(BlueprintImplementableEvent)
    void CloseDoubleDoor(int32 Direction);
};
```

**Derived Classes**:
- `PastDoubleDoor`
- `FutureDoubleDoor`

---

#### VentBase.h / .cpp

**Ventilation shaft for stealth traversal**

Crawl spaces that allow players to bypass areas or reach hidden locations.

```cpp
class ECHOESOFTIME_API AVentBase : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* VentMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* ExitPoint; // Target vent or location
    
    UPROPERTY(EditAnywhere)
    bool bRequiresTool = false; // Need crowbar/screwdriver
    
    UPROPERTY(EditAnywhere)
    bool bIsOpen = false;
    
    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void TeleportPlayerToExit(ACharacter* Character);
    void PlayCrawlAnimation();
};
```

**Features**:
- Stealth traversal path
- Can require tools to open
- Teleports to exit vent
- Hidden from guard sight

**Derived Classes**:
- `PastVent`
- `FutureVent`

---

### Interactive Objects

#### AlarmButton.h / .cpp

**Panic button for triggering alarms**

Guards can press this to trigger base-wide alarm.

```cpp
class ECHOESOFTIME_API AAlarmButton : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(ReplicatedUsing = OnRep_IsActivated)
    bool bIsActivated = false;
    
    UPROPERTY(EditAnywhere)
    bool bSingleUse = true;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION()
    void TriggerAlarm();
    
    UFUNCTION()
    void OnRep_IsActivated();
    
    UPROPERTY(BlueprintAssignable)
    FOnAlarmTriggered OnAlarmTriggered;
};
```

**Usage**: Placed near guard posts, security stations

---

#### CodeGenerator.h / .cpp

**Generates random access codes**

Creates random codes displayed on documents/screens that players need for keypads.

```cpp
class ECHOESOFTIME_API ACodeGenerator : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Code")
    int32 CodeLength = 4;
    
    UPROPERTY(EditAnywhere, Category = "Code")
    bool bUseLetters = false;
    
    UPROPERTY(EditAnywhere, Category = "Code")
    bool bUseNumbers = true;
    
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Code")
    FString GeneratedCode;
    
    UFUNCTION(BlueprintCallable, Category = "Code")
    FString GenerateNewCode();
    
    UFUNCTION(BlueprintPure, Category = "Code")
    FString GetCode() const { return GeneratedCode; }
    
protected:
    virtual void BeginPlay() override;
};
```

**Usage**: 
- Generates code on BeginPlay
- Code displayed on in-game screens/documents
- Players use code to unlock keypads

---

#### DeskActor.h / .cpp

**Interactive office desk**

```cpp
class ECHOESOFTIME_API ADeskActor : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* DeskMesh;
    
    UPROPERTY(EditAnywhere)
    TArray<UItemBase*> ContainedItems;
    
    UPROPERTY(EditAnywhere)
    bool bRequiresSearch = true;
    
    UPROPERTY()
    USearchComponent* SearchComponent;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
};
```

**Features**:
- Contains searchable items
- Can have documents on surface
- Drawers can be locked

---

#### ItemPickup.h / .cpp

**Collectible item actor**

World pickup that adds items to player inventory.

```cpp
class ECHOESOFTIME_API AItemPickup : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    UItemBase* ItemData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FGuid ItemInstanceID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    UStaticMeshComponent* MeshComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bAutoGenerateGUID = true;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UPROPERTY(BlueprintAssignable)
    FOnItemPickedUp OnItemPickedUp;
    
protected:
    virtual void BeginPlay() override;
    void PickupItem(AActor* Interactor);
};
```

**Derived Classes**:
- `PastItemPickup` - Item in past timeline
- `FutureItemPickup` - Item in future timeline (invalidated if past changes)

---

#### KeycardScanner.h / .cpp

**Keycard access control system**

```cpp
class ECHOESOFTIME_API AKeycardScanner : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Keycard")
    FGameplayTag RequiredKeycardTag;
    
    UPROPERTY(EditAnywhere, Category = "Keycard")
    TArray<AActor*> ControlledActors; // Doors, elevators, etc.
    
    UPROPERTY(EditAnywhere, Category = "Keycard")
    bool bRequiresSpecificKeycard = true;
    
    UPROPERTY(EditAnywhere, Category = "Keycard")
    bool bOneTimeUse = false;
    
    UPROPERTY(ReplicatedUsing = OnRep_IsActivated)
    bool bIsActivated = false;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
protected:
    bool CheckKeycardAccess(AActor* Interactor);
    void UnlockControlledActors();
    void GrantAccessToInteractor(AActor* Interactor);
    
    UFUNCTION()
    void OnRep_IsActivated();
};
```

**Features**:
- Requires specific keycard item
- Unlocks multiple connected actors
- Visual feedback (light changes)
- Network replicated

---

#### LockerActor.h / .cpp

**Storage locker**

```cpp
class ECHOESOFTIME_API ALockerActor : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* LockerMesh;
    
    UPROPERTY(EditAnywhere)
    TArray<UItemBase*> StoredItems;
    
    UPROPERTY(EditAnywhere)
    bool bIsLocked = true;
    
    UPROPERTY()
    ULockPickComponent* LockPickComponent;
    
    UPROPERTY()
    USearchComponent* SearchComponent;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void OpenLocker();
};
```

**Usage**: Contains loot, must be unlocked first

---

#### MetalDetector.h / .cpp

**Security checkpoint metal detector**

```cpp
class ECHOESOFTIME_API AMetalDetector : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UBoxComponent* DetectionVolume;
    
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer MetalTags; // Tags that trigger detection
    
    UPROPERTY(EditAnywhere)
    bool bIsActive = true;
    
    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
                                 AActor* OtherActor, ...);
    
    UPROPERTY(BlueprintAssignable)
    FOnMetalDetected OnMetalDetected;
    
private:
    bool ActorTriggersDetector(AActor* Actor);
};
```

**Features**:
- Detects weapons and metal items
- Triggers alarm if metal detected
- Can be disabled via hacking

---

#### NewspaperActor.h / .cpp

**Readable newspaper with lore**

```cpp
class ECHOESOFTIME_API ANewspaperActor : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Content")
    FText NewspaperHeadline;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    FText NewspaperContent;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    UTexture2D* NewspaperImage;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    FDateTime PublicationDate;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void DisplayNewspaper(APlayerController* PlayerController);
};
```

**Usage**: Environmental storytelling, hints, lore

---


#### PowerGenerator.h / .cpp

**Power generator for puzzle systems**

Powers devices and doors, can be activated/deactivated.

```cpp
class ECHOESOFTIME_API APowerGenerator : public AActor, public IInteractable, public IRepairable
{
    GENERATED_BODY()
public:
    UPROPERTY(ReplicatedUsing = OnRep_IsPowered, EditAnywhere, BlueprintReadWrite)
    bool bIsPowered = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
    TArray<AActor*> PoweredActors;
    
    UPROPERTY(EditAnywhere, Category = "Power")
    bool bRequiresRepair = false;
    
    UPROPERTY(EditAnywhere, Category = "Power")
    bool bStartsOn = true;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION(BlueprintCallable, Category = "Power")
    void TogglePower();
    
    UFUNCTION(BlueprintCallable, Category = "Power")
    void SetPowered(bool bNewPowered);
    
    // IRepairable
    virtual void Repair_Implementation(AActor* Repairer) override;
    virtual bool NeedsRepair_Implementation() const override;
    
    UFUNCTION()
    void OnRep_IsPowered();
    
private:
    void UpdatePoweredActors();
    void NotifyPoweredActors(bool bPowerState);
};
```

**Derived Classes**:
- `PastPowerGenerator`
- `FuturePowerGenerator`

**Features**:
- Powers doors, lights, terminals
- Can break and need repair
- Network replicated
- Visual indicators (lights, sounds)

---

### Security Systems

#### SecurityCamera.h / .cpp

**Surveillance camera with detection**

Automatically panning camera that detects players and triggers alarms.

```cpp
class ECHOESOFTIME_API ASecurityCamera : public AActor, public IDetectable
{
    GENERATED_BODY()
public:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    UStaticMeshComponent* CameraMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    class USceneCaptureComponent2D* SceneCapture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UArrowComponent* ArrowComp;
    
    // Detection Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionDistance = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ViewConeAngle = 90.0f;
    
    // Pan Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float MinYaw = -45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float MaxYaw = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float PanSpeed = 30.0f; // Degrees per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Pan")
    float PauseAtLimit = 2.0f; // Pause at each end
    
    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugDetectionCone = false;
    
    // State
    UPROPERTY(EditAnywhere, Category = "Camera")
    bool bCanBeHacked = true;
    
    UPROPERTY(ReplicatedUsing = OnRep_IsDisabled)
    bool bIsDisabled = false;
    
    // IDetectable
    virtual void OnFullyDetected_Implementation(AActor* DetectingActor) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;
    
    UFUNCTION()
    void OnRep_IsDisabled();
    
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    void DetectionUpdate();
    void PanUpdate();
    
    FTimerHandle DetectionTimerHandle;
    FTimerHandle PanTimerHandle;
    
    float DetectionInterval = 0.2f;
    float PanInterval = 0.02f;
    
    float CurrentYaw = 0.0f;
    bool bPanningRight = true;
    float PauseTimer = 0.0f;
    
    UPROPERTY(ReplicatedUsing = OnRep_PanOffset)
    float PanOffset = 0.0f;
    
    UFUNCTION()
    void OnRep_PanOffset();
    
    TSet<TWeakObjectPtr<AActor>> LastDetectedActors;
};
```

**Features**:
- Automatic panning within range
- Cone-based line-of-sight detection
- Can be hacked/disabled
- Triggers alarms on detection
- Optimized with timer-based ticking
- Network replicated pan position

---

#### SecurityDocumentActor.h / .cpp

**Readable security document**

Displays security information like access codes, patrol schedules.

```cpp
class ECHOESOFTIME_API ASecurityDocumentActor : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Content")
    FText DocumentTitle;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    FText DocumentContent;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    TSubclassOf<UUserWidget> DocumentWidgetClass;
    
    UPROPERTY(EditAnywhere, Category = "Content")
    UTexture2D* DocumentTexture;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void ShowDocument(APlayerController* PlayerController);
};
```

**Usage**: Contains hints, codes, lore, patrol routes

---

### Computer Systems

Located in `Actors/Computers/`

#### Computer.h / .cpp

**Base computer terminal**

```cpp
class ECHOESOFTIME_API AComputer : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* ComputerMesh;
    
    UPROPERTY(EditAnywhere)
    UHackComponent* HackComponent;
    
    UPROPERTY(EditAnywhere, Category = "Computer")
    bool bRequiresPassword = false;
    
    UPROPERTY(EditAnywhere, Category = "Computer")
    FString Password;
    
    UPROPERTY(ReplicatedUsing = OnRep_IsAccessed)
    bool bIsAccessed = false;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnComputerAccessed();
    
    UFUNCTION()
    void OnRep_IsAccessed();
};
```

---

#### ArchiveComputer.h / .cpp

**Archive/database computer**

```cpp
class ECHOESOFTIME_API AArchiveComputer : public AComputer
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Archive")
    TArray<FText> ArchiveEntries;
    
    UPROPERTY(EditAnywhere, Category = "Archive")
    TMap<FString, FText> DatabaseEntries;
    
    UFUNCTION(BlueprintCallable, Category = "Archive")
    void DisplayArchiveEntry(int32 Index);
    
    UFUNCTION(BlueprintCallable, Category = "Archive")
    FText SearchDatabase(const FString& Query);
};
```

---

### Timeline Objects

Located in `Actors/TimeObjects/` - Objects that exist in specific timelines

#### GhostCharacterActor.h / .cpp

**Ghost echo of past characters**

Visible only during Past Echo ability, shows past events.

```cpp
class ECHOESOFTIME_API AGhostCharacterActor : public AActor, public IGhostRevealable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    USkeletalMeshComponent* GhostMesh;
    
    UPROPERTY(EditAnywhere, Category = "Ghost")
    ETimelineEra SourceTimeline = ETimelineEra::Past;
    
    UPROPERTY(EditAnywhere, Category = "Ghost")
    bool bPlaysAnimation = true;
    
    UPROPERTY(EditAnywhere, Category = "Ghost")
    UAnimSequence* GhostAnimation;
    
    UPROPERTY(EditAnywhere, Category = "Ghost")
    float AnimationPlayRate = 1.0f;
    
    // IGhostRevealable
    virtual void RevealGhost_Implementation() override;
    virtual void HideGhost_Implementation() override;
    
protected:
    virtual void BeginPlay() override;
    
private:
    void SetupGhostMaterial();
    void PlayLoopingAnimation();
};
```

**Features**:
- Invisible by default
- Revealed during Past Echo ability
- Plays looping animations
- Ghostly material/shader
- No collision

---

### Specialized Actors

#### Teleporter.h / .cpp

**Teleportation pad**

```cpp
class ECHOESOFTIME_API ATeleporter : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Teleporter")
    AActor* DestinationTeleporter;
    
    UPROPERTY(EditAnywhere, Category = "Teleporter")
    FVector DestinationOffset = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, Category = "Teleporter")
    bool bBidirectional = true;
    
    UPROPERTY(EditAnywhere, Category = "Teleporter")
    float TeleportDelay = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Teleporter")
    UParticleSystem* TeleportEffect;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void TeleportActor(AActor* ActorToTeleport);
    void SpawnTeleportEffects(const FVector& Location);
    
    UFUNCTION(Server, Reliable)
    void ServerTeleportActor(AActor* ActorToTeleport);
};
```

---

#### Terminal.h / .cpp

**Interactive terminal**

```cpp
class ECHOESOFTIME_API ATerminal : public AActor, public IInteractable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* TerminalMesh;
    
    UPROPERTY(EditAnywhere)
    UHackComponent* HackComponent;
    
    UPROPERTY(EditAnywhere, Category = "Terminal")
    TArray<AActor*> ControlledDevices;
    
    UPROPERTY(EditAnywhere, Category = "Terminal")
    FText TerminalContent;
    
    virtual void Interact_Implementation(AActor* Interactor) override;
    
private:
    void ActivateControlledDevices();
};
```

---

## Characters

Located in `Characters/` - Player and AI character classes

### DefaultCharacter.h / .cpp

**Main player character**

The primary player character with full GAS integration, inventory, detection, and movement.

```cpp
class ECHOESOFTIME_API ADefaultCharacter : public ACharacter, 
                                           public IInteractable, 
                                           public IAbilitySystemInterface, 
                                           public IDetectable
{
    GENERATED_BODY()
public:
    ADefaultCharacter();
    
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ASC")
    UAbilitySystemComponent* AbilitySystemComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    UDetectionComponent* DetectionComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UStaticMeshComponent* EquippedItemMeshComp;
    
    // Ability System
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* FutureGASet;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UAbilityInputSet* SoloGASet;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    UDefaultGASet* DefaultGASet;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    TSubclassOf<UGameplayEffect> AttributeInitGE;
    
    // Input
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingSet* InputMappingSet;
    
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
    
    // State
    UPROPERTY(ReplicatedUsing = OnRep_SprintState)
    bool bIsSprinting = false;
    
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Pitch)
    float Pitch = 0.0f;
    
    // Methods
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnRep_PlayerState() override;
    virtual void PossessedBy(AController* NewController) override;
    
    // Movement
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    virtual void Jump() override;
    void StartCrouch();
    void StopCrouching();
    void StartSprint();
    void StopSprint();
    
    // Inventory
    void UpdateEquippedItemMesh();
    void DropActiveItem();
    void SelectInventorySlot(int32 SlotNumber);
    void HandleNumberKey(FKey PressedKey);
    void OnInventoryChanged(const TArray<FInventorySlot>& Slots);
    
    // Ability System
    void InitializeAbilitySystem();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    void HandleAbilityInput(const FInputActionInstance& Instance, FGameplayTag InputTag);
    void HandleAbilityInputReleased(const FInputActionInstance& Instance, FGameplayTag InputTag);
    const UPlayerAttributeSet* GetPlayerAttributeSet() const;
    
    // Attribute callbacks
    void OnWalkSpeedChanged(const FOnAttributeChangeData& ChangeData);
    void OnRunSpeedChanged(const FOnAttributeChangeData& ChangeData);
    void OnCrouchSpeedChanged(const FOnAttributeChangeData& ChangeData);
    
    // Interaction
    void HandleInteractHoldStart();
    void HandleInteractHoldStop();
    void HandleInteractInstant();
    void UpdateInteractHighlight();
    
    UFUNCTION(Server, Reliable)
    void ServerHandleInteract(AActor* TargetActor);
    
    // Camera
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FVector CameraDefaultLocation;
    
    UPROPERTY(BlueprintReadOnly, Category = "Camera")
    FRotator CameraDefaultRotation;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StartAimCamera(FVector AimLocation, FRotator AimRotation);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Aim")
    void StopAimCamera(FVector ReturnLocation, FRotator ReturnRotation);
    
    // IDetectable
    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* Detector) override;
    
    // Utility
    bool GetForwardTraceResult(float TraceDistance, FHitResult& OutHit, FVector& OutTraceEnd) const;
    
    UFUNCTION(BlueprintPure, Category = "Detection")
    static float CalculateDetectionAngle(const FVector& CameraLocation,
                                        const FRotator& PlayerCameraRotation,
                                        const FVector& SelfLocation);
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
protected:
    AActor* HighlightedActor = nullptr;
    AActor* ProgressiveActor = nullptr;
    
private:
    void GrantAbilitiesFromInputSet();
    void GrantAbilitiesFromDefaultSet();
    void OnIllegalTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
```

**Features**:
- Full Gameplay Ability System integration
- Inventory management
- Detection system
- Enhanced Input System
- Network replication
- Timeline-based abilities
- Sprint, crouch, jump mechanics
- Item interaction and pickup
- Camera system with aim support

---


### AICharacter.h / .cpp

**Base class for AI characters**

Abstract base for AI-controlled NPCs including guards and civilians.

```cpp
UCLASS(Abstract)
class ECHOESOFTIME_API AAICharacter : public ACharacter, 
                                       public IAbilitySystemInterface, 
                                       public IInteractable, 
                                       public IDetectable
{
    GENERATED_BODY()
public:
    AAICharacter();
    
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UAbilitySystemComponent* AbilitySystemComponent;
    
    UPROPERTY()
    UPlayerAttributeSet* AttributeSet;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    UDetectionComponent* DetectionComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
    USearchComponent* SearchComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UAISenseConfig_Sight* SightConfig;
    
    // State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor = nullptr;
    
    UPROPERTY()
    AActor* DetectedActor = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    ETimelineEra TimelineEra = ETimelineEra::Past;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsDead = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    int32 MoneyToSubtract = -10000; // Penalty for killing
    
    // Item System (for searchable loot)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UItemBase* ItemData;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    FGuid ItemInstanceID;
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Item")
    FOnAICharacterPickedUp OnItemPickedUp;
    
    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    // IInteractable
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void CancelInteract_Implementation(AActor* Interactor) override;
    virtual void SetHighlighted_Implementation(bool bHighlight) override;
    virtual bool IsProgressiveInteract_Implementation() override { return true; }
    
    // IDetectable
    virtual void OnDetected_Implementation(AActor* Detector) override;
    virtual void OnLost_Implementation(AActor* Detector) override;
    virtual void OnForceDetectionEnd_Implementation(AActor* Detector) override;
    virtual void OnFullyDetected_Implementation(AActor* ActorDetected) override;
    virtual bool IsActorAlreadyDetected_Implementation(AActor* DetectingActor) const override;
    
protected:
    virtual void BeginPlay() override;
    virtual void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    virtual void OnSearchComplete();
    virtual void TryPickup(AActor* Interactor);
    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};
```

**Features**:
- AI Perception System integration
- Searchable for loot
- Can be killed/knocked out
- Health system via GAS
- Timeline-specific spawning

**Derived Classes**:
- `GuardCharacter` - Security guards
- `RobotGuardCharacter` - Robot guards
- `CivilianCharacter` - Civilians
- `DronePawn` - Flying drones

---

### GuardCharacter.h / .cpp

**Security guard AI**

Guards that patrol, detect players, and respond to alarms.

```cpp
class ECHOESOFTIME_API AGuardCharacter : public AAICharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "AI|Patrol")
    TArray<AActor*> PatrolPoints;
    
    UPROPERTY(EditAnywhere, Category = "AI|Detection")
    float DetectionRange = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "AI|Detection")
    float HearingRange = 1000.0f;
    
    UPROPERTY(EditAnywhere, Category = "AI|Combat")
    bool bIsArmed = true;
    
    UPROPERTY(EditAnywhere, Category = "AI|Behavior")
    bool bCanCallBackup = true;
    
    // State Tree behaviors handle:
    // - Patrolling
    // - Investigating
    // - Chasing
    // - Combat
    // - Searching
    // - Calling for backup
};
```

---

### CivilianCharacter.h / .cpp

**Civilian NPC**

Non-hostile NPCs that can panic and call guards.

```cpp
class ECHOESOFTIME_API ACivilianCharacter : public AAICharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "AI|Behavior")
    bool bCanPanic = true;
    
    UPROPERTY(EditAnywhere, Category = "AI|Behavior")
    bool bWillCallGuards = true;
    
    UPROPERTY(EditAnywhere, Category = "AI|Behavior")
    float PanicRadius = 500.0f;
    
    // Behaviors:
    // - Walking/standing
    // - Fleeing when scared
    // - Calling for help
};
```

---

### DronePawn.h / .cpp

**Flying surveillance drone**

```cpp
class ECHOESOFTIME_API ADronePawn : public APawn, public IDetectable
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    UStaticMeshComponent* DroneMesh;
    
    UPROPERTY(EditAnywhere)
    class UFloatingPawnMovement* MovementComponent;
    
    UPROPERTY(EditAnywhere, Category = "AI")
    TArray<AActor*> PatrolPoints;
    
    UPROPERTY(EditAnywhere, Category = "Detection")
    float DetectionRange = 1500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Drone")
    bool bCanBeHacked = true;
    
    // Behaviors:
    // - Patrolling
    // - Detecting players
    // - Can be disabled by hacking
};
```

---

### RobotGuardCharacter.h / .cpp

**Robotic security guard**

Future-era robot guards with different behaviors.

```cpp
class ECHOESOFTIME_API ARobotGuardCharacter : public AGuardCharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Robot")
    bool bCanBeHacked = true;
    
    UPROPERTY(EditAnywhere, Category = "Robot")
    bool bRequiresRepair = false;
    
    // Additional robotic features
};
```

---

## Controllers

Located in `Controllers/` - Player controllers for different game modes

### DefaultPlayerController.h / .cpp

**Main game player controller**

```cpp
class ECHOESOFTIME_API ADefaultPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADefaultPlayerController();
    
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    
    // HUD Management
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowPauseMenu();
    
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HidePauseMenu();
    
    // Input Mode Management
    void SetInputModeGameOnly();
    void SetInputModeUIOnly();
    void SetInputModeGameAndUI();
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> PauseMenuClass;
    
    UPROPERTY()
    UUserWidget* PauseMenuWidget;
};
```

---

### LobbyPlayerController.h / .cpp

**Multiplayer lobby controller**

```cpp
class ECHOESOFTIME_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    // Lobby-specific functionality
    UFUNCTION(BlueprintCallable)
    void ReadyUp();
    
    UFUNCTION(BlueprintCallable)
    void SelectCharacter(ETimelineEra Era);
};
```

---

### MainMenuPlayerController.h / .cpp

**Main menu controller**

```cpp
class ECHOESOFTIME_API AMainMenuPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    // Main menu functionality
    virtual void BeginPlay() override;
};
```

---

## Data Assets

Located in `DataAssets/` - Data-driven configuration

### ItemBase.h / .cpp

**Base item data asset**

Defines items that can be picked up and stored in inventory.

```cpp
UCLASS(BlueprintType)
class ECHOESOFTIME_API UItemBase : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UTexture2D* ItemIcon;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UStaticMesh* ItemMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;
    
    // Gameplay Effects granted when equipped
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;
    
    // Abilities granted when equipped
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    bool bIsEquippable = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    bool bIsStackable = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxStackSize = 1;
};
```

**Item Types**:
- Keycards (unlock doors)
- Tools (lockpicks, hacking devices)
- Weapons (pistols)
- Documents (readable information)
- Collectibles (lore items)

---

### AbilityInputSet.h / .cpp

**Maps abilities to input actions**

```cpp
UCLASS(BlueprintType)
class ECHOESOFTIME_API UAbilityInputSet : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FAbilityInputMapping> AbilityInputMappings;
};

USTRUCT(BlueprintType)
struct FAbilityInputMapping
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> Ability;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag InputTag;
};
```

---

### DefaultGASet.h / .cpp

**Default ability set**

Defines default abilities available to all characters.

```cpp
UCLASS(BlueprintType)
class ECHOESOFTIME_API UDefaultGASet : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TSubclassOf<UGameplayAbility>> Abilities;
};
```

---

### InputMappingSet.h / .cpp

**Input mapping configuration**

```cpp
UCLASS(BlueprintType)
class ECHOESOFTIME_API UInputMappingSet : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UInputMappingContext* InputMappingContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FInputActionMapping> ActionMappings;
};
```

---

## Game Modes & States

### BaseGameMode.h / .cpp

**Base game mode**

```cpp
class ECHOESOFTIME_API ABaseGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABaseGameMode();
    
    virtual void BeginPlay() override;
};
```

---

### DefaultGameMode.h / .cpp

**Main gameplay mode**

```cpp
class ECHOESOFTIME_API ADefaultGameMode : public ABaseGameMode
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    
    UFUNCTION(BlueprintCallable)
    void RestartLevel();
    
protected:
    UFUNCTION()
    void OnAlarmStarted(float AlarmEndTime);
    
    UFUNCTION()
    void OnAlarmCanceled();
    
    UFUNCTION()
    void OnPreAlarmStarted(float PreAlarmEndTime, AActor* PreAlarmInstigator);
    
    UFUNCTION()
    void OnPreAlarmCanceled();
    
private:
    FTimerHandle RestartTimerHandle;
    FTimerHandle PreAlarmTimerHandle;
};
```

---

### LobbyGameMode.h / .cpp

**Multiplayer lobby mode**

```cpp
class ECHOESOFTIME_API ALobbyGameMode : public ABaseGameMode
{
    GENERATED_BODY()
public:
    // Lobby management
    UFUNCTION(BlueprintCallable)
    void StartGame();
    
    UFUNCTION(BlueprintCallable)
    bool AllPlayersReady() const;
};
```

---

### DefaultGameState.h / .cpp

**Main game state**

```cpp
class ECHOESOFTIME_API ADefaultGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bIsAlarmActive = false;
    
    UPROPERTY(Replicated, BlueprintReadOnly)
    float AlarmTimeRemaining = 0.0f;
    
    UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
    void TriggerAlarm(float Duration);
    
    UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
    void CancelAlarm();
};
```

---

### LobbyGameState.h / .cpp

**Lobby state**

```cpp
class ECHOESOFTIME_API ALobbyGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    TArray<APlayerState*> ReadyPlayers;
    
    UPROPERTY(Replicated, BlueprintReadOnly)
    FString SelectedMap;
};
```

---

## Interfaces

Located in `Interfaces/` - C++ interfaces for polymorphic behavior

### IInteractable.h / .cpp

**Interface for interactive objects**

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void CancelInteract(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void SetHighlighted(bool bHighlight);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsProgressiveInteract();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool IsCorrectItem(UItemBase* Item) const;
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool RequiresItem() const;
};
```

**Implemented By**: Doors, computers, items, NPCs, terminals, etc.

---

### IDetectable.h / .cpp

**Interface for detectable entities**

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UDetectable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IDetectable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detectable")
    void OnDetected(AActor* Detector);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detectable")
    void OnLost(AActor* Detector);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detectable")
    void OnForceDetectionEnd(AActor* Detector);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detectable")
    void OnFullyDetected(AActor* Detector);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Detectable")
    bool IsActorAlreadyDetected(AActor* DetectingActor) const;
};
```

**Implemented By**: Player character, AI characters, cameras

---

### IKeycardUnlockable.h / .cpp

**Interface for keycard-locked objects**

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UKeycardUnlockable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IKeycardUnlockable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keycard")
    void UnlockWithKeycard(AActor* Interactor);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Keycard")
    bool RequiresKeycard() const;
};
```

---

### IGhostRevealable.h / .cpp

**Interface for ghost/past echo objects**

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UGhostRevealable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IGhostRevealable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    void RevealGhost();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ghost")
    void HideGhost();
};
```

---

### IRepairable.h / .cpp

**Interface for repairable objects**

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class URepairable : public UInterface
{
    GENERATED_BODY()
};

class ECHOESOFTIME_API IRepairable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    void Repair(AActor* Repairer);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
    bool NeedsRepair() const;
};
```

---

## Minigames

Located in `Minigames/` - Interactive mini-game systems

### FirewallMiniGame.h / .cpp

**Space Invaders-style hacking mini-game**

A retro-style space shooter mini-game used for hacking terminals.

```cpp
UCLASS(Blueprintable, BlueprintType)
class ECHOESOFTIME_API UFirewallMiniGame : public UObject
{
    GENERATED_BODY()
public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame")
    UInputMappingContext* FirewallIMC;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame")
    TSubclassOf<UFirewallWidget> WidgetClass;
    
    // Textures
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame")
    UTexture2D* PlayerTexture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame")
    UTexture2D* EnemyTexture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame")
    UTexture2D* BossTexture;
    
    // Gameplay Tuning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    int32 BossTotalHP = 100;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Boss Tuning")
    float BossModeDuration = 20.0f;
    
    // Easter Egg
    UPROPERTY(BlueprintReadWrite, Category = "EasterEgg")
    bool bUseEasterEggSprites = false;
    
    // Delegates
    UPROPERTY(BlueprintAssignable)
    FFirewallMiniGameEnded OnMiniGameEnded;
    
    // Methods
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void StartGame(APlayerController* PlayerController);
    
    UFUNCTION(BlueprintCallable, Category = "MiniGame")
    void EndGame();
    
private:
    // Game state
    FMiniGamePlayer Player;
    TArray<FMiniGameEnemy> Enemies;
    TArray<FMiniGameProjectile> Projectiles;
    bool bIsGameOver;
    
    void TickGame();
    void SpawnEnemy();
    void UpdatePlayer(float DeltaTime);
    void UpdateEnemies(float DeltaTime);
    void CheckCollisions();
    void GameOver();
    void Victory();
};
```

**Features**:
- Space Invaders-style gameplay
- Difficulty increases over time
- Boss fight after 20 seconds
- Easter egg sprite mode
- Victory = successful hack

---

### NeonRunnerMiniGame.h / .cpp

**Endless runner mini-game**

```cpp
UCLASS(Blueprintable, BlueprintType)
class ECHOESOFTIME_API UNeonRunnerMiniGame : public UObject
{
    GENERATED_BODY()
public:
    // Similar structure to FirewallMiniGame
    // Endless runner gameplay
    // Used for different hacking challenges
};
```

---

## Widgets

Located in `Widgets/` - UI components (30+ widget classes)

### HUD Widgets

- `CharacterHUD` - Main HUD container
- `CharacterOverlay` - Health, stamina, detection meter
- `HackWidget` - Hacking progress UI
- `LockPickWidget` - Lock picking mini-game UI
- `SearchWidget` - Search progress indicator
- `DetectionWidget` - Detection meter
- `ProximityHackWidget` - Proximity hack indicator

### Menu Widgets

- `MainMenuWidget` - Main menu UI
- `PauseMenuWidget` - Pause menu
- `SettingsWidget` - Settings menu
- `GraphicsWidget` - Graphics settings
- `InputWidget` - Input remapping
- `KeybindWidget` - Individual keybind UI

### Lobby Widgets

- `LobbyUI` - Multiplayer lobby
- `PlayerLobbyInfo` - Player info card
- `FriendList` - Friends list
- `FriendWidget` - Individual friend entry

### Mini-game Widgets

- `FirewallWidget` - Firewall mini-game UI
- `NeonRunnerWidget` - Neon Runner UI

---

## Utilities

### Saving System

Located in `Saving/`

#### UserSettingsSaveGame.h / .cpp

**Saves user preferences**

```cpp
USTRUCT(BlueprintType)
struct FSavedKeybind
{
    GENERATED_BODY()
    
    UPROPERTY()
    FName ActionName;
    
    UPROPERTY()
    FKey Key;
};

UCLASS()
class ECHOESOFTIME_API UUserSettingsSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FSavedKeybind> SavedKeybinds;
    
    UPROPERTY()
    float MouseSensitivity = 1.0f;
    
    UPROPERTY()
    int32 GraphicsQuality = 3;
    
    UPROPERTY()
    bool bVSyncEnabled = true;
    
    UPROPERTY()
    float MasterVolume = 1.0f;
};
```

---

## Summary

This comprehensive code reference documents **274 C++ source files** across **14 major modules**:

1. **Core Module** (5 files) - Module initialization, timeline system, game instance
2. **Ability System** (26 files) - GAS implementation with abilities, tasks, attributes, cues
3. **Actor Components** (7 files) - Reusable gameplay components
4. **Actors** (50+ files) - Interactive world objects
5. **Characters** (6 files) - Player and AI characters
6. **Controllers** (3 files) - Player controllers
7. **Data Assets** (4 files) - Data-driven configuration
8. **Game Modes & States** (6 files) - Game flow management
9. **Interfaces** (8 files) - Polymorphic behavior contracts
10. **Minigames** (2 files) - Interactive mini-games
11. **Widgets** (30+ files) - UI components
12. **Utilities** (2 files) - Helper systems

### Key Design Patterns

- **Component-Based Architecture**: Reusable components for common functionality
- **Interface-Driven Design**: Polymorphism via C++ interfaces
- **Data-Driven Configuration**: Data Assets for flexible content
- **Network Replication**: Full multiplayer support
- **Gameplay Ability System**: Abilities, attributes, and effects
- **Enhanced Input System**: Modern input handling
- **Timeline Mechanics**: Past/Future era system

### Best Practices

1. **Modularity**: Systems are loosely coupled
2. **Reusability**: Components can be mixed and matched
3. **Extensibility**: Base classes for custom implementations
4. **Network Safety**: Proper replication and RPC usage
5. **Blueprint Integration**: C++ exposes functionality to Blueprints
6. **Performance**: Timer-based updates, optimized tick

---

**Document Version**: 1.0  
**Last Updated**: January 2025  
**Total Lines**: ~3700  
**Coverage**: Complete C++ source codebase

For system-level documentation, see [SYSTEMS.md](SYSTEMS.md).  
For architecture overview, see [ARCHITECTURE.md](ARCHITECTURE.md).  
For getting started, see [QUICK_START.md](QUICK_START.md).

---

