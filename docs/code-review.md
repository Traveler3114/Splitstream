# Code Review (Updated — Round 5)

This document provides a comprehensive code review of the **Splitstream** codebase under `Source/Splitstream/`, reflecting all fixes applied across review rounds, including newly verified improvements in Round 5.

---

## Summary

Splitstream is a well-architected, feature-rich Unreal Engine 5 multiplayer heist game. The project integrates the Gameplay Ability System (GAS), a server-authoritative multiplayer architecture, custom AI systems with StateTree, and multiple interactive puzzle mechanics. Significant improvements have been made since the initial review. **Round 5** confirms that all previously identified code-level issues have been resolved: stale development comments removed, garbled Unicode in doc comments fixed, dead commented-out code cleaned up, `CachedDynMat` properly initialized, `IsPendingKillPending()` replaced with `IsValid()`, `LaunchMoveTimerHandle` cleanup added to `EndPlay()`, remaining `LogTemp` replaced with `LogSplitstream`, and `DroneSpawner::TickPlatformAnim()` now uses actual elapsed time. The codebase is now clean of all identified technical debt.

---

## All Issues Fixed Across Reviews ✅

### ~~TActorIterator in Tag Change Handler~~ → **Fixed** ✅
`ADefaultCharacter::OnIllegalTagChanged()` no longer iterates all world actors. A `UDetectorRegistry` world subsystem tracks detector actors via `Register()`/`Unregister()` in `BeginPlay()`/`EndPlay()`. Uses `TSet<TWeakObjectPtr<AActor>>` for safe, efficient lookup with automatic stale reference cleanup.

### ~~Tick Performance (Interact Highlight)~~ → **Fixed** ✅
`ADefaultCharacter::Tick()` uses `InteractionComponent->InteractHighlightInterval` (default 0.1s) to throttle `UpdateInteractHighlight()` line traces.

### ~~CastChecked Usage~~ → **Fixed** ✅
`SetupPlayerInputComponent()` now uses `Cast<>` with a null check and descriptive `UE_LOG(LogSplitstream, Error, ...)` using the project log category.

### ~~Duplicate Leave/Loading Logic~~ → **Fixed** ✅
`ABasePlayerController` provides shared `RequestLeaveToMainMenu()` and `ClientShowLoadingWidget()` methods. Both `ADefaultPlayerController` and `ALobbyPlayerController` inherit from it.

### ~~Duplicate Ability Granting Loops~~ → **Fixed** ✅ (Improved in Round 4)
Ability granting is now fully extracted to `UDefaultAbilitySystemComponent`. Methods `GrantAbilitiesFromSet()`, `GrantAbilitiesFromDefaultSet()`, `InitializeAttributes()`, `HandleAbilityInputPressed()`, and `HandleAbilityInputReleased()` are all on the custom ASC subclass. `DefaultCharacter` delegates to `GetDefaultASC()` for all ability operations.

### ~~Duplicate Server Time Retrieval~~ → **Fixed** ✅
`ADefaultPlayerController::GetSyncedServerTime()` is a shared const method. Both `UpdateAlarmUI()` and `UpdatePreAlarmUI()` now call it instead of duplicating the server time retrieval block.

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

### ~~Timer-Based Animation Hardcoded Delta~~ → **Fixed** ✅
`ADronePawn::MoveUpForLaunch()` and `UpdateMeshAlignInterp()` now compute actual elapsed time via `GetWorld()->GetTimeSeconds()` instead of assuming `0.02f` per callback. This ensures frame-rate-independent animation on all hardware.

### ~~Custom Log Category~~ → **Fixed** ✅
`DECLARE_LOG_CATEGORY_EXTERN(LogSplitstream, Log, All)` is now defined in `Splitstream.h` and implemented in `Splitstream.cpp`. `DefaultCharacter.cpp` and `DefaultPlayerController.cpp` use `LogSplitstream` instead of `LogTemp`.

### ~~Error Handling in Critical Paths~~ → **Fixed** ✅
`ADefaultCharacter::InitializeAbilitySystem()` now logs a warning when `PlayerState` or `AbilitySystemComponent` is null. `ServerHandleInteract_Implementation()` now logs when `TargetActor` is null. `SetupPlayerInputComponent()` uses `LogSplitstream` for its error message. `ServerExecuteAction_Implementation()` in `DefaultPlayerController` also uses `LogSplitstream`.

### ~~Missing EndPlay Cleanup~~ → **Fixed** ✅ (New in Round 4)
`ADefaultCharacter::EndPlay()` now properly removes GAS delegates — the illegal tag event delegate and attribute change delegates (walk, run, crouch speed) — using stored `FDelegateHandle` members (`IllegalTagDelegateHandle`, `WalkSpeedDelegateHandle`, `RunSpeedDelegateHandle`, `CrouchSpeedDelegateHandle`). This prevents dangling references when the ASC on PlayerState outlives the pawn.

### ~~HandleInteractInstant() Dual Path~~ → **Fixed** ✅ (New in Round 4)
Instant interaction logic has been extracted to `UInteractionComponent::HandleInstantInteract()`. The listen-server skip is now a clean single-line guard (`if PawnOwner is locally controlled and has authority, return`), and the client always forwards via the provided `ServerInteractCallback`.

### ~~HandleNumberKey() Redundant Parsing~~ → **Fixed** ✅ (New in Round 4)
`HandleNumberKey()` now uses only a single `TMap<FName, int32>` lookup strategy. The redundant character-based digit parsing has been removed.

---

## New Improvements Found in Round 4

### Interaction Logic Extraction → `UInteractionComponent` ✅
A new `UInteractionComponent` (in `ActorComponents/InteractionComponent.h/.cpp`) now owns:
- `UpdateInteractHighlight()` — line trace + highlight toggle
- `HandleHoldInteractStart()` / `HandleHoldInteractStop()` / `HandleInstantInteract()`
- `GetForwardTraceResult()` utility
- `DropEquippedItem()` — drop logic with camera-forward trace
- `InteractDistance`, `InteractHighlightInterval`, `HighlightedActor`, `ProgressiveActor`, `bIsHoldingInteract`

`ADefaultCharacter` now simply delegates to the component: `OnInstantInteract()`, `OnHoldInteractStart()`, `OnHoldInteractStop()`, `OnDropActiveItem()` are thin wrappers. This is exactly the decomposition recommended in Round 3.

### Ability System Extraction → `UDefaultAbilitySystemComponent` ✅
A new `UDefaultAbilitySystemComponent` (in `AbilitySystem/DefaultAbilitySystemComponent.h/.cpp`) now owns:
- `GrantAbilitiesFromSet(const UAbilityInputSet*)` — input-tagged ability granting
- `GrantAbilitiesFromDefaultSet(const UDefaultGASet*)` — default ability granting
- `InitializeAttributes(TSubclassOf<UGameplayEffect>)` — attribute init GE application
- `HandleAbilityInputPressed(FGameplayTag)` / `HandleAbilityInputReleased(FGameplayTag)` — input routing

`ADefaultCharacter` delegates via `GetDefaultASC()` typed accessor. `PossessedBy()` calls `ASC->GrantAbilitiesFromSet()` and `ASC->GrantAbilitiesFromDefaultSet()` directly.

### DefaultCharacter Size Reduction ✅
With the extraction of `UInteractionComponent` and ability logic to `UDefaultAbilitySystemComponent`, `DefaultCharacter.cpp` is now ~741 lines (down from ~905), a **~18% reduction**. The `.h` file is also cleaner with fewer responsibilities.

### Delegate Handle Storage ✅
`ADefaultCharacter` now stores `FDelegateHandle` members for all GAS delegates:
- `IllegalTagDelegateHandle`
- `WalkSpeedDelegateHandle`
- `RunSpeedDelegateHandle`
- `CrouchSpeedDelegateHandle`

These are properly assigned in `InitializeAbilitySystem()` and cleaned up in `EndPlay()`.

---

## Strengths

### 1. Architecture & Organization ✅
- **Clean directory structure** — Logical folder layout (Characters, Controllers, GameModes, GameStates, Interfaces, ActorComponents, Actors, AbilitySystem, DataAssets, Widgets, Subsystems, Minigames).
- **Interface-driven design** — `IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, `IGhostMirrorSource`, `IGhostRevealable`, `IPuzzleCompletionReceiver`, `IServerActionInterface` provide excellent decoupling.
- **Component-based reusability** — `UDetectionComponent`, `UHackComponent`, `USearchComponent`, `ULockPickComponent`, `UProximityHackComponent`, `UInventoryComponent`, `UInteractionComponent` are modular and composable.
- **Data-driven design** — `UItemBase`, `UAbilityInputSet`, `UDefaultGASet`, `UInputMappingSet` let designers configure gameplay without C++ changes.
- **Subsystem usage** — `UDetectorRegistry` and `URepairableRegistry` demonstrate proper use of UE5 World Subsystems for cross-system communication.
- **Custom ASC subclass** — `UDefaultAbilitySystemComponent` consolidates ability granting and input routing logic in the correct place.

### 2. GAS Integration ✅
- **Correct ASC ownership** — ASC lives on `PlayerState`, the recommended pattern for multiplayer with pawn respawning/seamless travel.
- **Native gameplay tags** — All tags declared via `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG`.
- **Ability input binding** — Clean data-driven mapping from input actions to abilities via `UAbilityInputSet` and dynamic spec tags.
- **Attribute change delegates** — Properly bound for walk speed, run speed, crouch speed, and health, with `FDelegateHandle` stored for cleanup.
- **Per-item ability/effect grants** — `UItemBase` handles per-instance GAS effect and ability grants with proper handle tracking for clean removal.
- **Custom ASC methods** — `HandleAbilityInputPressed()`/`Released()` on the ASC keep input routing clean.

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

### 7. Code Quality & Lifecycle Management ✅ (Improved in Round 5)
- **Delegate cleanup in EndPlay** — `ADefaultCharacter` and `ADefaultPlayerController` both properly clean up GAS delegates on destruction.
- **Component extraction** — `UInteractionComponent` and `UDefaultAbilitySystemComponent` demonstrate proper responsibility separation.
- **Typed ASC accessor** — `GetDefaultASC()` provides safe access to the custom component without casting throughout the class.
- **No stale comments or dead code** — All development markers (`// ADD THIS`, `// REMOVED:`) and commented-out code have been cleaned up.
- **Consistent log category** — All files now use `LogSplitstream`; no `LogTemp` remains.
- **Modern UE5 API usage** — Deprecated `IsPendingKillPending()` replaced with `IsValid()` throughout.

---

## DefaultCharacter In-Depth Review

`ADefaultCharacter` is the player pawn class (~737 lines in .cpp, ~208 lines in .h). After Round 5 cleanup, it now handles:

1. **Movement** — Walk, sprint, crouch, jump with GAS attribute-driven speeds
2. **Camera** — Look, pitch replication, ADS socket calculation
3. **Input** — Enhanced Input binding, ability input dispatch, number key inventory selection
4. **Ability System** — ASC initialization, attribute delegates (granting logic delegated to `UDefaultAbilitySystemComponent`)
5. **Interaction** — Delegated to `UInteractionComponent`; thin wrappers for input events
6. **Inventory** — Equipped item mesh management, drop logic, slot selection
7. **Detection** — IDetectable implementation, illegal tag change handling via DetectorRegistry

### What's Well Done in DefaultCharacter ✅

- **Clean component delegation** — Interaction via `UInteractionComponent`, abilities via `UDefaultAbilitySystemComponent`.
- **Proper delegate cleanup** — `EndPlay()` removes all 4 GAS delegate handles.
- **Proper replication** — Sprint state, pitch, inventory all replicated correctly.
- **GAS integration** — ASC initialized from PlayerState (correct ownership), attribute delegates wired with stored handles.
- **Throttled highlight** — `InteractHighlightInterval` prevents per-frame traces.
- **Error logging** — `InitializeAbilitySystem()` and `ServerHandleInteract_Implementation()` log on failure.
- **Clean number key handling** — Single `TMap` lookup, no redundant parsing.

### All DefaultCharacter Issues Resolved ✅

Previous round identified stale `// ADD THIS` and `// REMOVED:` development comments, and hardcoded trace offsets for `DropActiveItem()`. Development comments have been cleaned up in Round 5. The `DropEquippedItem()` method in `InteractionComponent.cpp` now uses local named constants (`UpwardOffset`, `DownwardTrace`) for trace distances rather than raw magic numbers in the trace call.

---

## All Round 4 Issues Resolved ✅

### ~~`CachedDynMat` Uninitialized Pointer~~ → **Fixed** ✅
`CachedDynMat` is now initialized to `nullptr` in the header declaration: `UMaterialInstanceDynamic* CachedDynMat = nullptr;`

### ~~`IsPendingKillPending()` Deprecated API~~ → **Fixed** ✅
`DefaultPlayerController.cpp` now uses `!IsValid(Detector)` instead of the deprecated `IsPendingKillPending()`.

### ~~`LogTemp` Still Used in Some Files~~ → **Fixed** ✅
All `LogTemp` usages in `DefaultGameInstance.cpp` and `DroneSpawner.cpp` have been replaced with `LogSplitstream`.

### ~~`LaunchMoveTimerHandle` Not Cleared in `DronePawn::EndPlay()`~~ → **Fixed** ✅
`DronePawn::EndPlay()` now clears all three timer handles: `DetectionTimerHandle`, `MeshAlignTimerHandle`, and `LaunchMoveTimerHandle`.

### ~~`DroneSpawner::TickPlatformAnim()` Hardcoded Delta~~ → **Fixed** ✅
`DroneSpawner::TickPlatformAnim()` now tracks `LastPlatformAnimTime` and computes actual elapsed delta time, consistent with `DronePawn::MoveUpForLaunch()`.

### `DronePawn` Repairable Registry Registration ✅
`ADronePawn::BeginPlay()` now calls `UUtilityLibrary::RegisterRepairable(this, this)` and `EndPlay()` calls `UUtilityLibrary::UnregisterRepairable(this, this)`, making it discoverable through the registry.

### ~~`DronePawn` Health Delegate Re-binding~~ → **Fixed** ✅
`DronePawn::RequestRepair_Implementation()` now calls `RemoveAll(this)` before `AddUObject()` for the health delegate, preventing potential duplicate bindings.

### ~~Stale Development Comments~~ → **Fixed** ✅ (New in Round 5)
Leftover `// REMOVED:` comments in `DefaultCharacter.h` and commented-out code in `DefaultCharacter::PostInitializeComponents()` have been cleaned up.

### ~~Garbled Unicode in Doc Comment~~ → **Fixed** ✅ (New in Round 5)
The garbled `�` character in the `GetDefaultASC()` doc comment has been replaced with a proper em dash (`—`).

---

## Remaining Global Issues

### 1. Code Documentation 🔸 (Medium Priority)

Most `UCLASS`, `UFUNCTION`, and `UPROPERTY` declarations still lack doc comments. `UDetectorRegistry`, `URepairableRegistry`, and `UInteractionComponent` have good comments as examples to follow.

**Recommendation:** Prioritize documenting all interface methods (most impactful for Blueprint users) and the GameState alarm/pre-alarm API.

### 2. Magic Numbers & Hardcoded Strings 🔸 (Medium Priority)

**Remaining:**
- Hardcoded timer intervals (`1.0f` ping, `0.1f` alarm/prealarm UI update) in `DefaultPlayerController.cpp`
- Hardcoded floor trace offsets in `InteractionComponent.cpp`

**Improved since last review:** Team comparisons now use `FGameplayTag` throughout (`TeamTag` is an `FGameplayTag` on `DefaultPlayerState`). String-based team comparisons are effectively eliminated — team display names use a static method `GetTeamDisplayName()` that compares tags, not strings. ✅

### 3. Detection Component Network Bandwidth 🔸 (Medium Priority)

`UDetectionComponent::TickComponent()` calls `ClientUpdateDetectionWidget()` (a `Client, Unreliable` RPC) for **every detector** on **every player controller** each tick while detection is active. The component tick interval is set to `0.1f` which already provides some throttling.

**Impact:** With 4 players and 2 active detectors, this sends 8 RPCs per tick (~80/sec at 0.1s interval). Acceptable, but could be further optimized.

**Recommendation:**
- Only send updates when progress changes meaningfully (e.g., ≥1% change)
- Consider sending only to the owning player controller rather than all controllers

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
| Architecture | ✅ Excellent | Clean separation, interface-driven, subsystems, component extraction |
| GAS Integration | ✅ Excellent | Correct ownership, per-item grants, attributes, custom ASC subclass |
| Multiplayer | ✅ Very Good | Server-authoritative, session cleanup, synced timers |
| Gameplay Depth | ✅ Excellent | Rich feature set, dual timeline, mini-games |
| AI System | ✅ Very Good | StateTree, progressive detection, repair queue |
| Code Organization | ✅ Very Good | Clear folder structure, base class hierarchy |
| Data-Driven Design | ✅ Very Good | DataAssets for items, abilities, input |
| Performance Patterns | ✅ Very Good | Subsystem registries, throttled traces, managed ticking, frame-rate-independent animation |
| Error Handling | ✅ Good | Custom log category, logging on critical paths, delegate cleanup |
| Helper Extraction | ✅ Very Good | `UDefaultAbilitySystemComponent`, `UInteractionComponent`, `GetSyncedServerTime` |
| Lifecycle Management | ✅ Very Good | EndPlay cleanup, delegate handle storage, consistent timer cleanup |
| Code Quality | ✅ Good | No stale comments, consistent log category, no deprecated API usage |
| Code Documentation | 🔸 Needs Work | Most classes still lack doc comments |
| Network Bandwidth | 🟡 Acceptable | Tick-interval throttled RPCs in detection, could optimize further |

---

## Grade: **9.0 / 10** (up from 8.8)

### Justification

**Splitstream has reached the 9.0 milestone with Round 5.** All previously identified code-level issues have been resolved.

**Points gained this round (+0.2):**
- **Stale comments and dead code cleanup** (+0.05) — All `// REMOVED:`, `// ADD THIS`, and commented-out code blocks have been removed. The codebase now reads cleanly without development artifacts.
- **`CachedDynMat` initialization** (+0.05) — Already fixed; confirmed initialized to `nullptr` in the header. Prevents undefined behavior.
- **`IsPendingKillPending()` replaced** (+0.03) — Already fixed; confirmed `IsValid()` used throughout. Modern UE5 API.
- **`LogTemp` fully replaced** (+0.02) — Already fixed; confirmed all log calls use `LogSplitstream`.
- **`LaunchMoveTimerHandle` cleanup** (+0.02) — Already fixed; confirmed cleared in `EndPlay()`.
- **`DroneSpawner` delta timing** (+0.02) — Already fixed; confirmed `LastPlatformAnimTime` delta used.
- **Garbled Unicode fixed** (+0.01) — Doc comment now uses proper em dash character.

**Remaining deductions (-1.0):**
- **Code documentation** (-0.3) — Most classes still have no doc comments. Interfaces and GameState alarm API should be documented first.
- **DefaultCharacter size** (-0.1) — ~737 lines in .cpp. Further extraction of inventory mesh management and detection handling would bring it under 500 lines.
- **Network bandwidth** (-0.15) — Detection RPCs could be further optimized (only send on meaningful change).
- **Hardcoded timer intervals** (-0.1) — Ping update (1.0f), alarm/prealarm UI update (0.1f) in `DefaultPlayerController.cpp` should be named constants or UPROPERTY values.
- **Remaining error handling gaps** (-0.1) — Sprint RPCs silently execute without verbose logging.
- **Minor remaining items** (-0.25) — `DronePawn` health delegate could use `FDelegateHandle` pattern instead of `RemoveAll`/`AddUObject`; DroneSpawner::ActivatePendingDrone still uses `GetAllActorsOfClass` for NavNode lookup.

### Path to 9.5+

1. **Add Doxygen doc comments** to all public interfaces and key classes — Biggest remaining improvement area
2. **Throttle detection RPCs** — Only send on meaningful progress change (≥1%)
3. **Extract inventory mesh management** from `DefaultCharacter` to a dedicated component
4. **Replace hardcoded timer intervals** with named constants or UPROPERTY values
5. **NavNode registry/subsystem** — Replace `GetAllActorsOfClass(ANavNode)` in `DroneSpawner::ActivatePendingDrone()` with a registry pattern

This is a strong **senior-level** UE5 project that has reached professional-grade quality. The iterative improvement pattern across five review rounds demonstrates exceptional engineering maturity. All identified bugs and code quality issues have been systematically addressed. The architecture decisions — ASC on PlayerState, component extraction, subsystem registries, interface-driven design, frame-rate-independent animation — are all industry best practices.
