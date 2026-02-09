# Actors & Components Reference

This document provides a detailed reference for all actor classes and actor components in **Splitstream**.

---

## Actor Components

### `UInventoryComponent`
**File:** `ActorComponents/InventoryComponent.h`
**Parent:** `UActorComponent`

Replicated inventory with fixed slot count (default 10). Manages item addition, removal, equipping, dropping, and passive gameplay effect/ability granting.

| Property | Type | Description |
|---|---|---|
| `Slots` | `TArray<FInventorySlot>` | Replicated inventory slots |
| `ActiveSlotIndex` | `int32` | Currently equipped slot |
| `SlotCount` | `int32` | Maximum slots (default 10) |
| `DefaultItemAssets` | `TArray<UItemBase*>` | Items added on BeginPlay |

**RPCs:** `ServerSetActiveSlot`, `ServerDropActiveItem`, `ServerAddItem`

---

### `UDetectionComponent`
**File:** `ActorComponents/DetectionComponent.h`
**Parent:** `UActorComponent`

Progressive detection meter. Tracks detection progress per-detector using `TMap<AActor*, FDetectionState>`.

| Property | Type | Description |
|---|---|---|
| `DetectionDuration` | `float` | Time to fully detect (default 2.5s) |
| `BaseDetectionSpeedMultiplier` | `float` | Distance-based speed modifier (default 1200) |

**Delegates:** `OnDetectionBegan`, `OnDetectionEnded`

---

### `UHackComponent`
**File:** `ActorComponents/HackComponent.h`
**Parent:** `UActorComponent`

Timed hacking interaction. Used by computers and other hackable objects.

| Property | Type | Description |
|---|---|---|
| `HackDuration` | `float` | Time to complete hack (default 10s) |
| `bHackingInProgress` | `bool` | Currently hacking (replicated) |
| `bHacked` | `bool` | Successfully hacked (replicated) |

**Delegate:** `OnHackComplete`

---

### `USearchComponent`
**File:** `ActorComponents/SearchComponent.h`
**Parent:** `UActorComponent`

Timed search interaction for bodies, containers, and searchable actors.

| Property | Type | Description |
|---|---|---|
| `SearchDuration` | `float` | Time to complete search (default 10s) |
| `bSearched` | `bool` | Already searched (replicated) |
| `bAllowMultipleSearches` | `bool` | Allow re-searching |

**Delegate:** `OnSearchComplete`

---

### `ULockPickComponent`
**File:** `ActorComponents/LockPickComponent.h`
**Parent:** `UActorComponent`, implements `IServerActionInterface`

Pin-based lock picking with difficulty levels.

| Property | Type | Description |
|---|---|---|
| `PinCount` | `int32` | Number of pins (1-5, default 3) |
| `LockDifficulty` | `ELockDifficulty` | Easy/Medium/Hard |
| `Pins` | `TArray<FLockPinData>` | Generated pin data (replicated) |
| `bUnlocked` | `bool` | Lock state (replicated) |

**Delegate:** `OnUnlock`

---

### `UProximityHackComponent`
**File:** `ActorComponents/ProximityHackComponent.h`
**Parent:** `UHackComponent`

Extends hacking with proximity-based activation. Uses a sphere collision for range detection and GAS tags for status.

| Property | Type | Description |
|---|---|---|
| `ProximityRadius` | `float` | Detection range (default 600) |
| `ProximityHackingTag` | `FGameplayTag` | Tag applied to hacking player |
| `RewardItem` | `UItemBase*` | Item granted on hack completion |

---

## Characters

### `ADefaultCharacter`
**File:** `Characters/DefaultCharacter.h`
**Parent:** `ACharacter`, implements `IInteractable`, `IAbilitySystemInterface`, `IDetectable`

The main player character. Handles movement, input, inventory, interaction, ability system, and detection.

| Component | Type | Description |
|---|---|---|
| `CameraComponent` | `UCameraComponent` | First-person camera (found by class) |
| `InventoryComponent` | `UInventoryComponent` | Player inventory |
| `DetectionComponent` | `UDetectionComponent` | Detection meter |
| `EquippedItemMeshComp` | `UStaticMeshComponent` | Held item visual |

**Key input actions:** Move, Look, Sprint, Jump, Crouch, Interact, DropItem, Number keys (1-0)

---

### `AAICharacter`
**File:** `Characters/AICharacter.h`
**Parent:** `ACharacter`, implements `IAbilitySystemInterface`, `IInteractable`, `IDetectable`

Abstract base for all AI NPCs. Features:
- `UAIPerceptionComponent` with sight config (1500 radius, 90° FOV).
- Own `UAbilitySystemComponent` and `UPlayerAttributeSet`.
- `UDetectionComponent` and `USearchComponent`.
- Death handling with ragdoll physics and navigation cleanup.
- `MoneyToSubtract` (default -10,000) deducted from objective on death.

---

### `AGuardCharacter`
**File:** `Characters/GuardCharacter.h`
**Parent:** `AAICharacter`, implements `IGhostMirrorSource`

Security guard with:
- `GuardName` (replicated) and `PortraitTexture`.
- `NameText` (UTextRenderComponent) displayed above head.
- `AssignedLocker` reference for locker/fingerprint puzzle.
- Ghost mirroring — spawns `AGhostCharacterActor` on BeginPlay.
- Pre-alarm trigger on full detection of a player.
- Patrol via `NavNode` graph (`CurrentNode`, `PreviousNode`, `NextNode`).

---

### `ACivilianCharacter`
**File:** `Characters/CivilianCharacter.h`
**Parent:** `AAICharacter`

Civilian NPC with:
- `CivilianName`, `PortraitTexture`.
- `AssignedDesk` reference.
- `AllowedFloors` for movement restriction.
- `CurrentSearchable` / `LastSearchable` for schedule behavior.

---

### `ARobotGuardCharacter`
**File:** `Characters/RobotGuardCharacter.h`
**Parent:** `AGuardCharacter`

Repair-capable robot guard:
- Maintains `RepairQueue` of `IRepairable` actors.
- Listens to `FOnRepairRequested` delegates from all repairables in its era.
- Processes queue via StateTree events.
- Immune to health damage (empty `OnHealthChanged` override).

---

### `ADronePawn`
**File:** `Characters/DronePawn.h`
**Parent:** `APawn`, implements `IAbilitySystemInterface`, `IRepairable`

Security drone with:
- `USpotLightComponent` spotlight (green = idle, red = detected).
- Custom cone-based detection (not `UAIPerceptionComponent`).
- Smooth launch animation via timer-based lerp.
- Death → ragdoll physics + disable spotlight.
- Repairable — robot guard can restore drone functionality.
- Patrol via `NavNode` graph.

---

### `ADefaultSpectatorPawn`
**File:** `Characters/DefaultSpectatorPawn.h`
**Parent:** `ASpectatorPawn`

Post-elimination spectator. Cycles through alive players with A/D keys. Movement disabled.

---

## World Actors

### Security & Detection

| Actor | File | Description |
|---|---|---|
| `ASecurityCamera` | `Actors/SecurityCamera.h` | Panning camera with cone detection, scene capture |
| `AMetalDetector` | `Actors/MetalDetector.h` | Detects players carrying metal items |
| `AIllegalArea` | `Actors/Area/IllegalArea.h` | Box trigger applying illegal GAS effect |
| `AAlarmButton` | `Actors/AlarmButton.h` | Manual alarm trigger |

### Doors & Access

| Actor | File | Description |
|---|---|---|
| `ADoorBase` | `Actors/DoorBase.h` | Base door with lock, guard auto-open, open direction |
| `ADoubleDoorBase` | `Actors/DoubleDoorBase.h` | Two-panel door |
| `AVentBase` | `Actors/VentBase.h` | Vent entry/exit |
| `AGarageDoor` | `Actors/GarageDoor.h` | Large garage door |
| `AKeycardScanner` | `Actors/KeycardScanner.h` | Keycard-locked access |
| `AFingerprintScanner` | `Actors/FingerprintScanner.h` | Fingerprint-locked access |

### Items & Inventory

| Actor | File | Description |
|---|---|---|
| `AItemPickup` | `Actors/ItemPickup.h` | World item with search + pickup flow |
| `APastItemPickup` | `Actors/TimeObjects/PastItemPickup.h` | Past-era item variant |
| `AFutureItemPickup` | `Actors/TimeObjects/FutureItemPickup.h` | Future-era item variant |
| `ASearchableActor` | `Actors/SearchableActor.h` | Searchable container/object |
| `ALockerActor` | `Actors/LockerActor.h` | Guard locker (fingerprint source) |
| `ADeskActor` | `Actors/DeskActor.h` | Civilian desk with searchable items |

### Puzzles

| Actor | File | Description |
|---|---|---|
| `AWirePuzzleManager` | `Actors/Wire/WirePuzzleManager.h` | Wire cutting sequence manager |
| `AWireDeviceActor` | `Actors/Wire/WireDeviceActor.h` | Device with cuttable wires |
| `AWireActor` | `Actors/Wire/WireActor.h` | Individual wire |
| `ALeverManager` | `Actors/Lever/LeverManager.h` | Lever sequence validator |
| `ALeverActor` | `Actors/Lever/LeverActor.h` | Interactive lever |
| `AKeypadScanner` | `Actors/KeypadScanner/KeypadScanner.h` | Code entry keypad |
| `AKeypadButton` | `Actors/KeypadScanner/KeypadButton.h` | Keypad button |
| `ALaserManager` | `Actors/Laser/LaserManager.h` | Laser grid controller |
| `ALaserSensor` | `Actors/Laser/LaserSensor.h` | Individual laser beam |
| `ADisablingDeviceActor` | `Actors/DisablingDevice/DisablingDeviceActor.h` | Disabling device |
| `ADevicesManagerActor` | `Actors/DisablingDevice/DevicesManagerActor.h` | Devices manager |

### Infrastructure

| Actor | File | Description |
|---|---|---|
| `APowerGenerator` | `Actors/PowerGenerator.h` | Power source for connected systems |
| `ATerminal` | `Actors/Terminal.h` | Interactive terminal (launches NeonRunner mini-game) |
| `AComputer` | `Actors/Computers/Computer.h` | Computer terminal |
| `AArchiveComputer` | `Actors/Computers/ArchiveComputer.h` | Calendar/archive lookup |
| `ATeleporter` | `Actors/Teleporter.h` | Teleporter (requires PowerCell item) |
| `ACodeGenerator` | `Actors/CodeGenerator.h` | Generates random codes |
| `ADroneSpawner` | `Actors/DroneSpawner.h` | Spawns security drones |
| `ADroneSpawnerDisabler` | `Actors/DroneSpawnerDisabler.h` | Disables drone spawner |

### Navigation & Spawning

| Actor | File | Description |
|---|---|---|
| `ANavNode` | `Actors/PointActors/NavNode.h` | AI patrol waypoint (era + type + neighbors) |
| `ARefPointActor` | `Actors/PointActors/RefPointActor.h` | Reference point for timeline offset |
| `ACivilianSpawnPoint` | `Actors/PointActors/CivilianSpawnPoint.h` | Civilian spawn location |
| `ARandomPointActor` | `Actors/PointActors/RandomPointActor.h` | Randomized point selection |
| `ASearchableItemSpawnPoint` | `Actors/PointActors/SearchableItemSpawnPoint.h` | Searchable item spawn |
| `AProceduralLevelGenerator` | `Actors/ProceduralLevelGenerator.h` | Procedural level setup |

### Lobby

| Actor | File | Description |
|---|---|---|
| `ALobbyPlatformActor` | `Actors/LobbyPlatformActor.h` | Player seating platform with UI widgets |

### Timeline/Ghost

| Actor | File | Description |
|---|---|---|
| `AGhostCharacterActor` | `Actors/TimeObjects/GhostCharacterActor.h` | Cross-timeline ghost mirror |
| `APastDoor` / `AFutureDoor` | `Actors/TimeObjects/` | Era-specific door variants |
| `APastDoubleDoor` / `AFutureDoubleDoor` | `Actors/TimeObjects/` | Era-specific double doors |
| `APastVent` / `AFutureVent` | `Actors/TimeObjects/` | Era-specific vents |
| `APastPowerGenerator` / `AFuturePowerGenerator` | `Actors/TimeObjects/` | Era-specific generators |

### Miscellaneous

| Actor | File | Description |
|---|---|---|
| `ANewspaperActor` | `Actors/NewspaperActor.h` | Newspaper with date clue |
| `ASecurityDocumentActor` | `Actors/SecurityDocumentActor.h` | Security document with guard info |
| `ABullet` | `Actors/Projectiles/Bullet.h` | Projectile actor |
