# Gameplay Systems

This document provides detailed documentation of all core gameplay systems in **Splitstream**.

---

## 1. Timeline System

### `ETimelineEra` (TimelineEra.h)

The game world exists in two parallel eras:

```cpp
UENUM(BlueprintType)
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};
```

- **Past Team** — Players assigned to the Past era operate in the historical version of the map.
- **Future Team** — Players assigned to the Future era operate in the futuristic version.
- **Solo** — Single-player mode where one player gets abilities from both teams.

Team assignment is stored in `ADefaultPlayerState::TeamName` and synchronized via gameplay tags (`Team.Past`, `Team.Future`, `Team.Solo`) on the player's `AbilitySystemComponent`.

### Timeline-Specific Actors

Many actors have Past and Future variants under `Actors/TimeObjects/`:

| Base Class | Past Variant | Future Variant |
|---|---|---|
| `ADoorBase` | `APastDoor` | `AFutureDoor` |
| `ADoubleDoorBase` | `APastDoubleDoor` | `AFutureDoubleDoor` |
| `AVentBase` | `APastVent` | `AFutureVent` |
| `AItemPickup` | `APastItemPickup` | `AFutureItemPickup` |
| `APowerGenerator` | `APastPowerGenerator` | `AFuturePowerGenerator` |

### Ghost Mirroring System

Guards visible in one timeline produce **ghost** representations in the other timeline, giving players cross-timeline situational awareness.

**Key classes:**
- `IGhostMirrorSource` — Interface implemented by `AGuardCharacter`; provides `ShouldGhostBeVisible()` and `GetMirrorMesh()`.
- `IGhostRevealable` — Interface implemented by `AGhostCharacterActor`; provides `SetGhostRevealed()`.
- `AGhostCharacterActor` — The ghost actor spawned per guard. Mirrors the source guard's skeletal mesh pose each tick with a configurable offset (`GhostOffset`). The offset is computed from `ARefPointActor::GetOffsetBetweenFirstTwoRefPoints()` to correctly position ghosts across timeline boundaries.
- `UFutureGAPastEcho` — A toggle ability that activates/deactivates ghost visibility via Gameplay Cues (`GameplayCue.PastEcho.Activated` / `GameplayCue.PastEcho.Deactivated`).

---

## 2. Detection System

### `UDetectionComponent` (ActorComponents/DetectionComponent.h)

A reusable component that implements a **progressive detection meter**. When an AI guard or camera detects a player, the detection progress fills over time. If it reaches 100%, the actor is "fully detected."

**Key properties:**
- `DetectionDuration` (default `2.5s`) — Time to fill from 0% to 100%.
- `BaseDetectionSpeedMultiplier` (default `1200.f`) — Distance-based speed modifier. Closer targets fill faster.
- `DetectionStates` — `TMap<AActor*, FDetectionState>` tracking per-detector progress.

**Key methods:**
- `StartDetection(AActor* Detector)` — Begins filling the meter for the given detector.
- `StopDetection(AActor* Detector)` — Begins draining the meter.
- `ForceImmediateDetectionEnd(AActor* Detector)` — Instantly resets detection (e.g., when guard dies).
- `HandleFullyDetected(AActor* Detector)` — Called when progress hits 100%; broadcasts `IDetectable::OnFullyDetected`.

**Integration flow:**
1. AI guard's `UAIPerceptionComponent` detects a player.
2. `AAICharacter::OnPerceptionUpdated()` calls `IDetectable::Execute_OnDetected(Player, Guard)`.
3. `ADefaultCharacter::OnDetected_Implementation()` checks if the player has `TAG_Character_Status_Illegal`.
4. If illegal, `DetectionComponent->StartDetection(Guard)` begins filling the meter.
5. `ADefaultPlayerController::ClientUpdateDetectionWidget()` renders the progress on screen.
6. On full detection, the guard triggers `StartPreAlarm()` on the `ADefaultGameState`.

### `IDetectable` Interface

```cpp
void OnDetected(AActor* Detector);         // Detection begins
void OnLost(AActor* Detector);             // Detection lost (target left sight)
void OnFullyDetected(AActor* DetectingActor); // 100% detected
void OnForceDetectionEnd(AActor* Detector);   // Immediate reset
bool IsActorAlreadyDetected(AActor* DetectingActor) const;
```

### Illegal Status Tag

The `Character.Status.Illegal` gameplay tag is the key trigger for detection. It is applied by:
- `AIllegalArea` — Box trigger volumes that apply a gameplay effect granting the illegal tag when players enter.
- `UItemBase::bAlertGuardsWhenSeen` — Items that apply the illegal tag when held.
- Direct gameplay effect application.

When the illegal tag changes, `ADefaultCharacter::OnIllegalTagChanged()` iterates all actors implementing `IDetectable` and starts/stops detection accordingly.

---

## 3. Alarm System

### `ADefaultGameState` — Alarm State Machine

The alarm system has two phases:

#### Pre-Alarm
- Triggered by `StartPreAlarm(AActor*, float Duration, ETimelineEra)`.
- Each era has independent pre-alarm state (`FPerEraPreAlarmState`).
- Multiple instigators can contribute; the soonest ETA is used.
- If the instigator is eliminated (guard killed), `RemovePreAlarmInstigator()` is called.
- If all instigators are removed, the pre-alarm cancels.
- Displayed as "Guard spotted you! Alarm in X s" on the HUD (yellow text).

#### Full Alarm
- Triggered by `StartAlarm(AActor*, ETimelineEra)` when pre-alarm timer expires.
- `AlarmDuration` (default `5s`) countdown begins.
- Displayed as "ALARM - Restart in X s" on the HUD (red text).
- When the alarm timer expires, `ADefaultGameMode::EliminatePlayersInEra()` destroys all player pawns in that era and switches them to spectator mode.

**Replicated state:**
- `AlarmEndTime`, `bAlarmActive`, `AlarmInstigator`, `AlarmEra` — Replicated via `DOREPLIFETIME`.
- `PastPreAlarm`, `FuturePreAlarm` — Per-era pre-alarm states with `ReplicatedUsing`.

---

## 4. Interaction System

### `IInteractable` Interface

All interactable objects implement this interface:

```cpp
void Interact(AActor* Interactor);           // Primary interaction
void CancelInteract(AActor* Interactor);     // Cancel progressive interaction
void SetHighlighted(bool bHighlight);        // Visual feedback (custom depth)
bool IsProgressiveInteract();                // true = hold-to-interact
bool IsCorrectItem(const FInventorySlot& Slot) const; // Item validation
bool RequiresItem() const;                   // Does this require a specific item?
```

### Interaction Flow (DefaultCharacter)

1. **Highlight** — Every tick, `UpdateInteractHighlight()` performs a forward line trace. If it hits an `IInteractable`, it calls `SetHighlighted(true)`.
2. **Instant Interact** — On `InteractAction::Started`, `HandleInteractInstant()` performs a trace and calls `ServerHandleInteract()` via RPC.
3. **Progressive Interact** — On `InteractAction::Triggered`, `HandleInteractHoldStart()` calls `Interact()` on the target if `IsProgressiveInteract()` returns true. On `Completed`, `HandleInteractHoldStop()` calls `CancelInteract()`.
4. **Server Validation** — `ServerHandleInteract_Implementation()` validates the item requirement server-side before executing.

---

## 5. Inventory System

### `UInventoryComponent` (ActorComponents/InventoryComponent.h)

A replicated actor component managing a fixed-size inventory (default 10 slots).

**Key types:**
```cpp
struct FInventorySlot
{
    UItemBase* ItemAsset;          // Pointer to DataAsset
    FGuid ItemInstanceID;          // Unique instance identifier
    ACivilianCharacter* OwnerCivilian; // Optional civilian owner reference
    TArray<FActiveGameplayEffectHandle> GrantedGameplayEffectHandles;
    TArray<FGameplayAbilitySpecHandle>  GrantedAbilityHandles;
};
```

**Key methods:**
- `AddItem(UItemBase*, FGuid)` — Adds an item to the first empty slot.
- `RemoveItem(int32 Index)` — Removes item at index.
- `SetActiveSlot(int32 Index)` — Changes equipped slot.
- `DropActiveItem(FVector)` — Drops the currently equipped item.
- `ServerSetActiveSlot()` / `ServerDropActiveItem()` / `ServerAddItem()` — Server RPCs.

**Item lifecycle:**
1. `AddItem()` → `UItemBase::OnAddedToInventory()` → grants passive effects/abilities.
2. `SetActiveSlot()` → `UItemBase::OnEquipped()` / `OnUnequipped()`.
3. `RemoveItem()` → `UItemBase::OnRemovedFromInventory()` → removes granted effects/abilities.
4. `DropActiveItem()` → `UItemBase::OnDropped()` → spawns pickup actor in world.

### `UItemBase` Data Asset

Data-driven item definitions with:
- `ItemName`, `ItemDescription`, `ItemIcon`, `ItemType` (enum: Keycard, Fingerprint, MoneyBag, Pistol, PowerCell, etc.)
- `Value` — Money value when collected.
- `ItemPickupToSpawn` / `PastItemPickupToSpawn` / `FutureItemPickupToSpawn` — Actor classes spawned on drop (era-specific).
- `bRemoveFromInventoryOnUse`, `bEnablePhysicsOnDrop`, `bAlertGuardsWhenSeen`.
- `GrantedGameplayEffects` — Effects applied passively while in inventory.
- `AbilitySet` — Abilities granted while item is equipped.

---

## 6. Puzzle Systems

### Wire Puzzle (`Wire/`)
- `AWireActor` — Individual wire with a color.
- `AWireDeviceActor` — A device containing multiple wires to cut.
- `AWirePuzzleManager` — Manages the correct wire-cutting sequence across devices.

### Lever Puzzle (`Lever/`)
- `ALeverActor` — A lever that can be pulled.
- `ALeverManager` — Validates the correct lever order and fires `IPuzzleCompletionReceiver::OnPuzzleCompleted()`.

### Keypad Puzzle (`KeypadScanner/`)
- `AKeypadButton` — Individual keypad button.
- `AKeypadScanner` — Full keypad with code validation. Codes are randomly generated by `AProceduralLevelGenerator`.

### Laser Grid (`Laser/`)
- `ALaserSensor` — Individual laser beam sensor.
- `ALaserManager` — Controls the laser grid pattern.

### Lock Picking (`ULockPickComponent`)
- Pin-based lock picking with difficulty levels (Easy/Medium/Hard).
- Each pin has a `SweetSpotAngle` and `Tolerance`.
- Integrated with GAS ability `UDefaultGALockPick` and `ULockPickAbilityTask`.

### Hacking (`UHackComponent`)
- Timed hacking with progress bar.
- Integrated with GAS ability `UDefaultGAHack` and `UHackAbilityTask`.

### Terminal Mini-Games (`Minigames/`)
- `UFirewallMiniGame` — Space-invader-style arcade mini-game with waves, bosses, and difficulty scaling.
- `UNeonRunnerMiniGame` — Endless-runner with obstacles, jump, and hover abilities.
- Both are `UObject`-based, create their own input contexts, and delegate rendering to custom UMG widgets.

---

## 7. AI System

### Character Hierarchy
```
ACharacter
├── ADefaultCharacter      (Player)
└── AAICharacter            (Abstract AI Base)
    ├── AGuardCharacter     (Security guard)
    │   └── ARobotGuardCharacter (Repair robot)
    └── ACivilianCharacter  (Civilian NPC)

APawn
└── ADronePawn              (Security drone)
```

### AI Perception
Guards use `UAIPerceptionComponent` with `UAISenseConfig_Sight`:
- Sight radius: 1500 units (lose sight at 1600).
- Peripheral vision: 90°.
- Detects enemies, neutrals, and friendlies.

### AI Navigation
- `ANavNode` — Custom navigation nodes with neighbor connections, era assignment, and stay-point markers.
- Guards use `NavNode` graphs for patrol routing. `GetRandomNextNode()` selects the next patrol point.
- `UNavUtilityLibrary::EstimateTravelTimeBetweenActors()` provides ETA calculations for repair countdowns.

### StateTree Integration
AI behavior is driven by StateTree assets on the controller's `UStateTreeComponent`. Key events:
- `TAG_StateTree_Event_FullyDetected_Pawn` — Player fully detected, switch to chase.
- `TAG_StateTree_Event_FullyDetected_Actor` — Suspicious actor detected.
- `TAG_StateTree_Event_RepairNeeded` — Robot guard dispatched to repair.

### Drone Detection
`ADronePawn` uses a custom cone-based detection system (not `UAIPerceptionComponent`):
- Periodically scans for `ADefaultCharacter` actors with `TAG_Character_Status_Illegal`.
- Tests bounds points against a view cone + line-of-sight checks.
- On detection, triggers full alarm directly via `ADefaultGameState::StartAlarm()`.

---

## 8. Objective/Money System

- `ADefaultGameState::TargetMoneyAmount` — Target money to collect (default 50,000).
- `ADefaultGameState::CurrentMoneyCollected` — Current progress (replicated).
- `AddCollectedMoney(int32 Amount)` — Called when items are collected or guards are eliminated.
- `OnMoneyCollectedChanged` delegate updates the HUD objective text.
- Guard kills subtract money via `AAICharacter::MoneyToSubtract` (default -10,000).

---

## 9. Repair System

### `IRepairable` Interface
```cpp
void RequestRepair(AActor* RepairInstigator);
float GetRepairTime() const;
ETimelineEra GetTimelineEra() const;
AActor* GetCompletionTarget() const;
FOnRepairRequested& GetOnRepairRequested();
```

Implemented by `ADronePawn` and `ATerminal`.

### `ARobotGuardCharacter`
- On `BeginPlay`, registers with all `IRepairable` actors in its `TimelineEra`.
- Maintains a `RepairQueue` of pending repairs.
- `OnRepairRequested()` → queues repair → `TryStartNextRepair()` → sends `TAG_StateTree_Event_RepairNeeded` to StateTree.
- After repair completes, `OnRepairFinished()` → removes `TAG_Guard_Status_Repair` tag → processes next in queue.

---

## 10. Procedural Level Generation

`AProceduralLevelGenerator` handles randomized level setup on `BeginPlay()` (server-only):

- **Civilian spawning** — Spawns civilians at `ACivilianSpawnPoint` locations with randomized names.
- **Desk assignment** — Assigns civilians to `ADeskActor` instances.
- **Guard-locker assignment** — Pairs guards with lockers.
- **Puzzle setup** — Generates wire sequences, lever orders, keypad codes, and disabling device placement.
- **Newspaper spawning** — Places newspapers with randomly generated dates.
- **Security document spawning** — Generates security documents with guard information.

Replicated data includes `PastDate`, `FutureDate`, `PastLeverOrderString`, and `PastWireDeviceSequence`.
