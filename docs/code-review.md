# Code Review (Updated — Round 3)

This document provides a comprehensive code review of the **Splitstream** codebase under `Source/Splitstream/`, reflecting all fixes applied across review rounds.

---

## Summary

Splitstream is a well-architected, feature-rich Unreal Engine 5 multiplayer heist game. The project integrates the Gameplay Ability System (GAS), a server-authoritative multiplayer architecture, custom AI systems with StateTree, and multiple interactive puzzle mechanics. Significant improvements have been made since the initial review, demonstrating strong iterative code quality improvement.

---

## All Issues Fixed Across Reviews ✅

### ~~TActorIterator in Tag Change Handler~~ → **Fixed** ✅
`ADefaultCharacter::OnIllegalTagChanged()` no longer iterates all world actors. A `UDetectorRegistry` world subsystem tracks detector actors via `Register()`/`Unregister()` in `BeginPlay()`/`EndPlay()`. Uses `TSet<TWeakObjectPtr<AActor>>` for safe, efficient lookup with automatic stale reference cleanup.

### ~~Tick Performance (Interact Highlight)~~ → **Fixed** ✅
`ADefaultCharacter::Tick()` uses a configurable `InteractHighlightInterval` (default 0.1s) to throttle `UpdateInteractHighlight()` line traces.

### ~~CastChecked Usage~~ → **Fixed** ✅
`SetupPlayerInputComponent()` now uses `Cast<>` with a null check and descriptive `UE_LOG(LogSplitstream, Error, ...)` using the project log category.

### ~~Duplicate Leave/Loading Logic~~ → **Fixed** ✅
`ABasePlayerController` provides shared `RequestLeaveToMainMenu()` and `ClientShowLoadingWidget()` methods. Both `ADefaultPlayerController` and `ALobbyPlayerController` inherit from it.

### ~~Duplicate Ability Granting Loops~~ → **Fixed** ✅
Extracted `GrantAbilitiesFromSet(UAbilitySystemComponent* ASC, const UAbilityInputSet* Set)` as a private helper. `GrantAbilitiesFromInputSet()` now calls this helper for both `FutureGASet` and `SoloGASet`, eliminating the triplicated for-loop body.

### ~~Duplicate Server Time Retrieval~~ → **Fixed** ✅
`ADefaultPlayerController::GetSyncedServerTime()` is a shared const method. Both `UpdateAlarmUI()` and `UpdatePreAlarmUI()` now call it instead of duplicating the 8-line server time retrieval block.

### ~~Duplicate StateTree Event Dispatch~~ → **Fixed** ✅
`AAICharacter::OnFullyDetected_Implementation()` now selects the correct detection tag (`TAG_StateTree_Event_FullyDetected_Pawn` vs `TAG_StateTree_Event_FullyDetected_Actor`) with a single ternary expression and one dispatch block, eliminating the two near-identical code blocks.

### ~~SetRevealProgress Creates Material Instance Every Call~~ → **Fixed** ✅
`ADronePawn::SetRevealProgress()` now caches the `UMaterialInstanceDynamic*` as `CachedDynMat` and only creates it once on first call.

### ~~RobotGuardCharacter BeginPlay World Iteration~~ → **Fixed** ✅
`ARobotGuardCharacter::BeginPlay()` no longer calls `GetAllActorsOfClass(AActor::StaticClass())`. A `URepairableRegistry` world subsystem with `OnRepairableRegistered`/`OnRepairableUnregistered` delegates is used instead, with proper `HandleRepairableRegistered` / `HandleRepairableUnregistered` bindings.

### ~~DronePawn BeginPlay Null Guard~~ → **Fixed** ✅
`ADronePawn::BeginPlay()` now guards `AbilitySystemComponent && AttributeSet` before binding the health attribute delegate, consistent with `AAICharacter::BeginPlay()`.

### ~~Build.cs Duplicate Module~~ → **Fixed** ✅
`"UMG"` is only in `PublicDependencyModuleNames`. The `PrivateDependencyModuleNames` no longer duplicates it.

### ~~Timer-Based Animation Hardcoded Delta~~ → **Fixed** ✅ (This round)
`ADronePawn::MoveUpForLaunch()` and `UpdateMeshAlignInterp()` now compute actual elapsed time via `GetWorld()->GetTimeSeconds()` instead of assuming `0.02f` per callback. This ensures frame-rate-independent animation on all hardware.

### ~~Custom Log Category~~ → **Fixed** ✅ (This round)
`DECLARE_LOG_CATEGORY_EXTERN(LogSplitstream, Log, All)` is now defined in `Splitstream.h` and implemented in `Splitstream.cpp`. `DefaultCharacter.cpp` and `DefaultPlayerController.cpp` use `LogSplitstream` instead of `LogTemp`.

### ~~Error Handling in Critical Paths~~ → **Partially Fixed** ✅ (This round)
`ADefaultCharacter::InitializeAbilitySystem()` now logs a warning when `PlayerState` or `AbilitySystemComponent` is null. `ServerHandleInteract_Implementation()` now logs when `TargetActor` is null. `SetupPlayerInputComponent()` uses `LogSplitstream` for its error message. `ServerExecuteAction_Implementation()` in `DefaultPlayerController` also uses `LogSplitstream`.

---

## Strengths

### 1. Architecture & Organization ✅
- **Clean directory structure** — Logical folder layout (Characters, Controllers, GameModes, GameStates, Interfaces, ActorComponents, Actors, AbilitySystem, DataAssets, Widgets, Subsystems, Minigames).
- **Interface-driven design** — `IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, `IGhostMirrorSource`, `IGhostRevealable`, `IPuzzleCompletionReceiver`, `IServerActionInterface` provide excellent decoupling.
- **Component-based reusability** — `UDetectionComponent`, `UHackComponent`, `USearchComponent`, `ULockPickComponent`, `UProximityHackComponent`, `UInventoryComponent` are modular and composable.
- **Data-driven design** — `UItemBase`, `UAbilityInputSet`, `UDefaultGASet`, `UInputMappingSet` let designers configure gameplay without C++ changes.
- **Subsystem usage** — `UDetectorRegistry` and `URepairableRegistry` demonstrate proper use of UE5 World Subsystems for cross-system communication.

### 2. GAS Integration ✅
- **Correct ASC ownership** — ASC lives on `PlayerState`, the recommended pattern for multiplayer with pawn respawning/seamless travel.
- **Native gameplay tags** — All tags declared via `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG`.
- **Ability input binding** — Clean data-driven mapping from input actions to abilities via `UAbilityInputSet` and dynamic spec tags.
- **Attribute change delegates** — Properly bound for walk speed, run speed, crouch speed, and health.
- **Per-item ability/effect grants** — `UItemBase` handles per-instance GAS effect and ability grants with proper handle tracking for clean removal.

### 3. Multiplayer Architecture ✅
- **Server-authoritative** — All game logic (detection, alarms, inventory, interaction) runs on the server.
- **Proper replication** — Consistent use of `ReplicatedUsing`, `DOREPLIFETIME`, Server/Client/NetMulticast RPCs.
- **Seamless travel** — `CopyProperties()` on `ADefaultPlayerState` persists team assignment across maps.
- **Session cleanup** — Robust session destruction with double-destroy guards, delegate cleanup, and net driver shutdown.
- **Server time synchronization** — `GetSyncedServerTime()` helper used for all network-synced countdown display.

### 4. Gameplay Systems ✅
- **Dual-timeline system** — Past/Future era with per-era pre-alarms, alarms, and player start assignment.
- **Progressive detection** — Distance-weighted fill rate with visual feedback via detection widgets.
- **Multiple puzzle types** — Wires, levers, keypads, lasers, locks, hacking, searching, proximity hacking.
- **Full mini-games** — Space invader (Firewall) and endless runner (NeonRunner) with difficulty scaling.
- **Inventory system** — Slot-based with passive/active ability/effect grants, item dropping, team-aware spawning.

### 5. AI System ✅
- **StateTree integration** — Modern UE5 AI behavior via `UStateTreeComponent`.
- **Custom detection** — Distance-weighted progressive detection meter.
- **Drone cone detection** — Custom bounds-point-in-cone algorithm with LOS line traces and multi-point AABB testing.
- **Repair system** — `ARobotGuardCharacter` queue-based repair with `URepairableRegistry` and `IRepairable`.

### 6. Performance Optimizations ✅
- **DetectorRegistry / RepairableRegistry** — Replaces world-wide actor iteration.
- **Throttled interact highlight** — Configurable interval instead of per-frame traces.
- **Timer-based detection** — Security cameras and drones use configurable timer intervals.
- **Tick disabled by default** — Most actors set `PrimaryActorTick.bCanEverTick = false`.
- **Component tick management** — Components enable/disable tick based on active state.
- **Frame-rate-independent animation** — DronePawn timers use actual elapsed time.

---

## DefaultCharacter In-Depth Review

`ADefaultCharacter` is the player pawn class (~905 lines in .cpp, ~222 lines in .h). It is the **largest single class** in the project and handles:

1. **Movement** — Walk, sprint, crouch, jump with GAS attribute-driven speeds
2. **Camera** — Look, pitch replication, ADS socket calculation
3. **Input** — Enhanced Input binding, ability input dispatch, number key inventory selection
4. **Ability System** — ASC initialization, attribute delegates, ability granting from data assets
5. **Interaction** — Line trace highlighting, instant/hold interactions, server RPC dispatch
6. **Inventory** — Equipped item mesh management, drop logic, slot selection
7. **Detection** — IDetectable implementation, illegal tag change handling via DetectorRegistry

### What's Well Done in DefaultCharacter ✅

- **Clean helper extraction** — `GrantAbilitiesFromSet()`, `GetForwardTraceResult()`, `GetPlayerAttributeSet()` reduce duplication.
- **Proper replication** — Sprint state, pitch, inventory all replicated correctly.
- **GAS integration** — ASC initialized from PlayerState (correct ownership), attribute delegates wired for movement speed changes.
- **Throttled highlight** — `InteractHighlightInterval` prevents per-frame traces.
- **Error logging** — `InitializeAbilitySystem()` and `ServerHandleInteract_Implementation()` now log on failure.

### How to Reduce DefaultCharacter Size 🔸

The class has **7 distinct responsibilities**, which is too many for a single class. Here is a recommended decomposition strategy that preserves all functionality while making each piece more testable, reusable, and readable:

#### Recommended Extraction Plan

**a) Extract `UCharacterInteractionComponent` (~150 lines)**
Move interaction logic into a new `UActorComponent`:
- `UpdateInteractHighlight()` — line trace + highlight toggle
- `HandleInteractHoldStart()` / `HandleInteractHoldStop()` / `HandleInteractInstant()`
- `ServerHandleInteract()` RPC
- `InteractDistance`, `InteractHighlightInterval`, `HighlightedActor`, `ProgressiveActor`, `bIsHoldingInteract`
- `GetForwardTraceResult()` utility (shared via component reference)

**Benefits:** Interaction system becomes reusable on any pawn. The component can be added to AI characters or vehicles that need interaction. Tick-based highlight logic is encapsulated.

**b) Extract `UCharacterAbilityComponent` (~100 lines)**
Move GAS initialization and ability management:
- `InitializeAbilitySystem()` — ASC lookup, attribute delegate binding
- `GrantAbilitiesFromInputSet()` / `GrantAbilitiesFromDefaultSet()` / `GrantAbilitiesFromSet()`
- `HandleAbilityInput()` / `HandleAbilityInputReleased()`
- `OnIllegalTagChanged()` — tag event handler
- GAS data references: `FutureGASet`, `SoloGASet`, `DefaultGASet`, `AttributeInitGE`

**Benefits:** Separates GAS concerns. The ability granting logic is self-contained and easier to test. Tag event handling is near the ASC initialization that sets it up.

**c) Extract Inventory Mesh Management to `UInventoryComponent` (~60 lines)**
Move equipped item visual logic:
- `UpdateEquippedItemMesh()` — mesh/transform management
- `OnInventoryChanged()` delegate handler
- `EquippedItemMeshComp` management
- ADS socket calculation (`CameraAimLocation`, `CameraAimRotation`)

**Benefits:** `UInventoryComponent` already exists; this logic naturally belongs there. The inventory component already handles item data; adding the visual mesh representation makes it a complete system.

**d) Keep in DefaultCharacter (~500 lines → manageable)**
After extraction, DefaultCharacter retains:
- Constructor and component creation
- Movement (Move, Look, Jump, Sprint, Crouch) with attribute callbacks
- Camera management and pitch replication
- Input setup (`SetupPlayerInputComponent`)
- Detection interface implementation (`OnDetected`, `OnLost`, `OnForceDetectionEnd`)
- Lifecycle overrides (`BeginPlay`, `Tick`, `PossessedBy`, `OnRep_PlayerState`)
- Replication (`GetLifetimeReplicatedProps`)
- `CalculateDetectionAngle` static utility

This reduces the file from ~905 lines to ~500 lines, a 45% reduction, with each extracted component being independently testable and reusable.

### Remaining Issues in DefaultCharacter

#### 1. Constructor Magic Numbers 🔸
```cpp
GetCharacterMovement()->JumpZVelocity = 300.f;
GetCharacterMovement()->AirControl = 0.35f;
GetCharacterMovement()->MaxWalkSpeed = 300.f;
GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
```
These should be `UPROPERTY(EditDefaultsOnly)` constants or exclusively initialized from `AttributeInitGE`. Currently they set defaults that may conflict with the attribute init gameplay effect.

#### 2. `HandleInteractInstant()` Dual Path 🔸
```cpp
void ADefaultCharacter::HandleInteractInstant()
{
    if (HasAuthority() && IsLocallyControlled())
    {
        // Listen server path: trace + ServerHandleInteract
        ...
        return;
    }
    // Client path: trace + local prediction + ServerHandleInteract
    ...
}
```
This method has two nearly-identical code paths (listen server vs. client) with subtle differences. The client path calls `IInteractable::Execute_Interact()` locally before the server RPC (client-side prediction), but the listen server path does not. This asymmetry could cause different behavior between listen server and dedicated server hosts. Consider unifying the paths.

#### 3. `HandleNumberKey()` Redundant Parsing 🟡
The method has two parsing strategies: first it tries character-based digit parsing, then falls back to a `TMap<FName, int32>` lookup. Since all keys are bound individually via `BindKey(EKeys::One, ...)`, the `FKey` will always match — the character parsing path is redundant. One approach is sufficient.

#### 4. Missing `EndPlay` Cleanup 🟡
`ADefaultCharacter::EndPlay()` is empty — it only calls `Super::EndPlay()`. Consider cleaning up:
- Tag event delegate registered in `InitializeAbilitySystem()` (`OnIllegalTagChanged`)
- Attribute change delegates (walk/run/crouch speed)

While UE garbage collection handles most of this, explicit cleanup is safer for multiplayer scenarios where ASC outlives the pawn (ASC is on PlayerState).

#### 5. `DropActiveItem()` Hard Floor Trace 🟡
```cpp
FVector DownTraceStart = DropLocation + FVector(0, 0, 50);
FVector DownTraceEnd = DropLocation - FVector(0, 0, 200);
```
Hardcoded trace offsets (50 units up, 200 units down). These should be constants or `UPROPERTY` values, especially since different maps might have different floor distances.

---

## Remaining Global Issues

### 1. Code Documentation 🔸 (Medium Priority)

Most `UCLASS`, `UFUNCTION`, and `UPROPERTY` declarations still lack doc comments. `UDetectorRegistry` and `URepairableRegistry` have good comments as examples to follow.

**Recommendation:** Prioritize documenting all interface methods (most impactful for Blueprint users) and the GameState alarm/pre-alarm API.

### 2. Magic Numbers & Hardcoded Strings 🔸 (Medium Priority)

**Remaining:**
- Constructor movement values in `DefaultCharacter.cpp`
- String comparison for team names (`"Past"`, `"Future"`, `"Solo"`) in `DefaultPlayerState.cpp`, `InventoryComponent.cpp`, `DefaultGameMode.cpp`, `LobbyGameMode.cpp`
- Hardcoded timer intervals (`1.0f` ping, `0.1f` alarm/prealarm UI update) in `DefaultPlayerController.cpp`

**Recommendation:** Consider an `ETeamType` enum or `FGameplayTag`-based team comparison instead of raw `FString` comparisons scattered across 5+ files.

### 3. Detection Component Network Bandwidth 🔸 (Medium Priority)

`UDetectionComponent::TickComponent()` calls `ClientUpdateDetectionWidget()` (a `Client, Unreliable` RPC) for **every detector** on **every player controller** each tick while detection is active.

**Impact:** With 4 players and 2 active detectors, this sends 8 RPCs per tick (~480/sec). Even as unreliable, this is high bandwidth.

**Recommendation:**
- Send detection updates at a reduced rate (e.g., every 0.1s) rather than every tick
- Only send updates when progress changes meaningfully (e.g., ≥1% change)

### 4. Remaining Error Handling Gaps 🟡 (Low Priority)

Some server RPCs still silently execute without validation logging:
```cpp
void ServerStartSprint_Implementation() { StartSprint(); }
void ServerStopSprint_Implementation() { StopSprint(); }
```

**Recommendation:** Add `UE_LOG(LogSplitstream, Verbose, ...)` to server RPCs for debugging multiplayer issues. Use `Verbose` level to avoid log spam in production.

---

## What's Done Well vs. What Needs Work

| Area | Status | Notes |
|---|---|---|
| Architecture | ✅ Excellent | Clean separation, interface-driven, subsystems |
| GAS Integration | ✅ Excellent | Correct ownership, per-item grants, attributes |
| Multiplayer | ✅ Very Good | Server-authoritative, session cleanup, synced timers |
| Gameplay Depth | ✅ Excellent | Rich feature set, dual timeline, mini-games |
| AI System | ✅ Very Good | StateTree, progressive detection, repair queue |
| Code Organization | ✅ Very Good | Clear folder structure, base class hierarchy |
| Data-Driven Design | ✅ Very Good | DataAssets for items, abilities, input |
| Performance Patterns | ✅ Very Good | Subsystem registries, throttled traces, managed ticking, frame-rate-independent animation |
| Error Handling | ✅ Improved | Custom log category, logging on critical paths |
| Helper Extraction | ✅ Improved | `GrantAbilitiesFromSet`, `GetSyncedServerTime`, conditional tag dispatch |
| Code Documentation | 🔸 Needs Work | Most classes still lack doc comments |
| Magic Numbers/Strings | 🔸 Needs Work | Constructor values, string team comparisons |
| Network Bandwidth | 🔸 Needs Work | Per-tick RPCs in detection component |
| DefaultCharacter Size | 🔸 Needs Work | ~905 lines, 7 responsibilities — see decomposition plan above |

---

## Grade: **8.5 / 10** (up from 8.0)

### Justification

**Splitstream continues to improve measurably with each review round.** The codebase now demonstrates:

**Points gained this round (+0.5):**
- **Custom log category** (+0.15) — `LogSplitstream` replaces `LogTemp`, enabling filtered log output for debugging.
- **Error logging in critical paths** (+0.15) — `InitializeAbilitySystem()` and `ServerHandleInteract` now log failures, making multiplayer debugging much easier.
- **Frame-rate-independent animation** (+0.1) — DronePawn timers now use actual elapsed time, fixing stuttery animation on high-refresh displays.
- **Cumulative fix quality** (+0.1) — The overall pattern of fixes shows strong engineering discipline: subsystem registries, helper extraction, null guards, and logging.

**Remaining deductions (-1.5):**
- **Code documentation** (-0.3) — Most classes still have no doc comments.
- **DefaultCharacter size** (-0.3) — 905 lines with 7 responsibilities is too much for one class. The decomposition plan is straightforward.
- **Magic numbers/strings** (-0.3) — Constructor values and string-based team comparisons.
- **Network bandwidth** (-0.3) — Per-tick RPCs in detection system.
- **Minor issues** (-0.3) — Remaining logging gaps, `HandleInteractInstant` dual path, hardcoded trace offsets.

### Path to 9.0+

1. **Decompose DefaultCharacter** — Extract `UCharacterInteractionComponent` and `UCharacterAbilityComponent` (biggest single improvement)
2. **Add Doxygen doc comments** to all public interfaces and key classes
3. **Throttle detection RPCs** to reduce network bandwidth
4. **Replace string team comparisons** with `ETeamType` enum or `FGameplayTag`

This is a strong **senior-level** UE5 project. The iterative improvement pattern across reviews demonstrates real engineering maturity.
