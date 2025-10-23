# Echoes of Time

A co-op multiplayer Unreal Engine 5 project featuring time-travel mechanics, team-based gameplay, inventory systems, lockpicking/hacking/searching via Gameplay Ability System (GAS), stealth/detection, and a robust lobby/online session UX. The codebase emphasizes clean separation of Gameplay, UI, and Networking, authoritative server logic with client-side prediction for responsive input, and extensibility for new mechanics and content.

This README documents how the systems are implemented in detail, with pointers to the relevant classes and flows. It goes well beyond a feature list to explain how and why things work.

---

## Table of Contents

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
- [Level Gadgets & World Actors](#level-gadgets--world-actors)
  - [Laser Sensors/Groups](#laser-sensorsgroups)
  - [Keypads & Code Generator](#keypads--code-generator)
  - [Desks, Computers, Newspapers, Nav Nodes, Spawn Points](#desks-computers-newspapers-nav-nodes-spawn-points)
- [Project Structure](#project-structure)
- [Setup Instructions](#setup-instructions)
- [Extending & Contribution](#extending--contribution)
- [License](#license)

---

## High-Level Architecture

- Server-authoritative multiplayer with client QoL:
  - Client-side prediction for some abilities (e.g., Aim/Fire, Hacking, Lockpick, Search) using GAS and scoped prediction windows.
  - UI-only elements render client-side but are driven by replicated state or predicted tasks.
- Core systems are isolated:
  - Gameplay/Actors (Doors, Items, Scanners, Cameras, Guards, etc.)
  - Components (Inventory, LockPick, Hack, Search)
  - AbilitySystem (Abilities, AbilityTasks, Attribute sets, Gameplay tags)
  - UI Widgets (HUD, Lobby, Minigames)
  - GameMode/GameState/PlayerState for flow and replicated meta.

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

### Desks, Computers, Newspapers, Nav Nodes, Spawn Points

- `ADeskActor` replicates `StaffName` and owns references to desk computer and spawn points for searchable items.
- `AComputer` integrates `UHackComponent` and replicates `StoredCode` (reveals via on-screen debug on hack complete; wire to UI as needed).
- `ANewspaperActor` replicates a `NewspaperDateString` and updates its `UTextRenderComponent` via `OnRep`.
- `ANavNode` defines patrol graph edges for guards with helpers to pick next node.
- `ACivilianSpawnPoint`, `ARandomPointActor` hold simple timeline/era metadata for procedural systems.

---

## Project Structure

```
/Source
  /AbilitySystem
    - EOTGameplayTags.*             // Tag declarations/definitions
    - AttributeSets/PlayerAttributeSet.*  // Health attribute
    - AbilityTasks/ (LockPick/Hack/Search tasks)
    - Abilities/
      - DefaultGALockPick.*, DefaultGAHack.*, DefaultGASearch.*
      - FutureGAPastEcho.*
      - Weapon/
        - PistolGAAim.*, PistolGAFire.*
    - DefaultAbilitySystemComponent.*
  /Actors
    - ItemPickup.*, CodeGenerator.*, Computer.*, KeycardScanner(s).*
    - DoorBase.*, DoubleDoorBase.* + TimeObjects (Past/Future variants)
    - SecurityCamera.*, GhostCharacterActor.*, GuardCharacter.*
    - Keypad/
      - KeypadButton.*, KeypadScanner.*
    - TimeObjects/
      - PastDoor.*, FutureDoor.*, PastDoubleDoor.*, FutureDoubleDoor.*
      - PastItemPickup.*, FutureItemPickup.*
    - PointActors/
      - NavNode.*, RandomPointActor.*, RefPointActor.*
      - SearchableItemSpawnPoint.*
    - SearchableActor.*, CupActor.*
    - Laser/
      - LaserSensor.*, LaserGroup.*
  /Components
    - InventoryComponent.*, LockPickComponent.*, HackComponent.*, SearchComponent.*
  /Controllers
    - DefaultPlayerController.*, LobbyPlayerController.*, MainMenuPlayerController.*
  /GameModes
    - DefaultGameMode.*, LobbyGameMode.*
  /GameStates
    - DefaultGameState.*, LobbyGameState.*
  /Interfaces
    - IInteractable.*, IRequiresItem.*, IKeycardUnlockable.*, ICameraDetectable.*, IGhostMirrorSource.*, IGhostRevealable.*
  /Widgets
    - HUD/ (CharacterHUD/Overlay, LockPickWidget, HackWidget, SearchWidget, DetectionWidget)
    - Lobby/ (LobbyUI, PlayerLobbyInfo, OpenFriendsListButton, FriendList, FriendWidget)
    - MainMenu/MainMenuWidget.*
    - Calendar/ (CalendarWidget, CalendarButtonWidget, CalendarResultWidget)
  /DataAssets
    - Items/ (ItemBase, FingerprintItem, ItemBaseWithAbilities)
    - AbilitySets/ (AbilityInputSet, DefaultGASet)
  /Misc
    - InputMappingSet.*
    - TimelineEra enum (header elsewhere)
```

---

## Setup Instructions

1. Requirements
   - Unreal Engine 5.x
   - Visual Studio 2022 (or equivalent toolchain)
   - Plugins:
     - GameplayAbilities (GAS)
     - Online Subsystem (for sessions) and Advanced Sessions/Friends if using `UAdvancedFriendsGameInstance`
     - Niagara (for laser effects)

2. Clone and Build
   - `git clone <repo-url>`
   - Open `EchoesOfTime.uproject` in Unreal Editor and build.

3. Configure Plugins/Subsystems
   - Enable GameplayAbilities, Niagara, OnlineSubsystem (Steam/EOS, etc.).
   - Configure OSS in `DefaultEngine.ini` if using sessions.

4. Editor Setup
   - Maps:
     - `/Game/Maps/MainMenuMap` for main menu.
     - `/Game/Maps/LobbyMap` for lobby.
     - `/Game/Maps/TestMap` for gameplay (configurable: `ALobbyGameMode::MatchMapPath`).
   - Set DefaultGameMode per map (Lobby vs Match).
   - Player Controllers:
     - `AMainMenuPlayerController` on Main Menu.
     - `ALobbyPlayerController` on Lobby.
     - `ADefaultPlayerController` on gameplay.

5. Online Session Wiring (optional)
   - Implement `UDefaultGameInstance::CreateSession` in Blueprint to create/join sessions.
   - `UMainMenuWidget` calls `CreateSession()` on button click.

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

Please follow code style and PR guidelines (see CONTRIBUTING.md if provided). This project aims for clean server-authoritative gameplay with thoughtful client prediction and robust UI patterns.

---

## License

Specify your license here (e.g., MIT, Apache 2.0) or link to a LICENSE file.
