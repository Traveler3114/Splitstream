# Architecture Overview

This document describes the high-level architecture of the **Splitstream** Unreal Engine 5.7 project.

---

## Module Structure

Splitstream is a single-module UE5 game project. All gameplay C++ code resides in the `Source/Splitstream/` module.

### Build Dependencies

Defined in `Splitstream.Build.cs`:

| Dependency | Purpose |
|---|---|
| `Core`, `CoreUObject`, `Engine` | UE5 core framework |
| `EnhancedInput` | Enhanced Input system for keybinding |
| `AIModule`, `NavigationSystem` | AI perception, pathfinding |
| `StateTreeModule`, `GameplayStateTreeModule` | StateTree-driven AI behavior |
| `GameplayAbilities`, `GameplayTags`, `GameplayTasks` | GAS framework |
| `Niagara` | VFX |
| `UMG`, `Slate`, `SlateCore` | UI framework |
| `RHI`, `RenderCore` | Rendering utilities |
| `OnlineSubsystem`, `OnlineSubsystemUtils`, `NetCore` | Multiplayer networking |

### External Plugins

| Plugin | Purpose |
|---|---|
| `AdvancedSessions` / `AdvancedSteamSessions` | Extended session/friend management for Steam |
| `SteamSockets` | Steam networking transport |
| `OnlineSubsystemEOS` | Epic Online Services integration |
| `GameplayAbilities` | GAS plugin |
| `StateTree` / `GameplayStateTree` | AI behavior authoring |
| `GPULightmass` | GPU-accelerated lightmaps |

---

## Directory Layout

```
Source/Splitstream/
├── AbilitySystem/           # GAS: abilities, tasks, attribute sets, gameplay cues, tags
│   ├── Abilities/           # Concrete GameplayAbility classes
│   │   └── Weapons/         # Weapon-specific abilities (Pistol fire/aim)
│   ├── AbilityTasks/        # Async ability tasks for progressive interactions
│   ├── AttributeSets/       # UPlayerAttributeSet (Health, WalkSpeed, RunSpeed, CrouchSpeed)
│   ├── GameplayCues/        # GameplayCue notifiers (PastEcho activation/deactivation)
│   ├── DefaultAbilitySystemComponent.h/.cpp
│   └── SplitstreamGameplayTags.h/.cpp
│
├── ActorComponents/         # Reusable actor components
│   ├── DetectionComponent   # Progressive detection meter (guard → player)
│   ├── HackComponent        # Timed hacking interaction
│   ├── InventoryComponent   # Replicated inventory with slots, items, equipping
│   ├── LockPickComponent    # Pin-based lock picking mini-game component
│   ├── ProximityHackComponent # Proximity-triggered passive hacking
│   └── SearchComponent      # Timed search interaction (bodies, containers)
│
├── Actors/                  # World-placed actor classes
│   ├── Area/                # IllegalArea (GAS effect zones)
│   ├── Computers/           # Computer, ArchiveComputer
│   ├── DisablingDevice/     # DisablingDeviceActor, DevicesManagerActor
│   ├── KeypadScanner/       # KeypadScanner, KeypadButton
│   ├── Laser/               # LaserManager, LaserSensor
│   ├── Lever/               # LeverActor, LeverManager
│   ├── PointActors/         # NavNode, RefPointActor, CivilianSpawnPoint, etc.
│   ├── Projectiles/         # Bullet actor
│   ├── TimeObjects/         # Past/Future variants + GhostCharacterActor
│   └── Wire/                # WireActor, WireDeviceActor, WirePuzzleManager
│
├── Characters/              # All pawn/character classes
├── Controllers/             # Player controllers (Default, Lobby, MainMenu)
├── DataAssets/              # Data-driven asset definitions
├── GameModes/               # Server-authoritative game modes
├── GameStates/              # Replicated game state
├── Interfaces/              # UE interfaces (Blueprintable)
├── Minigames/               # Standalone mini-game logic
├── Saving/                  # Save game classes
├── Widgets/                 # UMG widget classes
│
├── DefaultGameInstance.h/.cpp
├── DefaultPlayerState.h/.cpp
├── NavUtilityLibrary.h/.cpp
├── TimelineEra.h/.cpp
├── Splitstream.h/.cpp
└── Splitstream.Build.cs
```

---

## Core Design Patterns

### 1. Interface-Driven Actor Interactions

All actor interactions use **UE interfaces** (`IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, etc.). Any actor can participate in these systems by implementing the interface, keeping coupling low and enabling Blueprint extensibility.

### 2. GAS as Central Authority

Player and AI attributes (health, speed), status effects (illegal, hacking, aiming), and abilities are all driven through the Gameplay Ability System. Gameplay tags serve as the primary communication layer between systems — for example, `Character.Status.Illegal` triggers detection by guards.

### 3. Component-Based Architecture

Reusable behaviors are Actor Components: `UInventoryComponent`, `UDetectionComponent`, `UHackComponent`, `USearchComponent`, `ULockPickComponent`, `UProximityHackComponent`.

### 4. Data-Driven Configuration

Items (`UItemBase`), ability sets (`UAbilityInputSet`, `UDefaultGASet`), and input mappings (`UInputMappingSet`) are Data Assets configurable in the editor.

### 5. Server-Authoritative Multiplayer

Listen-server architecture with server-only GameModes, replicated GameStates and PlayerStates, and RPCs for client-server communication.

### 6. StateTree AI Behavior

AI characters use StateTree components on their controllers. Gameplay tag events trigger behavior transitions.

---

## Game Flow

```
Main Menu → Create/Join Session → Lobby → ServerTravel → Gameplay → Alarm/Win/Restart → Lobby
```

1. **Main Menu** (`AMainMenuPlayerController`): Session creation/joining via AdvancedSessions + Steam/EOS.
2. **Lobby** (`ALobbyGameMode` + `ALobbyGameState`): Platform seating, ready system, team selection.
3. **Gameplay** (`ADefaultGameMode` + `ADefaultGameState`): Stealth heist with dual timelines.
4. **End**: Restart back to lobby via `ServerTravel`.
