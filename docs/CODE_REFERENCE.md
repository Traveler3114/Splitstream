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

