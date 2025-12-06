# Echoes of Time

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue.svg)](https://www.unrealengine.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-See%20LICENSE-green.svg)](#license)

## Overview

**Echoes of Time** is an advanced co-op multiplayer stealth game built with Unreal Engine 5 that features innovative time-travel mechanics and team-based gameplay. Players work together across two timelines (Past and Future) to complete objectives while navigating complex puzzles, avoiding detection, and managing shared temporal consequences.

### Key Features

- 🕰️ **Time-Travel Mechanics**: Past actions directly affect the Future timeline in real-time
- 👥 **Cooperative Multiplayer**: Team-based gameplay with specialized roles for Past and Future teams
- 🎮 **Gameplay Ability System (GAS)**: Advanced ability framework for lockpicking, hacking, and searching
- 👻 **Ghost/Echo System**: Future team can see echoes of Past team's actions
- 🎯 **Stealth & Detection**: Sophisticated AI with cameras, guards, and progressive alarm states
- 🔓 **Interactive Puzzles**: Wire cutting, lever sequences, keypad codes, and device disabling
- 🎨 **Rich Inventory System**: Time-aware item management with cross-timeline invalidation
- 🌐 **Robust Networking**: Server-authoritative with client-side prediction for responsive gameplay
- 🎪 **Complete Lobby System**: Full session management with player readiness and team selection

### What Makes This Project Special

This README documents how the systems are implemented in detail, with pointers to the relevant classes and flows. It goes well beyond a feature list to explain how and why things work. The codebase emphasizes:

- **Clean Separation**: Gameplay, UI, and Networking are isolated into distinct systems
- **Authoritative Server Logic**: All gameplay state mutations are server-controlled
- **Client-Side Prediction**: Responsive input with prediction windows for smooth UX
- **Extensibility**: Designed for easy addition of new mechanics and content
- **Professional Architecture**: Production-ready patterns and best practices throughout

---

## Quick Start

```bash
# 1. Clone the repository
git clone https://github.com/Traveler3114/Echoes-of-Time.git

# 2. Open the project in Unreal Engine 5.7+
# Double-click EchoesOfTime.uproject

# 3. Build from the editor or with Visual Studio
# File → Generate Visual Studio project files
# Build Solution in Visual Studio 2022
```

For detailed setup instructions, see [Setup Instructions](#setup-instructions).

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)

- [High-Level Architecture](#high-level-architecture)
- [Networking & Replication Model](#networking--replication-model)
- [Teams, Time, and Timeline Linkage](#teams-time-and-timeline-linkage)
  - [Past vs Future Items](#past-vs-future-items)
  - [How Past destroys Future (invalidations)](#how-past-destroys-future-invalidations)
  - [Past/Future Doors and Double Doors](#pastfuture-doors-and-double-doors)
  - [Ghosts/Echoes and Past Echo ability](#ghostsechoes-and-past-echo-ability)
- [Stealth & Detection](#stealth--detection)
  - [Security Cameras](#security-cameras)
  - [Guard Detection, Pre-Alarm, and Alarm](#guard-detection-pre-alarm-and-alarm)
  - [Player HUD detection ring](#player-hud-detection-ring)
- [Gameplay Ability System (GAS)](#gameplay-ability-system-gas)
  - [Lockpicking](#lockpicking)
  - [Hacking](#hacking)
  - [Searching](#searching)
  - [Weapon Abilities (Aim/Fire)](#weapon-abilities-aimfire)
  - [Past Echo Toggle (Future ability)](#past-echo-toggle-future-ability)
  - [Gameplay Tags](#gameplay-tags)
- [Inventory & Items](#inventory--items)
  - [InventoryComponent](#inventorycomponent)
  - [ItemBase and ItemBaseWithAbilities](#itembase-and-itembasewithabilities)
  - [Pickups and Team-Aware Drops](#pickups-and-team-aware-drops)
  - [Keycard Scanners and Code System](#keycard-scanners-and-code-system)
  - [Fingerprint & Civilian linkage](#fingerprint--civilian-linkage)
- [Interaction System](#interaction-system)
  - [IInteractable, IRequiresItem, IKeycardUnlockable](#iinteractable-irequiresitem-ikeycardunlockable)
  - [Authority, Prediction, and Validation](#authority-prediction-and-validation)
- [Lobby, Match Flow, and Online Sessions](#lobby-match-flow-and-online-sessions)
  - [Lobby seating and player meta](#lobby-seating-and-player-meta)
  - [Readiness aggregation and host start](#readiness-aggregation-and-host-start)
  - [Leaving/Destroying Sessions/Kicking](#leavingdestroying-sessionskicking)
- [UI & Widgets](#ui--widgets)
  - [HUD/Overlay](#hudoverlay)
  - [Pause Menu & Settings](#pause-menu--settings)
  - [Calendar UI & Archive Computer](#calendar-ui--archive-computer)
  - [Lobby Widgets](#lobby-widgets)
- [Characters](#characters)
  - [DefaultCharacter (player)](#defaultcharacter-player)
  - [GuardCharacter (AI)](#guardcharacter-ai)
  - [CivilianCharacter (NPC)](#civiliancharacter-npc)
  - [DronePawn](#dronepawn)
- [Level Gadgets & World Actors](#level-gadgets--world-actors)
  - [Laser Sensors/Groups](#laser-sensorsgroups)
  - [Keypads & Code Generator](#keypads--code-generator)
  - [Lever Puzzles](#lever-puzzles)
  - [Wire Puzzles](#wire-puzzles)
  - [Disabling Devices](#disabling-devices)
  - [Metal Detector](#metal-detector)
  - [Vents](#vents)
  - [Alarm Button](#alarm-button)
  - [Desks, Computers, Newspapers, Nav Nodes, Spawn Points](#desks-computers-newspapers-nav-nodes-spawn-points)
- [Procedural Level Generator](#procedural-level-generator)
- [Money & Objective System](#money--objective-system)
- [Puzzle Completion Interface](#puzzle-completion-interface)
- [Project Structure](#project-structure)
- [Setup Instructions](#setup-instructions)
- [Development Workflow](#development-workflow)
- [Configuration Guide](#configuration-guide)
- [Gameplay Overview](#gameplay-overview)
- [Troubleshooting](#troubleshooting)
- [Performance Optimization](#performance-optimization)
- [FAQ](#faq)
- [Extending & Contribution](#extending--contribution)
- [Credits & Acknowledgments](#credits--acknowledgments)
- [License](#license)

---

## High-Level Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        Client Layer                             │
├─────────────────────────────────────────────────────────────────┤
│  UI Layer (UMG Widgets)                                         │
│  ├─ CharacterOverlay (HUD, Inventory, Status)                  │
│  ├─ Minigame Widgets (Lockpick, Hack, Search)                 │
│  ├─ Lobby Widgets (Team Selection, Ready State)               │
│  └─ Menus (Pause, Settings, Main Menu)                        │
├─────────────────────────────────────────────────────────────────┤
│  Input Layer (Enhanced Input System)                            │
│  └─ Input Contexts → Gameplay Ability System                   │
├─────────────────────────────────────────────────────────────────┤
│  Prediction Layer (Client-Side Prediction)                      │
│  └─ GAS Prediction Windows for Responsive Abilities            │
└─────────────────────────────────────────────────────────────────┘
                            ↕ RPC / Replication
┌─────────────────────────────────────────────────────────────────┐
│                        Server Layer                              │
├─────────────────────────────────────────────────────────────────┤
│  Game Flow (GameMode/GameState/PlayerState)                     │
│  ├─ Session Management (Lobby → Match transitions)             │
│  ├─ Alarm System (Pre-Alarm → Alarm → Level Restart)          │
│  ├─ Objective Tracking (Money collection, Puzzle completion)   │
│  └─ Team Management (Past/Future assignment)                   │
├─────────────────────────────────────────────────────────────────┤
│  Gameplay Systems                                               │
│  ├─ Ability System (GAS)                                       │
│  │   ├─ Abilities (Lockpick, Hack, Search, Aim, Fire)        │
│  │   ├─ Ability Tasks (UI integration)                        │
│  │   ├─ Gameplay Effects (Health, Status)                     │
│  │   └─ Gameplay Tags (Status, Team, Abilities)              │
│  ├─ Interaction System (IInteractable interfaces)             │
│  ├─ Detection System (Cameras, Guards, Lasers)                │
│  ├─ Time-Link System (Past → Future invalidation)             │
│  └─ Inventory System (Item management, Equipping)             │
├─────────────────────────────────────────────────────────────────┤
│  Actor Layer                                                     │
│  ├─ Characters (Player, Guard, Civilian, Drone)               │
│  ├─ Interactive Actors (Doors, Computers, Scanners)           │
│  ├─ Puzzle Actors (Levers, Wires, Devices, Keypads)          │
│  ├─ Items (Pickups, Past/Future variants)                     │
│  └─ Detection Actors (Cameras, Lasers, Metal Detectors)       │
├─────────────────────────────────────────────────────────────────┤
│  Component Layer                                                 │
│  ├─ InventoryComponent (Item storage and management)          │
│  ├─ AbilitySystemComponent (GAS integration)                  │
│  ├─ LockPickComponent (Lockpicking logic)                     │
│  ├─ HackComponent (Hacking progress)                          │
│  └─ SearchComponent (Search progress)                         │
└─────────────────────────────────────────────────────────────────┘
```

### Architecture Principles

**Server-Authoritative Multiplayer with Client QoL**:
- Client-side prediction for abilities (Aim/Fire, Hacking, Lockpick, Search) using GAS scoped prediction windows
- UI-only elements render client-side but are driven by replicated state or predicted tasks
- All gameplay state mutations happen on server, then replicate to clients
- Client predictions are validated and corrected by server if needed

**Core Systems Isolation**:
- **Gameplay/Actors**: Doors, Items, Scanners, Cameras, Guards, Puzzles
- **Components**: Inventory, LockPick, Hack, Search (reusable functionality)
- **Ability System**: Abilities, AbilityTasks, Attribute sets, Gameplay tags
- **UI Widgets**: HUD, Lobby, Minigames (clean separation from logic)
- **GameMode/GameState/PlayerState**: Flow control and replicated meta data

**Data Flow Pattern**:
1. **Input** → Enhanced Input System → Ability Activation
2. **Client Prediction** → Immediate UI response
3. **Server RPC** → Authority validates and executes
4. **Replication** → State changes propagate to all clients
5. **OnRep Functions** → Clients update visuals and UI

### Module Dependencies

```
EchoesOfTime (Runtime)
├─ Engine (Core UE functionality)
├─ UMG (UI widgets)
├─ GameplayAbilities (GAS framework)
├─ AdvancedSessions (Multiplayer session management)
├─ Niagara (VFX for lasers, effects)
├─ OnlineSubsystem (Steam/EOS integration)
└─ AIModule (Guard and Civilian AI)
```

---

## Networking & Replication Model

- Server holds authority on gameplay state mutations:
  - Interactions are executed on server after client-side initiation (prediction is used for responsiveness).
  - Core toggles/flags replicate via `DOREPLIFETIME` and drive `OnRep_*` handlers for visuals and UI updates.
- Selective network relevancy:
  - Future-only actors (e.g., `AFutureItemPickup`, `AGhostCharacterActor`) override `IsNetRelevantFor` to only replicate to Future-team players.
- OnRep-driven visuals:
  - Doors, alarms, pre-alarms, keypad code displays, camera panning, etc., update on replicated state changes.

---

## Teams, Time, and Timeline Linkage

Team membership is tracked in `ADefaultPlayerState::TeamName` with corresponding loose gameplay tags `Team.Past` or `Team.Future` added to the player's `AbilitySystemComponent`. Team tags drive spawn points, relevancy, and time-logic.

### Past vs Future Items

Classes:
- Past Pickup: `APastItemPickup`
- Future Pickup: `AFutureItemPickup`
- Generic Pickup: `AItemPickup`

Creation flow:
- When a Past item exists, it spawns a linked Future item at an offset derived from two `ARefPointActor`s in the world:
  - `APastItemPickup::BeginPlay` -> `FutureItemPickupOffset = ARefPointActor::GetOffsetBetweenFirstTwoRefPoints(...)`.
  - `APastItemPickup::SpawnLinkedFutureItem` spawns an `AFutureItemPickup` with the same `ItemInstanceID` and matches mesh/materials.
- Future pickup registers the instance ID with any inventory that collects it for later invalidation (see below).

Team-aware drops:
- `UItemBase::OnDroppedWithTeam` checks the dropping player's team tag:
  - Past tag -> spawn `APastItemPickup`
  - Future tag -> spawn `AFutureItemPickup`
  - No tag -> fallback `AItemPickup`

Visibility / relevancy:
- `AFutureItemPickup::IsNetRelevantFor` returns true only for Future team PlayerControllers, ensuring Past players don’t see future items.

### How Past destroys Future (invalidations)

The “time destruction” mechanic is implemented with instance IDs and a multicast delegate:

- Each item instance has a globally unique `FGuid ItemInstanceID`.
- When a Past pickup is removed from the world (picked up or destroyed), we broadcast invalidation:
  - `APastItemPickup::EndPlay` invokes `AFutureItemPickup::OnFutureItemInvalidated.Broadcast(ItemInstanceID)`.
- The Future pickup listens and destroys itself:
  - Future pickup constructor binds to the static `FFutureItemInvalidated` delegate; `HandleInvalidation` destroys the actor if IDs match.
- Players who already took the Future item:
  - `UInventoryComponent::RegisterFutureInstance` subscribes to the same delegate; on invalidation (`HandleFutureItemInvalidated`) it removes the matching item from the player’s inventory.
- Net-only for Future team:
  - Future items are only relevant to Future team clients, reducing replication churn.

Result: Changes in the Past invalidate the corresponding object/item in the Future across world and inventories.

### Past/Future Doors and Double Doors

- Base doors: `ADoorBase`, `ADoubleDoorBase` (replicate `bIsOpen`, `bIsLocked` with `OnRep_IsOpen` driving `OpenDoor/CloseDoor` Blueprint events).
- Past doors:
  - `APastDoor`, `APastDoubleDoor` broadcast `OnDoorStateChanged`/`OnDoubleDoorStateChanged` upon interactions and `OnRep_IsOpen`.
- Future doors mirror Past:
  - `AFutureDoor`, `AFutureDoubleDoor` bind to Past door events in `BeginPlay`.
  - When Past reports open/close, Future door calls `OpenDoor/CloseDoor` and updates its replicated `bIsOpen`.
- Guard auto-open:
  - Both bases have a `GuardOpenTrigger` box component. Overlap by a `AGuardCharacter` forces door open/close without changing lock/keycard state.

### Ghosts/Echoes and Past Echo ability

- Ghost actors: `AGhostCharacterActor` are visible only to Future team:
  - `IsNetRelevantFor` returns true only for Future-team PlayerControllers.
  - Ghost meshes mirror a source skeletal mesh (`IGhostMirrorSource::GetMirrorMesh`) and copy pose each Tick via `SetLeaderPoseComponent` (client-side, no server pose replication).
- Reveal control:
  - Toggle controlled by a combination of:
    - Security camera detecting guards (`IGhostMirrorSource::ShouldGhostBeVisible` returns camera visibility).
    - Player’s Past Echo status (local loose tag `Character.Status.PastEcho`).
- Past Echo ability (`UFutureGAPastEcho`):
  - Toggles the local loose tag `Character.Status.PastEcho` and executes gameplay cues:
    - `GameplayCue.PastEcho.Activated` -> `UGCN_PastEchoActivated` sets `IGhostRevealable::SetGhostRevealed(true)` on all relevant actors (local player only).
    - `GameplayCue.PastEcho.Deactivated` -> clears reveal.
- Guard implements `IGhostMirrorSource` and `ICameraDetectable` to feed camera state to the ghost.

---

## Stealth & Detection

### Security Cameras

- Actor: `ASecurityCamera`
- Detection loop (server-only timer):
  - Every `DetectionInterval` seconds, perform a sphere overlap around camera, then for each candidate do FOV cone test using dot product.
  - If inside view cone, call `ICameraDetectable::OnDetectedByCamera`. When lost, call `OnLostByCamera`.
- Pan behavior:
  - A replicated `PanOffset` animates yaw between `MinYaw..MaxYaw` with optional pause at limits; `OnRep_PanOffset` updates mesh and capture rotations on all clients.
- Debug draw is scaffolded (commented) for cone visualization.

### Guard Detection, Pre-Alarm, and Alarm

Guards escalate detection to pre-alarm and then alarm:

- Guard (`AGuardCharacter`) uses Perception Sight and also observes players’ Illegal status via gameplay tags.
- Timeline logic (`GuardTimeline`) drives a progress value [0..1] during which the player’s HUD detection ring fills (see below).
- When fully filled:
  - Guard sets `TargetActor` and, server-side, starts Pre-Alarm via GameState: `ADefaultGameState::StartPreAlarm(this, PreAlarmDuration)`.
  - Pre-Alarm: replicated `bPreAlarmActive` and `PreAlarmEndTime` broadcast `OnPreAlarmStarted`.
  - `ADefaultGameMode` listens: schedules `PreAlarmTimeout` to start full Alarm at `PreAlarmEndTime` unless canceled.
- Cancelation:
  - If guard loses sight/illegal state clears: `GuardTimeline` reverses; on reaching 0 it calls `ADefaultGameState::CancelPreAlarm(this)`.
- Alarm:
  - `ADefaultGameState::StartAlarm` sets `bAlarmActive` + `AlarmEndTime` and cancels any pre-alarm (broadcasts `OnPreAlarmCanceled`).
  - `ADefaultGameMode::OnAlarmStarted` schedules a server travel restart after remaining time. Canceling alarm clears the restart timer.
  - Safety: `RestartLevel` validates current time against `AlarmEndTime` with tolerance to avoid early restarts; also aborts if alarm was canceled before the timer fired.
- Player UI:
  - `ADefaultPlayerController` binds to replicated events:
    - Pre-Alarm active: yellow status “Guard spotted you! Alarm in X s”.
    - Alarm active: red status “ALARM - Restart in X s”.
    - Cancel events clear the status text.
- Laser sensors can trigger immediate Alarm via GameState (see below).

Summary timeline:
1) Guard sees illegal player -> detection UI fills -> on full detection -> Pre-Alarm starts with duration D.
2) If still not canceled when D elapses -> Alarm starts, countdown displayed, level restart scheduled.
3) Canceling during pre-alarm (lost sight) stops countdown; canceling during alarm stops restart.

### Player HUD detection ring

- `UCharacterOverlay::UpdateDetectionWidgetForGuard` manages per-guard `UDetectionWidget` instances on a canvas ring around the screen center.
- Angle calculation:
  - Guard computes the angle of its position relative to player camera forward/right to place a detection slice at the correct direction.
  - The widget’s progress bar color interpolates green->yellow->orange->red, locking to red when `bIsLocked == true`.

---

## Gameplay Ability System (GAS)

GAS powers interaction minigames and player actions with prediction and UI tasks.

### Lockpicking

- Ability: `UDefaultGALockPick`
- Component: `ULockPickComponent`
  - Procedurally generates pins (count and tolerance vary with difficulty), replicated to clients.
  - Server authoritative pin checks; clients view via widget.
- Task: `ULockPickAbilityTask`
  - Shows `ULockPickWidget` with pins. Mouse movement controls angle; Space confirms.
  - On confirm, the player controller RPC `ServerTryLockPick` asks server to evaluate angle (`ULockPickComponent::ServerTrySetPin`).
- Doors with locks (`ADoorBase`/`ADoubleDoorBase`) own a `ULockPickComponent`. On unlock, door opens.

### Hacking

- Ability: `UDefaultGAHack`
- Component: `UHackComponent` (replicated)
  - Ticks a timed “hack in progress” until complete.
- Task: `UHackAbilityTask`
  - Displays circular `UHackWidget` progress UI; Escape cancels.
  - Ability uses LocalPredicted execution to start local UI quickly, then reconciles with server.

### Searching

- Ability: `UDefaultGASearch`
- Component: `USearchComponent` (replicated)
  - Similar to Hack: timed search, `OnSearchComplete` when done.
- Task: `USearchAbilityTask` shows `USearchWidget`.
- Integration example: `ASearchableActor` binds `OnSearchComplete` and can reward items (e.g., `ACupActor` gives a fingerprint item on server).

### Weapon Abilities (Aim/Fire)

- Aim: `UPistolGAAim`
  - LocalPredicted; sets `Character.Status.Aiming` and plays a camera timeline for aim FOV/offset locally.
- Fire: `UPistolGAFire`
  - Spawns `ABullet` at weapon muzzle socket; bullet applies a GameplayEffect on overlap to any `IAbilitySystemInterface` target, then destroys.

### Past Echo Toggle (Future ability)

- `UFutureGAPastEcho` is a one-shot toggle ability:
  - Adds/removes loose tag `Character.Status.PastEcho`.
  - Executes `GameplayCue.PastEcho.Activated/Deactivated` to show/hide ghosts locally for Future players.

### Gameplay Tags

Defined in `EOTGameplayTags.*`:
- Status: Block movement/look, Illegal, Hacking, LockPicking, Searching, Aiming, Firing, PastEcho.
- Abilities: LockPick, Hack, Search, Future.PastEcho, Weapon.Pistol.Aim/Fire.
- Cues: PastEcho.Activated/Deactivated.

Abilities are mapped to input via `UInputMappingSet` and `UAbilityInputSet` and activated by matching input tags on their `GameplayAbilitySpec`.

---

## Inventory & Items

### InventoryComponent

`UInventoryComponent` (replicated):
- Fixed-size slots with `FInventorySlot { UItemBase* ItemAsset, FGuid ItemInstanceID }`.
- Active slot index replicated. Changing active slot calls `OnUnequipped` on previous item and `OnEquipped` on the new one.
- Equip/Unequip grant/remove item effects/abilities (see below).
- Server RPCs: set active slot, drop item (server computes safe ground point), add item.

Time-link registration:
- When picking a Future item, `RegisterFutureInstance(ItemInstanceID)` subscribes to global invalidations to auto-remove invalidated items.

### ItemBase and ItemBaseWithAbilities

- `UItemBase`:
  - Optional `GrantedGameplayEffects` applied on `OnEquipped`, removed on `OnUnequipped`/drop.
  - `OnDropped` spawns a generic `AItemPickup`; `OnDroppedWithTeam` spawns Past/Future variant based on owner’s team tag.
- `UItemBaseWithAbilities`:
  - Grants abilities from `UAbilityInputSet` on equip, clears them on unequip/drop.

### Pickups and Team-Aware Drops

- `AItemPickup` replicates mesh and destroys itself when picked up and added to inventory.
- `APastItemPickup`:
  - Auto-spawns linked `AFutureItemPickup` at offset and copies mesh/materials.
  - On destruction, broadcasts invalidation by `ItemInstanceID`.
- `AFutureItemPickup`:
  - Only relevant to Future team.
  - On invalidation, destroys itself; inventories remove invalidated items.

### Keycard Scanners and Code System

- `AKeycardScanner` (simple) uses `EItemType RequiredKeycardType` to allow calling `UnlockWithKeycard` on linked actor when interacted with correct item.

- `AKeypadScanner` (full keypad panel):
  - Spawns `AKeypadButton` grid; replicated `EnteredCode` drives display.
  - `SetCodeWithExpiry(Code, Lifetime)` sets a temporary correct code; server clears after expiry.
  - Entering code with `#` compares to `CorrectCode`, shows “READY” on success, “WRONG” otherwise (then clears).
  - `TryUnlock` calls `IKeycardUnlockable::UnlockWithKeycard` if `bCodeCorrect`.
  - `AComputer` stores codes; `ACodeGenerator` (Future) can procedurally assign codes to multiple `AKeypadScanner`s, display remaining times, and replicate the UI text via `CodesDisplayText`.

### Fingerprint & Civilian linkage

- `UFingerprintItem` has `OwnerCivilian` soft reference.
- `ACupActor`:
  - On search complete (server), loads the `DA_Fingerprint` item asset, sets `OwnerCivilian` to `LinkedCivilian`, and adds it to the interactor’s inventory.

---

## Interaction System

### IInteractable, IRequiresItem, IKeycardUnlockable

- `IInteractable::Interact(AActor* Interactor)` called on actors to perform their main interaction (doors toggle open/close, computers start hack, scanners unlock, etc).
- `IRequiresItem::IsCorrectItem(UItemBase* Item)` lets actors gate interactions by item (e.g., keycard scanners).
- `IKeycardUnlockable::UnlockWithKeycard(AActor* Interactor)` allows scanners to invoke door opening without bypassing locks/keycard state.

### Authority, Prediction, and Validation

- `ADefaultCharacter::HandleInteract`:
  - Client predicts UI and calls `IInteractable::Execute_Interact` for responsiveness.
  - Always calls `ServerHandleInteract(TargetActor)`.
- Server validates requirements (`IRequiresItem::IsCorrectItem`) and executes authoritative interaction.
- Abilities use LocalPredicted execution with scoped prediction windows to start client UI immediately; server runs the authoritative start/cancel/end.

---

## Lobby, Match Flow, and Online Sessions

### Lobby seating and player meta

- `ALobbyGameMode` manages:
  - Auto-seating players on `ALobbyPlatformActor`s (spawn matching lobby pawns, show per-slot UI).
  - Binding to `ADefaultPlayerState` delegates to recompute ready aggregates.
- `ALobbyPlatformActor`:
  - Replicates `OccupantPlayerState`.
  - Shows `UPlayerLobbyInfo` (name, avatar, ready, team) for the occupant, with a kick button visible only to the listen-server host (and not on their own slot).
  - Friend list toggle UI (`UOpenFriendsListButton`/`UFriendList`) shown only when the platform is unoccupied.
- `ADefaultPlayerState`:
  - Replicates display name, avatar index, ready state, team name.
  - Adds/removes `Team.Past`/`Team.Future` tags on the ASC.
  - `CopyProperties` preserves team across seamless travel.

### Readiness aggregation and host start

- `ALobbyGameMode::RecalculateAggregates` computes (Total, Ready, AllReady) from `AGameStateBase::PlayerArray`:
  - Replicated via `ALobbyGameState` (TotalPlayers, ReadyPlayers, bAllPlayersReady).
  - Host’s Lobby UI start button is enabled based on settings (`bRequireAllReadyToStart`).
- Starting the match:
  - Host calls `ServerStartGame` -> GameMode sets loading screens on all clients; then server travels to `MatchMapPath` with `?listen`.

### Leaving/Destroying Sessions/Kicking

- Host “Leave Lobby” flow:
  - Sends all remote clients to main menu (loading, then `ClientTravel`).
  - Shows host’s own loading screen.
  - Tries to destroy the online session via OSS; upon completion (or fallback timer), server travels host to `MainMenuMapPath`.
- Kicking:
  - Host triggers `ALobbyGameMode::KickPlayerByPlayerState`, which instructs the client to show loading & travel to main menu, then server closes their connection shortly after.

- GameInstance:
  - `UDefaultGameInstance` extends `UAdvancedFriendsGameInstance` and exposes `CreateSession` as a `BlueprintImplementableEvent` used by the Main Menu (`UMainMenuWidget`).

---

## UI & Widgets

### HUD/Overlay

- `ACharacterHUD` creates `UCharacterOverlay` on BeginPlay and binds to `UInventoryComponent::OnInventoryChanged`.
- `UCharacterOverlay`:
  - Inventory bar: dynamically adds item icons with active slot highlight.
  - Status text helpers: `SetStatusText`, `SetStatusTextWithColor`.
  - Ping text colorizes based on thresholds.
  - Detection ring: Places per-guard `UDetectionWidget` on a circle around the screen and adjusts bar angle/color.

### Pause Menu & Settings

- `UPauseMenuWidget` provides Resume/Settings/Quit buttons. Resume broadcasts `OnPauseMenuResumed`.
- `USettingsWidget`:
  - Reads `UGameUserSettings` on construct; exposes Resolution, Window Mode, Render Scale, and quality sliders; Apply persists and calls `ApplySettings`.

### Calendar UI & Archive Computer

- `AArchiveComputer` (Past or Future) builds a date list:
  - Past: Lists civilians whose `AComputer` has non-empty `StoredCode` for the Past date.
  - Future: Finds `ACodeGenerator` targets with `TargetCivilian` for the Future date.
  - Sends `ClientShowCalendarWidget` to local player with an array of `FCalendarCivilianRecord`.
- `UCalendarWidget` is a three-stage navigator: Years -> Months -> Days -> Results; Esc key goes back; shows results via `UCalendarResultWidget`.

### Lobby Widgets

- `ULobbyUI`: Buttons for Ready, Change Team, Start (host only), Leave; reflects ready/team states via `ADefaultPlayerState` bindings.
- `UPlayerLobbyInfo`: Shows player name, avatar, ready/teams; host sees Kick button on other players.
- `UOpenFriendsListButton` & `UFriendList`: Simple toggle widgets to show/hide friend panel around platforms.

---

## Characters

### DefaultCharacter (player)

- Input via Enhanced Input; abilities mapped by `UInputMappingSet` / `UAbilityInputSet`.
- Interaction:
  - Performs line trace from camera; highlights `IInteractable` actors via `SetHighlighted`.
  - On Interact: client predicts, server validates and executes.
- Inventory:
  - Active slot switching: number keys 1–0, server RPC updates active slot, HUD updates.
  - On equip, weapon mesh attaches to hand (`EquippedItemMeshComp`) and uses item mesh/scale/rotation.
- Movement:
  - Sprint replicated (`bIsSprinting` with `OnRep_SprintState`), crouch checks, jump blocks when movement blocked tag present.
- Camera:
  - Aim camera timeline alters location/rotation; camera pitch replicates to server (`Pitch` with `OnRep_Pitch`) to keep server-informed look.

### GuardCharacter (AI)

- Perception via `UAIPerceptionComponent` with Sight config.
- On seeing an illegal player, raises detection timeline; on completion triggers Pre-Alarm.
- Spawns a `AGhostCharacterActor` server-side with an offset computed from two `ARefPointActor`s; ghost visibility driven by camera view and Past Echo status.
- Health:
  - Damage via `UAbilitySystemComponent` health attribute; on 0 cancels alarms and destroys ghost and self.

### CivilianCharacter (NPC)

- `ACivilianCharacter`:
  - NPC character with AI perception and ability system.
  - `CivilianName` and `PortraitTexture` for identification/UI.
  - `AssignedDesk` links civilian to their workspace.
  - Implements `IDetectable` for player detection events.
  - `AIPerceptionComponent` with `UAISenseConfig_Sight` for player awareness.
  - `OnHealthChanged` callback via GAS attribute set.
  - `TimelineEra` support for Past/Future variants.

### DronePawn

- `ADronePawn`:
  - AI-controlled drone with cone-based detection.
  - Server-side `DetectionUpdate()` runs on timer (`DetectionInterval`).
  - `DetectionDistance` and `ViewConeAngle` configure detection cone.
  - Replicated `DetectedActor` with `OnRep_DetectedActor` for visual feedback.
  - `DroneSpotLight` component for visual detection indicator.
  - Optional `bDrawDebugCone` for development visualization.

---

## Level Gadgets & World Actors

### Laser Sensors/Groups

- `ALaserSensor`:
  - Start/End meshes define a laser line; `UBoxComponent` stretches between them and detects overlaps.
  - Replicated `bIsActive` toggles Niagara beam visibility and collision.
  - On overlap with `ADefaultCharacter` (server), calls `ADefaultGameState::StartAlarm()`.
- `ALaserGroup`:
  - Owns a list of sensors and can randomly activate `NumToShow` sensors at `RandomizeInterval` (server).
  - Conveniently toggles batches with Fisher–Yates shuffle for variety.

### Keypads & Code Generator

- `AKeypadScanner` spawns a 4x3 keypad of `AKeypadButton` actors; manages code entry and display, and unlocks linked actors with `IKeycardUnlockable`.
- `ACodeGenerator` (Future):
  - Takes a list of `AKeypadScanner`s and assigns random numeric codes with individual expiries, sets their display and server-replicated text UI.
  - Requires a matching `UFingerprintItem` for its `TargetCivilian` to interact successfully (`IRequiresItem::IsCorrectItem` returns true only when `OwnerCivilian` matches).

### Lever Puzzles

- `ALeverActor`:
  - Interactive lever actor that can be part of a puzzle sequence.
  - Supports `ETimelineEra` (Past/Future) and replicated `bActivated` state.
  - `OnLeverInteracted` delegate fires when player interacts, allowing managers to track activation.
  - Solo mode (`bIsSolo`) allows standalone levers without puzzle sequence.
  - `SpawnLocationName` replicated for UI hints to players.

- `ALeverManager`:
  - Manages a sequence of `ALeverActor`s for ordered puzzle completion.
  - Replicated `Order` array defines the correct activation sequence.
  - `ProgressIndex` tracks current progress; incorrect activation resets puzzle.
  - `OnLeverPuzzleCompleted` delegate and `CompletionTarget` (implements `IPuzzleCompletionReceiver`) for triggering effects on success.
  - Server-authoritative with OnRep-driven client feedback.

### Wire Puzzles

- `EWireColor` enum: Red, Green, Blue, Yellow.

- `AWireActor`:
  - Colored wire that can be cut via `USearchComponent` interaction.
  - Replicated `bIsCut` state with `OnRep_CutState` for visual updates.
  - `OnWireCut` delegate notifies managers when cut.
  - `WireColor` property determines puzzle validation.

- `AWireDeviceActor`:
  - Container for multiple `AWireActor`s at a single location.
  - `WireActors` array holds child wires.
  - `SpawnLocationName` replicated for UI hints.
  - `TimelineEra` support for Past/Future placement.

- `AWirePuzzleManager`:
  - Manages wire puzzle sequence across multiple devices.
  - Replicated `DeviceOrder` and `CorrectWireColors` arrays define the expected sequence.
  - `ProgressIndex` tracks completion; cutting wrong wire/color resets puzzle.
  - `OnWirePuzzleCompleted` delegate and `CompletionTarget` for success handling.
  - Binds to all wires in `PuzzleDevices` on BeginPlay for unified event handling.

### Disabling Devices

- `ADisablingDeviceActor`:
  - Device that can be disabled via `USearchComponent` interaction.
  - Replicated `bIsActive` state (default true) with `OnRep_DeviceState`.
  - `bIsSolo` mode for standalone devices; `OnSoloDeviceDisabled` delegate fires on disable.
  - `OnDeviceStateChanged` delegate for manager notification.
  - `SpawnLocationName` replicated for UI hints.

- `ADevicesManagerActor`:
  - Manages a group of `ADisablingDeviceActor`s.
  - Puzzle completes when all devices in `Devices` array are disabled.
  - Replicated `bPuzzleCompleted` with `OnRep_PuzzleCompleted`.
  - `OnPuzzleCompleted` delegate and `CompletionTarget` for success handling.
  - Server-side `CheckPuzzleState()` validates completion after each device change.

### Metal Detector

- `AMetalDetector`:
  - Overlap-based detection that triggers alarm when enabled.
  - Implements `IPuzzleCompletionReceiver` to disable via `OnPuzzleCompleted_Implementation()`.
  - Replicated `bEnabled` with `OnRep_Enabled` for visual state.
  - `TriggerBox` collision detects `ADefaultCharacter` overlap (server-only).

### Vents

- `AVent`:
  - Searchable vent that opens after `USearchComponent` completes.
  - Replicated `bIsOpen` with `OnRep_OpenState` calls `OpenVent()`/`CloseVent()` Blueprint events.
  - `TimelineEra` support for Past/Future variants.
  - Interaction triggers search ability flow.

### Alarm Button

- `AAlarmButton`:
  - Simple interactable that triggers immediate alarm when used.
  - `TimelineEra` support.
  - `Interact_Implementation` calls `ADefaultGameState::StartAlarm()`.

### Desks, Computers, Newspapers, Nav Nodes, Spawn Points

- `ADeskActor` replicates `StaffName` and owns references to desk computer and spawn points for searchable items.
- `AComputer` integrates `UHackComponent` and replicates `StoredCode` (reveals via on-screen debug on hack complete; wire to UI as needed).
- `ANewspaperActor` replicates a `NewspaperDateString` and updates its `UTextRenderComponent` via `OnRep`.
- `ANavNode` defines patrol graph edges for guards with helpers to pick next node.
- `ACivilianSpawnPoint`, `ARandomPointActor` hold simple timeline/era metadata for procedural systems.

---

## Procedural Level Generator

`AProceduralLevelGenerator` orchestrates runtime spawning and puzzle setup:

### Date System
- `FRandomDate` struct (Year, Month, Day) with comparison operators.
- `PastDate` and `FutureDate` replicated; Future date always > Past date.
- Used for newspaper text, calendar puzzle, and code generator timing.

### Spawn Flow
- `HandlePastSpawns()` and `HandleFutureSpawns()` run server-side on BeginPlay.
- `HandleEraSpawns(ETimelineEra)` spawns civilians at `ACivilianSpawnPoint`s, pairs with `ADeskActor`s, generates unique names.
- `SpawnCivilianDeskItems()` places searchable items (cups with fingerprints) at desk spawn points.

### Puzzle Setup
- Wire puzzles: Spawns `AWireDeviceActor`s at tagged `ARandomPointActor`s, adds to `AWirePuzzleManager`, generates random sequence stored in `PastWireDeviceSequence`.
- Lever puzzles: Spawns `ALeverActor`s, adds to `ALeverManager`, shuffles order, stores in `PastLeverOrderString`.
- Disabling devices: Spawns `ADisablingDeviceActor`s, adds to `ADevicesManagerActor`.
- Keypad codes: Assigns random 4-digit code to one computer and matching `AKeypadScanner`s.
- Newspapers: Spawns at random points with date text matching `PastDate`/`FutureDate`.

### Helper Template
```cpp
SpawnActorsOnRandomPointsAndAddToManager<TSpawnActor, TManagerActor>(...)
```
Generic function to spawn actors at tagged points and register with managers.

---

## Money & Objective System

`ADefaultGameState` includes a money collection objective:

- `TargetMoneyAmount`: Goal amount players must collect.
- `CurrentMoneyCollected`: Replicated progress with `OnRep_CurrentMoneyCollected`.
- `OnMoneyCollectedChanged` delegate broadcasts (Current, Target) for UI updates.
- `AddCollectedMoney(Amount)` server-callable to increment progress.

---

## Puzzle Completion Interface

`IPuzzleCompletionReceiver` interface allows actors to respond to puzzle completion:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
void OnPuzzleCompleted();
```

Implemented by `AMetalDetector`, laser managers, and other puzzle-gated actors. Managers call `Execute_OnPuzzleCompleted(CompletionTarget)` when puzzles are solved.

---

## Project Structure

```
/Source/EchoesOfTime
  │
  ├── EchoesOfTime.h/cpp              // Module definition
  ├── EchoesOfTime.Build.cs           // Build configuration with dependencies
  ├── DefaultGameInstance.h/cpp       // Game instance extending UAdvancedFriendsGameInstance
  ├── DefaultPlayerState.h/cpp        // Player state with team, ready, avatar, GAS integration
  ├── ProceduralLevelGenerator.h/cpp  // Runtime level population and puzzle setup
  ├── TimelineEra.h/cpp               // ETimelineEra enum (Past/Future)
  │
  ├── /AbilitySystem
  │   ├── EOTGameplayTags.h/cpp              // Native gameplay tag declarations
  │   ├── DefaultAbilitySystemComponent.h/cpp // Custom ASC
  │   ├── /Abilities
  │   │   ├── DefaultGALockPick.h/cpp        // Lockpicking ability
  │   │   ├── DefaultGAHack.h/cpp            // Hacking ability
  │   │   ├── DefaultGASearch.h/cpp          // Searching ability
  │   │   ├── FutureGAPastEcho.h/cpp         // Past Echo toggle (Future team)
  │   │   └── /Weapons
  │   │       ├── PistolGAAim.h/cpp          // Pistol aim ability
  │   │       └── PistolGAFire.h/cpp         // Pistol fire ability
  │   ├── /AbilityTasks
  │   │   ├── HackAbilityTask.h/cpp          // Hack UI task
  │   │   ├── LockPickAbilityTask.h/cpp      // LockPick UI task
  │   │   └── SearchAbilityTask.h/cpp        // Search UI task
  │   ├── /AttributeSets
  │   │   └── PlayerAttributeSet.h/cpp       // Health attribute
  │   └── /GameplayCues
  │       ├── GCN_PastEchoActivated.h/cpp    // Past Echo visual effect
  │       └── GCN_PastEchoDeactivated.h/cpp  // Past Echo deactivation
  │
  ├── /ActorComponents
  │   ├── InventoryComponent.h/cpp    // Player inventory management
  │   ├── LockPickComponent.h/cpp     // Lock picking mini-game logic
  │   ├── HackComponent.h/cpp         // Hacking progress logic
  │   └── SearchComponent.h/cpp       // Search progress logic
  │
  ├── /Actors
  │   ├── DoorBase.h/cpp              // Base door with lock/keycard
  │   ├── DoubleDoorBase.h/cpp        // Double door variant
  │   ├── ItemPickup.h/cpp            // Generic item pickup
  │   ├── KeycardScanner.h/cpp        // Simple keycard reader
  │   ├── SecurityCamera.h/cpp        // Detection camera with pan
  │   ├── SearchableActor.h/cpp       // Searchable container
  │   ├── CupActor.h/cpp              // Cup with fingerprint item
  │   ├── DeskActor.h/cpp             // Desk with staff info
  │   ├── NewspaperActor.h/cpp        // Date-display newspaper
  │   ├── CodeGenerator.h/cpp         // Future code generator terminal
  │   ├── AlarmButton.h/cpp           // Manual alarm trigger
  │   ├── MetalDetector.h/cpp         // Player detection trigger
  │   ├── Vent.h/cpp                  // Searchable vent
  │   ├── LobbyPlatformActor.h/cpp    // Lobby player slot
  │   │
  │   ├── /Computers
  │   │   ├── Computer.h/cpp           // Hackable computer
  │   │   └── ArchiveComputer.h/cpp    // Calendar/archive access
  │   │
  │   ├── /DisablingDevice
  │   │   ├── DisablingDeviceActor.h/cpp    // Device to disable
  │   │   └── DevicesManagerActor.h/cpp     // Manages device puzzle
  │   │
  │   ├── /KeypadScanner
  │   │   ├── KeypadButton.h/cpp       // Individual keypad button
  │   │   └── KeypadScanner.h/cpp      // Full keypad panel
  │   │
  │   ├── /Laser
  │   │   ├── LaserSensor.h/cpp        // Single laser beam
  │   │   └── LaserManager.h/cpp       // Laser group controller
  │   │
  │   ├── /Lever
  │   │   ├── LeverActor.h/cpp         // Interactive lever
  │   │   └── LeverManager.h/cpp       // Lever sequence puzzle
  │   │
  │   ├── /PointActors
  │   │   ├── NavNode.h/cpp            // Guard patrol node
  │   │   ├── RefPointActor.h/cpp      // Reference offset point
  │   │   ├── RandomPointActor.h/cpp   // Random spawn point
  │   │   ├── CivilianSpawnPoint.h/cpp // Civilian spawn location
  │   │   └── SearchableItemSpawnPoint.h/cpp // Item spawn location
  │   │
  │   ├── /Projectiles
  │   │   └── Bullet.h/cpp             // Pistol projectile
  │   │
  │   ├── /TimeObjects
  │   │   ├── PastDoor.h/cpp           // Past door (broadcasts state)
  │   │   ├── FutureDoor.h/cpp         // Future door (listens to Past)
  │   │   ├── PastDoubleDoor.h/cpp     // Past double door
  │   │   ├── FutureDoubleDoor.h/cpp   // Future double door
  │   │   ├── PastItemPickup.h/cpp     // Past item (spawns Future item)
  │   │   ├── FutureItemPickup.h/cpp   // Future item (invalidated by Past)
  │   │   └── GhostCharacterActor.h/cpp // Ghost mirror of guard
  │   │
  │   └── /Wire
  │       ├── WireActor.h/cpp          // Cuttable wire
  │       ├── WireDeviceActor.h/cpp    // Wire device container
  │       └── WirePuzzleManager.h/cpp  // Wire puzzle controller
  │
  ├── /Characters
  │   ├── DefaultCharacter.h/cpp      // Player character
  │   ├── GuardCharacter.h/cpp        // AI guard with detection
  │   ├── CivilianCharacter.h/cpp     // NPC civilian
  │   └── DronePawn.h/cpp             // Detection drone
  │
  ├── /Controllers
  │   ├── DefaultPlayerController.h/cpp    // Gameplay controller
  │   ├── LobbyPlayerController.h/cpp      // Lobby controller
  │   └── MainMenuPlayerController.h/cpp   // Menu controller
  │
  ├── /DataAssets
  │   ├── ItemBase.h/cpp              // Base item data asset
  │   ├── InputMappingSet.h/cpp       // Input action mappings
  │   └── /AbilitySets
  │       ├── AbilityInputSet.h/cpp   // Ability-to-input mappings
  │       └── DefaultGASet.h/cpp      // Default granted abilities
  │
  ├── /GameModes
  │   ├── DefaultGameMode.h/cpp       // Gameplay mode with alarm handling
  │   └── LobbyGameMode.h/cpp         // Lobby management mode
  │
  ├── /GameStates
  │   ├── DefaultGameState.h/cpp      // Alarm, pre-alarm, money tracking
  │   └── LobbyGameState.h/cpp        // Lobby ready aggregation
  │
  ├── /Interfaces
  │   ├── IInteractable.h/cpp         // Interaction interface
  │   ├── IRequiresItem.h/cpp         // Item requirement check
  │   ├── IKeycardUnlockable.h/cpp    // Keycard unlock action
  │   ├── IDetectable.h/cpp           // Detection callbacks
  │   ├── IGhostMirrorSource.h/cpp    // Ghost mesh source
  │   ├── IGhostRevealable.h/cpp      // Ghost reveal toggle
  │   └── IPuzzleCompletionReceiver.h/cpp // Puzzle completion callback
  │
  └── /Widgets
      ├── DetectionWidget.h/cpp       // Per-guard detection indicator
      ├── PauseMenuWidget.h/cpp       // Pause menu
      ├── SettingsWidget.h/cpp        // Graphics/audio settings
      │
      ├── /HUD
      │   ├── CharacterHUD.h/cpp      // Main HUD class
      │   ├── CharacterOverlay.h/cpp  // Inventory/status overlay
      │   ├── HackWidget.h/cpp        // Hack progress UI
      │   ├── LockPickWidget.h/cpp    // LockPick mini-game UI
      │   └── SearchWidget.h/cpp      // Search progress UI
      │
      ├── /Lobby
      │   ├── LobbyUI.h/cpp           // Main lobby widget
      │   ├── PlayerLobbyInfo.h/cpp   // Per-player info display
      │   ├── FriendList.h/cpp        // Friends panel
      │   ├── FriendWidget.h/cpp      // Single friend entry
      │   └── OpenFriendsListButton.h/cpp // Friend list toggle
      │
      ├── /MainMenu
      │   └── MainMenuWidget.h/cpp    // Main menu
      │
      └── /Calendar
          ├── CalendarWidget.h/cpp       // Calendar navigation
          ├── CalendarButtonWidget.h/cpp // Date button
          └── CalendarResultWidget.h/cpp // Search results display
```

---

## Setup Instructions

### Prerequisites

#### System Requirements

**Minimum**:
- **OS**: Windows 10 64-bit / Ubuntu 20.04 / macOS 12.0+
- **CPU**: Quad-core Intel or AMD, 2.5 GHz or faster
- **RAM**: 16 GB
- **GPU**: NVIDIA GTX 1060 / AMD RX 580 or better
- **Storage**: 50 GB available space (SSD recommended)
- **Network**: Broadband internet connection for multiplayer

**Recommended**:
- **CPU**: 8-core Intel i7 / AMD Ryzen 7, 3.0 GHz or faster
- **RAM**: 32 GB
- **GPU**: NVIDIA RTX 3070 / AMD RX 6800 or better
- **Storage**: 100 GB SSD

#### Software Requirements

- **Unreal Engine**: 5.7 or higher ([Download](https://www.unrealengine.com/download))
- **IDE**: Visual Studio 2022 Community or Professional with:
  - Game Development with C++ workload
  - Unreal Engine installer (optional)
  - Windows 10 SDK (10.0.18362 or newer)
- **Git**: For version control ([Download](https://git-scm.com/downloads))
- **Git LFS**: For large binary files ([Download](https://git-lfs.github.com/))

### Installation Steps

#### 1. Install Unreal Engine

```bash
# Via Epic Games Launcher (Recommended)
1. Install Epic Games Launcher
2. Navigate to Unreal Engine tab
3. Install Unreal Engine 5.7+

# Via Source (Advanced)
git clone https://github.com/EpicGames/UnrealEngine.git
cd UnrealEngine
./Setup.bat  # Windows
./Setup.sh   # Linux/Mac
./GenerateProjectFiles.bat  # Windows
```

#### 2. Clone Repository

```bash
# Clone with Git LFS support
git lfs install
git clone https://github.com/Traveler3114/Echoes-of-Time.git
cd Echoes-of-Time

# Verify LFS files are downloaded
git lfs pull
```

#### 3. Enable Required Plugins

The project requires these plugins (most are already configured in `.uproject`):

**Built-in Plugins**:
- ✅ GameplayAbilities (GAS)
- ✅ Enhanced Input
- ✅ Niagara
- ✅ OnlineSubsystem
- ✅ StateTree
- ✅ GameplayStateTree

**Third-Party Plugins**:
- ✅ Advanced Sessions (included)
- ⚠️ OnlineSubsystemEOS (configure for production)
- ⚠️ OnlineSubsystemSteam (configure for Steam)

To verify plugins:
1. Open `EchoesOfTime.uproject` in text editor
2. Check `Plugins` section
3. Or use Editor: Edit → Plugins → Search for plugin name

#### 4. Generate Project Files

```bash
# Windows
Right-click EchoesOfTime.uproject → "Generate Visual Studio project files"

# Or via command line:
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="EchoesOfTime.uproject" -game -engine

# Linux
Engine/Build/BatchFiles/Linux/GenerateMakefile.sh -project="EchoesOfTime.uproject"

# Mac
Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project="EchoesOfTime.uproject"
```

#### 5. Build the Project

**Option A: From Unreal Editor** (Recommended for first build)
```
1. Double-click EchoesOfTime.uproject
2. Editor will prompt: "Project is out of date, would you like to rebuild?"
3. Click "Yes"
4. Wait for compilation (5-15 minutes on first build)
```

**Option B: From Visual Studio**
```
1. Open EchoesOfTime.sln
2. Set configuration to "Development Editor"
3. Set platform to "Win64"
4. Build → Build Solution (Ctrl+Shift+B)
```

**Option C: Command Line**
```bash
# Windows
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
  EchoesOfTimeEditor Win64 Development ^
  -Project="EchoesOfTime.uproject"

# Linux
Engine/Build/BatchFiles/Linux/Build.sh ^
  EchoesOfTimeEditor Linux Development ^
  -Project="EchoesOfTime.uproject"
```

### Configure Online Subsystem

#### For Local Testing (Default)

Use NULL subsystem (no configuration needed):

```ini
# Config/DefaultEngine.ini
[OnlineSubsystem]
DefaultPlatformService=NULL
```

#### For Steam Integration

1. Install Steam SDK or use Steamworks plugin
2. Configure `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480  ; Use 480 for testing, replace with your App ID
bInitServerOnClient=true
bVACEnabled=0  ; Enable VAC in production

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")
```

3. Restart editor after configuration changes

#### For Epic Online Services (EOS)

1. Create EOS application at [Epic Developer Portal](https://dev.epicgames.com/)
2. Get Product ID, Sandbox ID, Deployment ID
3. Configure `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=YOUR_PRODUCT_ID_HERE
SandboxId=YOUR_SANDBOX_ID_HERE
DeploymentId=YOUR_DEPLOYMENT_ID_HERE
ClientId=YOUR_CLIENT_ID_HERE
ClientSecret=YOUR_CLIENT_SECRET_HERE
```

4. Configure additional EOS settings as per Epic's documentation

### Editor Setup

#### Map Configuration

Ensure these maps exist and are properly configured:

1. **Main Menu Map**: `/Content/Maps/MainMenuMap.umap`
   - **Game Mode**: Default (will use MainMenuPlayerController)
   - **Purpose**: Session creation, settings
   
2. **Lobby Map**: `/Content/Maps/LobbyMap.umap`
   - **Game Mode**: `BP_LobbyGameMode`
   - **Player Controller**: `BP_LobbyPlayerController`
   - **Purpose**: Team selection, readiness, session management

3. **Gameplay Map**: `/Content/Maps/TestMap.umap` (or custom map)
   - **Game Mode**: `BP_DefaultGameMode`
   - **Player Controller**: `BP_DefaultPlayerController`
   - **Game State**: `BP_DefaultGameState`
   - **Player State**: `BP_DefaultPlayerState`
   - **Purpose**: Main gameplay

#### Set Default Maps

In Project Settings → Maps & Modes:
- **Editor Startup Map**: MainMenuMap
- **Game Default Map**: MainMenuMap
- **Server Default Map**: MainMenuMap

#### Configure Game Modes Per Map

```
World Settings (each map):
├─ MainMenuMap → No specific GameMode override
├─ LobbyMap → BP_LobbyGameMode
└─ TestMap → BP_DefaultGameMode
```

### Verify Installation

#### Run Quick Test

1. **Open Editor**: Launch `EchoesOfTime.uproject`
2. **Check Compilation**: Verify no errors in Output Log
3. **Test Main Menu**: Press Play (PIE) in MainMenuMap
4. **Test Multiplayer**:
   - Edit → Editor Preferences → Play
   - Set "Number of Players" to 2
   - Set "Net Mode" to "Play As Listen Server"
   - Press Play
   - Verify both clients spawn

#### Verify Plugins

```
Editor → Edit → Plugins → Search:
✓ Gameplay Abilities (Enabled, Built-in)
✓ Enhanced Input (Enabled, Built-in)
✓ Advanced Sessions (Enabled, Project)
✓ Niagara (Enabled, Built-in)
```

#### Check Console for Errors

```
Window → Developer Tools → Output Log
Look for:
❌ LogAbilitySystem: Error - Fix GAS setup
❌ LogOnline: Error - Fix Online Subsystem
❌ LogNet: Error - Fix networking setup
✅ LogInit: Display: Game Engine Initialized - All good!
```

### Post-Installation Configuration

#### Input Bindings

Enhanced Input is configured in:
- Input Mapping Context: `Content/Input/IMC_Default`
- Input Actions: `Content/Input/Actions/`

Verify bindings in **Edit → Project Settings → Input**

#### Gameplay Tags

Verify tags are loaded:
- **Project Settings → Gameplay Tags**
- Should see tags from `Config/DefaultGameplayTags.ini`
- Tags like `Team.Past`, `Team.Future`, `Ability.LockPick`, etc.

#### Network Settings

For optimal multiplayer:
```ini
# Config/DefaultEngine.ini
[/Script/Engine.GameNetworkManager]
TotalNetBandwidth=32000
MaxDynamicBandwidth=7000
MinDynamicBandwidth=4000

[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate=60
MaxClientRate=25000
MaxInternetClientRate=10000
```

### Optional: Blueprint Implementation

Some features require Blueprint implementation:

#### Game Instance Session Creation

1. Open `BP_DefaultGameInstance` (Blueprint child of `UDefaultGameInstance`)
2. Implement `CreateSession` event (marked as BlueprintImplementableEvent)
3. Use Advanced Sessions plugin nodes:
   - Create Advanced Session
   - Find Sessions
   - Join Session

#### UI Widget Bindings

Verify UI widgets are properly bound in:
- `Content/UI/` - Widget blueprints
- Check event bindings connect to C++ functions

### Troubleshooting Setup

**"Missing DLL" on first launch?**
```
Solution: Rebuild from Visual Studio or let editor rebuild
```

**"Plugin 'GameplayAbilities' failed to load"?**
```
Solution: Verify plugin enabled in .uproject and editor
Regenerate project files
```

**"Cannot open project with current engine version"?**
```
Solution: Update to UE 5.7+, or right-click .uproject → 
"Switch Unreal Engine version"
```

**Compilation errors about missing headers?**
```
Solution: Check all plugins are enabled
Verify engine installation is complete
Clean build (delete Binaries, Intermediate, Saved)
```

For more troubleshooting, see [Troubleshooting](#troubleshooting) section.

---

## Development Workflow

### Building the Project

#### From Unreal Editor
1. Open `EchoesOfTime.uproject` in Unreal Engine 5.7+
2. Editor will prompt to rebuild modules if needed
3. Click "Yes" to compile C++ code

#### From Visual Studio
1. Right-click `EchoesOfTime.uproject` → "Generate Visual Studio project files"
2. Open `EchoesOfTime.sln` in Visual Studio 2022
3. Set configuration to `Development Editor` or `DebugGame Editor`
4. Build Solution (Ctrl+Shift+B)

#### Command Line Build
```bash
# Windows
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" EchoesOfTimeEditor Win64 Development -Project="Path\To\EchoesOfTime.uproject"

# Linux
/path/to/UE_5.7/Engine/Build/BatchFiles/Linux/Build.sh EchoesOfTimeEditor Linux Development -Project="/path/to/EchoesOfTime.uproject"
```

### Running and Testing

#### PIE (Play In Editor)
- **Single Player**: Click "Play" button (Alt+P)
- **Multiplayer Test**: 
  - Set Number of Players: Edit → Editor Preferences → Play → Multiplayer Options
  - Recommended: 2-4 players for testing Past/Future team mechanics
  - Use "New Editor Window (PIE)" for dedicated server testing

#### Standalone Game
- Play → Standalone Game (Alt+F11)
- Use command line arguments for networking:
  ```bash
  # Host session
  EchoesOfTime.exe /Game/Maps/LobbyMap?listen
  
  # Join session
  EchoesOfTime.exe 127.0.0.1
  ```

### Debugging

#### C++ Debugging
1. Set Visual Studio as debugger: Editor Preferences → General → Source Code → Source Code Editor
2. Attach to process: Debug → Attach to Process → UE4Editor.exe
3. Set breakpoints in .cpp files
4. Use `UE_LOG` macros for runtime logging:
   ```cpp
   UE_LOG(LogTemp, Warning, TEXT("Your message: %s"), *YourString);
   ```

#### Blueprint Debugging
- Set breakpoints in Blueprint nodes
- Use Print String nodes for quick debugging
- Enable Blueprint debugging: Debug → Enable Blueprint Debugging

#### Network Debugging
- Console commands:
  ```
  stat net          // Show network statistics
  net pktlag 100    // Simulate 100ms lag
  net pktloss 10    // Simulate 10% packet loss
  ShowDebug NET     // Display network debug info
  ```

### Code Style and Conventions

- **Naming**: Follow [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.7/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
  - Classes: `AMyActor`, `UMyComponent`, `FMyStruct`, `EMyEnum`, `IMyInterface`
  - Variables: `bIsActive`, `PlayerCount`, `CurrentHealth`
- **Headers**: Use forward declarations where possible
- **Replication**: Always use `DOREPLIFETIME` macros and OnRep functions
- **Memory**: Use UE smart pointers (`TSharedPtr`, `TWeakPtr`) for non-UObject types
- **Comments**: Document public APIs and complex logic

### Version Control

- **Branch Strategy**: Feature branches from main
- **Commit Messages**: Use descriptive messages explaining "why" not just "what"
- **Binary Files**: Large assets committed using Git LFS
- **Ignored Files**: Build artifacts, intermediate files, saved folders

---

## Configuration Guide

### Engine Configuration (`DefaultEngine.ini`)

Key sections to configure:

#### Online Subsystem
```ini
[OnlineSubsystem]
DefaultPlatformService=Steam  ; or EOS, NULL for local testing

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480  ; Replace with your Steam App ID
bInitServerOnClient=true

[OnlineSubsystemEOS]
bEnabled=false
ProductId=YourProductId
SandboxId=YourSandboxId
DeploymentId=YourDeploymentId
```

#### Network Settings
```ini
[/Script/Engine.GameNetworkManager]
TotalNetBandwidth=32000
MaxDynamicBandwidth=7000
MinDynamicBandwidth=4000

[/Script/OnlineSubsystemUtils.IpNetDriver]
MaxClientRate=25000
MaxInternetClientRate=10000
```

### Game Configuration (`DefaultGame.ini`)

Project settings and gameplay tags are defined here:

```ini
[/Script/EngineSettings.GeneralProjectSettings]
ProjectID=YourProjectGUID
ProjectName=Echoes of Time
CompanyName=YourCompany

[/Script/GameplayTags.GameplayTagsSettings]
ImportTagsFromConfig=True
```

### Input Configuration (`DefaultInput.ini`)

Enhanced Input System mappings are defined here. Key input contexts:
- `IMC_Default`: Main gameplay input context
- `IMC_UI`: UI navigation context

### Gameplay Tags Configuration (`DefaultGameplayTags.ini`)

All gameplay tags are defined here. See [Gameplay Tags](#gameplay-tags) section for details on tag usage.

### Editor Preferences (`DefaultEditor.ini`)

Editor-specific settings including:
- Asset editor layouts
- Content browser settings
- Blueprint editor preferences

---

## Gameplay Overview

### Game Modes

#### Main Menu
- **Map**: `MainMenuMap`
- **Controller**: `AMainMenuPlayerController`
- **Purpose**: Session creation/joining, settings configuration

#### Lobby
- **Map**: `LobbyMap`
- **Game Mode**: `ALobbyGameMode`
- **Controller**: `ALobbyPlayerController`
- **Features**:
  - Player seating and visualization
  - Team selection (Past/Future)
  - Readiness system
  - Host controls (start game, kick players)
  - Friend list integration

#### Gameplay Match
- **Map**: `TestMap` (configurable)
- **Game Mode**: `ADefaultGameMode`
- **Controller**: `ADefaultPlayerController`
- **Game State**: `ADefaultGameState`
- **Player State**: `ADefaultPlayerState`

### Core Gameplay Loop

1. **Lobby Phase**
   - Players join session and select seats
   - Choose team (Past or Future)
   - Mark ready when team is formed
   - Host starts match when requirements met

2. **Match Initialization**
   - Seamless travel to gameplay map
   - Players spawn at team-specific spawn points
   - Procedural level generator populates puzzles
   - Objectives and timers initialized

3. **Gameplay Phase**
   - **Past Team**: Collects items, solves puzzles, creates timeline
   - **Future Team**: Sees echoes of Past actions, adapts strategy
   - **Both Teams**: Avoid detection, coordinate across timelines

4. **Objective Completion**
   - Collect target money amount
   - Complete required puzzles
   - Escape before alarm countdown expires

5. **Match End**
   - Victory/defeat conditions evaluated
   - Session cleanup and return to lobby

### Player Abilities

| Ability | Team | Description | Input |
|---------|------|-------------|-------|
| **Lockpicking** | Both | Mini-game to unlock doors | E on locked door |
| **Hacking** | Both | Timed progress to access computers | E on computer |
| **Searching** | Both | Timed search of containers | E on searchable object |
| **Aim/Fire** | Both | Weapon combat | Right Mouse / Left Mouse |
| **Past Echo** | Future | Toggle visibility of Past team echoes | G (configurable) |
| **Sprint** | Both | Increased movement speed | Shift |
| **Crouch** | Both | Reduced detection profile | Ctrl |

### Puzzle Types

1. **Lever Puzzles**: Activate levers in correct sequence
2. **Wire Puzzles**: Cut specific colored wires in order across multiple devices
3. **Keypad Codes**: Enter correct numeric codes from hacked computers
4. **Fingerprint Matching**: Use fingerprints from cups to access code generators
5. **Device Disabling**: Search and disable multiple security devices

### Detection and Alarm System

#### Detection Levels
1. **Undetected**: Green/no indicator
2. **Suspicious**: Yellow, detection ring filling
3. **Pre-Alarm**: Orange, countdown starts
4. **Full Alarm**: Red, level restart countdown

#### Detection Sources
- **Security Cameras**: Pan in fixed patterns, FOV-based detection
- **Guards**: AI perception with sight sense, investigate suspicious activity
- **Laser Sensors**: Instant alarm on contact
- **Metal Detectors**: Alarm when carrying metal items

---

## Troubleshooting

### Common Issues

#### Build Errors

**Problem**: "Missing modules" error when opening project
```
Solution:
1. Right-click .uproject → "Generate Visual Studio project files"
2. Build in Visual Studio
3. Reopen project in editor
```

**Problem**: "Failed to load DLL" errors
```
Solution:
- Check that all required plugins are enabled
- Verify engine version (5.7+)
- Clean build: Delete Binaries, Intermediate, Saved folders
- Regenerate project files
```

#### Runtime Issues

**Problem**: Multiplayer desync or replication issues
```
Solution:
- Verify all gameplay state is replicated
- Check OnRep functions are called
- Use `net.SimulateLatency X` to test with lag
- Review server logs for RPC failures
```

**Problem**: Abilities not activating
```
Solution:
- Verify GameplayAbilitySystemComponent is initialized
- Check ability tags match input tags
- Ensure ASC is on PlayerState (not Character) for seamless travel
- Debug with `showdebug abilitysystem` console command
```

**Problem**: Items disappearing or not spawning
```
Solution:
- Verify ItemInstanceID uniqueness
- Check Past/Future team assignment
- Review `IsNetRelevantFor` implementation
- Ensure proper replication settings on pickup actors
```

#### Performance Issues

**Problem**: Low FPS in multiplayer
```
Solution:
- Reduce number of replicated actors
- Use relevancy optimization (IsNetRelevantFor)
- Enable network culling
- Profile with `stat fps`, `stat unit`, `stat game`
```

#### Network/Session Issues

**Problem**: Cannot create or join sessions
```
Solution:
- Verify OnlineSubsystem configuration
- Check firewall settings (ports 7777, 7778 for testing)
- Test with NULL subsystem first
- Review logs: Window → Developer Tools → Output Log
```

### Debug Console Commands

```cpp
// Networking
stat net                    // Network statistics
net.PackageMap.DebugObject  // Debug specific actor replication
net.Relevancy               // Show relevancy debugging

// Performance
stat fps                    // Frame rate
stat unit                   // Frame time breakdown
stat game                   // Game thread stats
stat scenerendering         // Rendering stats

// Gameplay
showdebug abilitysystem     // GAS debugging
showdebug ai                // AI debugging
displayall ADefaultCharacter bIsSprinting  // Show variable on all instances

// Logging
log LogNet All              // Enable all network logging
log LogAbilitySystem Verbose // Detailed GAS logs
log LogTemp Warning         // Default logging category
```

### Logs Location

- **Windows**: `%LOCALAPPDATA%\EchoesOfTime\Saved\Logs`
- **Linux**: `~/.config/EchoesOfTime/Saved/Logs`
- **Log File**: `EchoesOfTime.log`

---

## Performance Optimization

### Network Optimization

#### Replication Strategy
- Use `IsNetRelevantFor()` to limit replication scope (Future items only to Future team)
- Set appropriate `NetUpdateFrequency` on actors (default 100, reduce for less critical actors)
- Use conditional replication with `DOREPLIFETIME_CONDITION`
- Batch RPCs where possible to reduce overhead

#### Bandwidth Management
```cpp
// Example: Conditional replication
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(AMyActor, LowPriorityData, COND_SkipOwner);
    DOREPLIFETIME_CONDITION(AMyActor, OwnerOnlyData, COND_OwnerOnly);
}
```

### Rendering Optimization

#### LOD (Level of Detail)
- Use LOD groups for skeletal meshes
- Set appropriate screen sizes for LOD transitions
- Enable HLOD for static geometry in large levels

#### Culling
- Use occlusion culling for indoor environments
- Set appropriate cull distances on actors
- Enable distance culling for effects (Niagara systems)

### Memory Optimization

#### Asset Management
- Use asset streaming for large levels
- Implement soft object references for optional content
- Unload unused assets with garbage collection hints

#### Actor Pooling
Consider pooling for frequently spawned/destroyed actors:
- Pickups
- Projectiles (bullets)
- Effect actors

### CPU Optimization

#### Tick Management
- Disable tick on actors that don't need it
- Use timers instead of tick for infrequent updates
- Set appropriate tick intervals with `PrimaryActorTick.TickInterval`

#### Profiling Tools
- Unreal Insights: In-depth profiling system
- GPU Visualizer: Rendering performance analysis  
- Network Profiler: Replication analysis
- CPU Profiler: Game thread analysis

---

## FAQ

### General Questions

**Q: What version of Unreal Engine is required?**  
A: Unreal Engine 5.7 or higher. The project uses features introduced in UE5.

**Q: Does this support cross-platform multiplayer?**  
A: The architecture supports it, but requires proper Online Subsystem configuration (EOS recommended for cross-platform).

**Q: How many players can play simultaneously?**  
A: Designed for 2-8 players, with optimal experience at 4 players (2 Past, 2 Future).

### Technical Questions

**Q: Why use PlayerState for AbilitySystemComponent?**  
A: PlayerState persists across seamless travel, maintaining abilities and attributes between level transitions (Lobby → Match).

**Q: How does time invalidation work?**  
A: Each item has a unique `ItemInstanceID` (GUID). When Past item is destroyed, it broadcasts invalidation. Future items and inventories subscribe to this and remove/destroy matching instances.

**Q: Can I add new team types beyond Past/Future?**  
A: Yes, extend the team system by:
1. Add new gameplay tag (e.g., `Team.Present`)
2. Update spawn logic in ProceduralLevelGenerator
3. Modify relevancy checks in time-specific actors
4. Update lobby UI for team selection

**Q: How do I add a new ability?**  
A: See [Extending & Contribution → New Abilities](#extending--contribution).

### Gameplay Questions

**Q: What happens if Past team gets detected?**  
A: Pre-alarm starts with countdown. If not canceled, full alarm triggers and level restarts after timer expires.

**Q: Can Future team affect Past timeline?**  
A: No, the causality is one-way: Past → Future. This prevents temporal paradoxes and maintains consistent gameplay.

**Q: How do puzzles reset?**  
A: Most puzzles auto-reset on incorrect input (wires, levers). Some require full alarm reset.

### Development Questions

**Q: How do I test multiplayer locally?**  
A: Use PIE with "Number of Players" > 1, or run multiple standalone instances with `-log` flag.

**Q: Where should I put custom content?**  
A: 
- C++ code: `/Source/EchoesOfTime/`
- Blueprints: `/Content/Blueprints/`
- Assets: `/Content/` with organized subdirectories

**Q: How do I contribute?**  
A: See [Extending & Contribution](#extending--contribution) section below.

---

## Extending & Contribution

- New Items:
  - Create `UItemBase`/`UItemBaseWithAbilities` DataAssets; assign mesh/icon/effects/abilities.
  - If time-aware, ensure past and future pickups are spawned with shared `ItemInstanceID`.
- New Abilities:
  - Inherit from `UGameplayAbility`; define tags and optionally an AbilityTask UI.
  - Tag-map the input in `UAbilityInputSet` and assign to characters.
- New Time-Linked Objects:
  - Model Past and Future variants where Past broadcasts state to Future.
  - Use `IsNetRelevantFor` to restrict Future-only actors to Future team.
- New Detection Sources:
  - Implement `ICameraDetectable` for new sensors; call into `ADefaultGameState` to trigger Pre-Alarm/Alarm as desired.
- Lobby/Online:
  - Customize `ALobbyPlatformActor` look/placement; extend `UPlayerLobbyInfo`.
  - Adjust `ALobbyGameMode` rules for readiness/auto-start.
- UI:
  - Extend `UCharacterOverlay` for additional status or effect bars.
  - Wire computers/desks to display codes and hints via UMG instead of on-screen debug.

### Contributing Guidelines

We welcome contributions! Please follow these guidelines:

#### Code Contributions

1. **Fork and Branch**
   - Fork the repository
   - Create a feature branch: `git checkout -b feature/your-feature-name`
   - Make your changes following the code style

2. **Commit Standards**
   - Write clear, descriptive commit messages
   - Explain the "why" behind changes
   - Reference issues if applicable: `Fixes #123`

3. **Testing**
   - Test your changes in both single-player and multiplayer
   - Verify no replication issues
   - Check performance impact

4. **Pull Request**
   - Update documentation if needed
   - Describe what your PR does and why
   - Link related issues
   - Ensure CI passes (if configured)

#### Code Review Process

- PRs require review from maintainers
- Address feedback promptly
- Keep PRs focused on single features/fixes
- Maintain backward compatibility where possible

#### Reporting Issues

When reporting bugs, include:
- Unreal Engine version
- Steps to reproduce
- Expected vs actual behavior
- Relevant logs or screenshots
- System specifications

#### Feature Requests

- Describe the feature clearly
- Explain use cases and benefits
- Consider implementation complexity
- Discuss on issues before major work

Please follow code style and PR guidelines mentioned above. This project aims for clean server-authoritative gameplay with thoughtful client prediction and robust UI patterns.

---

## Credits & Acknowledgments

### Core Technologies

- **[Unreal Engine 5](https://www.unrealengine.com/)** - Epic Games
- **[Gameplay Ability System (GAS)](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)** - Epic Games
- **[Advanced Sessions Plugin](https://github.com/mordentral/AdvancedSessionsPlugin)** - mordentral
- **[Enhanced Input System](https://docs.unrealengine.com/5.7/en-US/enhanced-input-in-unreal-engine/)** - Epic Games

### Development Team

See the [Contributors](https://github.com/Traveler3114/Echoes-of-Time/graphs/contributors) page for a full list of project contributors.

### Special Thanks

- Unreal Engine community for extensive documentation and examples
- Online subsystem developers for multiplayer infrastructure
- GAS community for ability system patterns and best practices

### Third-Party Assets

List any third-party assets, plugins, or libraries used in the project with proper attribution.

---

## License

This project is licensed under the terms specified in the [LICENSE](LICENSE) file.

**Summary**: [Specify your license type here - e.g., MIT, Apache 2.0, GPL, or Proprietary]

### Additional Terms

- This project uses Unreal Engine 5, which is subject to Epic Games' license agreement
- Some plugins may have their own licenses - check individual plugin documentation
- Ensure compliance with all applicable licenses when contributing or using this code

For full license details, see the [LICENSE](LICENSE) file in the repository root.

---

## Additional Resources

### Documentation

- [Unreal Engine Documentation](https://docs.unrealengine.com/5.7/en-US/)
- [Gameplay Ability System Documentation](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Networking Overview](https://docs.unrealengine.com/5.7/en-US/networking-overview-for-unreal-engine/)
- [Multiplayer Programming](https://docs.unrealengine.com/5.7/en-US/multiplayer-programming-quick-start-for-unreal-engine/)

### Community

- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [Unreal Slackers Discord](https://unrealslackers.org/)
- [r/unrealengine](https://www.reddit.com/r/unrealengine/)

### Tutorials

- [GAS Documentation by Tranek](https://github.com/tranek/GASDocumentation)
- [Multiplayer in UE5 by CodeLikeMe](https://www.youtube.com/c/CodeLikeMe)

---

## Project Status

**Current Version**: Development  
**Status**: Active Development  
**Last Updated**: 2025-12-06

### Roadmap

Planned features and improvements:
- [ ] Enhanced AI behavior patterns
- [ ] Additional puzzle types
- [ ] More time-travel mechanics
- [ ] Expanded level generation
- [ ] Performance optimizations
- [ ] UI/UX improvements
- [ ] Additional game modes

For detailed progress and milestones, see [Issues](https://github.com/Traveler3114/Echoes-of-Time/issues) and [Projects](https://github.com/Traveler3114/Echoes-of-Time/projects).

---

## Contact & Support

For questions, issues, or contributions:

- **GitHub Issues**: [Report bugs or request features](https://github.com/Traveler3114/Echoes-of-Time/issues)
- **Discussions**: [Join community discussions](https://github.com/Traveler3114/Echoes-of-Time/discussions)
- **Email**: [Specify contact email if desired]

---

**[⬆ Back to Top](#echoes-of-time)**
