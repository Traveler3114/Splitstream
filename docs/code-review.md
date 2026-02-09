# Code Review (Updated)

This document provides an updated comprehensive code review of the **Splitstream** codebase under `Source/Splitstream/`, following fixes applied since the initial review.

---

## Summary

Splitstream is a well-architected, feature-rich Unreal Engine 5 multiplayer heist game. The project integrates the Gameplay Ability System (GAS), a server-authoritative multiplayer architecture, custom AI systems with StateTree, and multiple interactive puzzle mechanics. Since the last review, several high-priority issues have been addressed, demonstrating active improvement and code quality awareness.

---

## Issues Fixed Since Last Review ✅

### ~~TActorIterator in Tag Change Handler~~ → **Fixed** ✅
The most critical performance issue from the previous review has been resolved. `ADefaultCharacter::OnIllegalTagChanged()` no longer iterates all world actors. Instead, a `UDetectorRegistry` world subsystem now tracks detector actors via `Register()`/`Unregister()` in `BeginPlay()`/`EndPlay()`. Both `ASecurityCamera` and `AAICharacter` register properly. The subsystem uses `TSet<TWeakObjectPtr<AActor>>` for safe, efficient lookup with automatic stale reference cleanup.

### ~~Tick Performance (Interact Highlight)~~ → **Fixed** ✅
`ADefaultCharacter::Tick()` no longer calls `UpdateInteractHighlight()` every frame. A configurable `InteractHighlightInterval` (defaulting to 0.1s) throttles the line trace, reducing unnecessary per-frame overhead while keeping interaction responsive.

### ~~CastChecked Usage~~ → **Fixed** ✅
`SetupPlayerInputComponent()` now uses `Cast<>` with a null check and descriptive `UE_LOG(LogTemp, Error, ...)` instead of `CastChecked<>`, preventing a crash if the Enhanced Input Component is misconfigured.

### ~~Duplicate Leave/Loading Logic~~ → **Partially Fixed** ✅
`ABasePlayerController` now provides shared `RequestLeaveToMainMenu()` and `ClientShowLoadingWidget()` methods. Both `ADefaultPlayerController` and `ALobbyPlayerController` inherit from it, eliminating the duplicated leave-to-main-menu and loading screen code.

---

## Strengths

### 1. Architecture & Organization ✅
- **Clean directory structure** — Code is well-organized into logical folders (Characters, Controllers, GameModes, GameStates, Interfaces, ActorComponents, Actors, AbilitySystem, DataAssets, Widgets, Subsystems, Minigames).
- **Interface-driven design** — `IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, `IGhostMirrorSource`, `IGhostRevealable`, `IPuzzleCompletionReceiver`, and `IServerActionInterface` provide excellent decoupling. Actors interact through interfaces, not concrete class references.
- **Component-based reusability** — `UDetectionComponent`, `UHackComponent`, `USearchComponent`, `ULockPickComponent`, `UProximityHackComponent`, and `UInventoryComponent` are modular, reusable, and composable. Multiple actor types share the same interaction components.
- **Data-driven design** — `UItemBase`, `UAbilityInputSet`, `UDefaultGASet`, `UInputMappingSet` allow designers to configure gameplay without C++ changes. Items define their own effects, abilities, and pickup classes.
- **Subsystem usage** — `UDetectorRegistry` demonstrates proper use of UE5 World Subsystems for efficient cross-system communication.

### 2. GAS Integration ✅
- **Correct ASC ownership** — ASC lives on `PlayerState` (not pawn), which is the recommended pattern for multiplayer with pawn respawning/seamless travel.
- **Native gameplay tags** — All tags declared via `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG` for compile-time safety and global accessibility.
- **Ability input binding** — Clean data-driven mapping from input actions to abilities via `UAbilityInputSet` and dynamic spec tags.
- **Attribute change delegates** — Properly bound for walk speed, run speed, crouch speed, and health to update movement and HUD in real-time.
- **Gameplay Cue usage** — PastEcho uses Gameplay Cues correctly for visual-only client-side effects.
- **Per-item ability/effect grants** — `UItemBase` handles per-instance GAS effect and ability grants with proper handle tracking for clean removal.

### 3. Multiplayer Architecture ✅
- **Server-authoritative** — All game logic (detection, alarms, inventory, interaction) runs on the server; clients receive replicated state via `OnRep_*` callbacks.
- **Proper replication** — Consistent use of `ReplicatedUsing`, `DOREPLIFETIME`, Server/Client/NetMulticast RPCs. Components properly set `SetIsReplicatedByDefault(true)`.
- **Seamless travel** — `bUseSeamlessTravel` with `CopyProperties()` on `ADefaultPlayerState` to persist team assignment across maps.
- **Session cleanup** — Robust session destruction in `UDefaultGameInstance` with guards against double-destroy (`bSessionDestroyInProgress`), proper delegate cleanup, and net driver shutdown.
- **Server time synchronization** — Alarm/pre-alarm UI uses `GetServerWorldTimeSeconds()` for network-synced countdown display.

### 4. Gameplay Systems ✅
- **Dual-timeline system** — Past/Future era system with per-era pre-alarms, alarms, and player start assignment.
- **Ghost mirroring** — Cross-timeline awareness via `IGhostMirrorSource` and `GhostCharacterActor` with offset-based positioning.
- **Progressive detection** — Distance-weighted detection fill rate (`BaseDetectionSpeedMultiplier / distance`) with visual feedback via detection widgets.
- **Multiple puzzle types** — Wires, levers, keypads, lasers, locks (with sweet-spot pin mechanics), hacking, searching, proximity hacking.
- **Full mini-games** — Space invader (Firewall) and endless runner (NeonRunner) mini-games with difficulty scaling, boss fights, and custom Slate/UMG rendering.
- **Procedural generation** — Randomized civilian names, codes, puzzle sequences, and calendar dates.
- **Inventory system** — Full item management with passive/active ability/effect grants, item dropping with physics, team-aware item spawning, and future item invalidation.

### 5. AI System ✅
- **StateTree integration** — Modern UE5 AI behavior via `UStateTreeComponent` with custom event dispatch for detection and repair.
- **Custom detection** — Distance-weighted progressive detection meter on `UDetectionComponent`.
- **Drone cone detection** — Custom bounds-point-in-cone algorithm with LOS line traces and multi-point AABB testing.
- **Repair system** — `ARobotGuardCharacter` queue-based repair with delegate-driven dispatch and `IRepairable` interface.
- **Drone lifecycle** — Activation with smooth launch animation, death with ragdoll physics, repair with mesh realignment interpolation.
- **Guard behavior** — Pre-alarm escalation per-era, death with ragdoll + ghost cleanup, name rendering.

### 6. Performance Optimizations ✅ (New since last review)
- **DetectorRegistry subsystem** — Replaces world-wide actor iteration for illegal tag changes.
- **Throttled interact highlight** — Configurable interval instead of per-frame traces.
- **Timer-based detection** — Security cameras and drones use configurable timer intervals instead of ticking every frame.
- **Tick disabled by default** — Most actors set `PrimaryActorTick.bCanEverTick = false`, enabling tick only when needed.
- **Component tick management** — `UDetectionComponent`, `UHackComponent`, `USearchComponent` enable/disable tick based on active state.

---

## Remaining Areas for Improvement

### 1. Code Documentation 🔸 (Medium Priority)

**Status:** Still needs work. Most classes lack header-level documentation comments.

**What's good:** `UDetectorRegistry` has clear doc comments. `UFutureGAPastEcho` has meaningful documentation. Some inline comments explain logic well (e.g., detection component distance multiplier).

**What needs work:** The majority of `UCLASS`, `UFUNCTION`, and `UPROPERTY` declarations across all files still have no doc comments. Critical systems like `ADefaultGameState::StartPreAlarm()`, `UDetectionComponent::TickComponent()`, and `ADefaultCharacter::HandleInteractInstant()` are complex but undocumented.

**Recommendation:** Prioritize documenting:
1. All interface methods (most important for Blueprint users and team members)
2. GameState alarm/pre-alarm API
3. Inventory system public methods
4. Detection system flow

---

### 2. Magic Numbers & Hardcoded Strings 🔸 (Medium Priority)

**Status:** Partially improved but still present.

**Remaining issues:**
```cpp
// DefaultCharacter.cpp — Constructor magic numbers (no UPROPERTY)
GetCharacterMovement()->JumpZVelocity = 300.f;
GetCharacterMovement()->AirControl = 0.35f;
GetCharacterMovement()->MaxWalkSpeed = 300.f;
GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

// NavUtilityLibrary.cpp — Hardcoded fallback speed
float Speed = 300.f; // Default speed if unknown

// DefaultPlayerState.cpp — String comparison for team names
if (TeamName == "Past" && PastTag.IsValid())
// Also in: InventoryComponent.cpp, DefaultGameMode.cpp, LobbyGameMode.cpp

// DronePawn.cpp — Hardcoded timer intervals
0.02f  // launch/mesh alignment timer interval

// DefaultPlayerController.cpp — Hardcoded timer intervals
1.0f   // ping update interval
0.1f   // alarm/pre-alarm UI update interval
```

**Recommendation:**
- Move constructor movement values to `UPROPERTY(EditDefaultsOnly)` or initialize them from the `AttributeInitGE` gameplay effect exclusively.
- Consider an `ETeamType` enum or `FGameplayTag`-based team comparison instead of raw `FString` comparisons scattered across 5+ files.
- Replace hardcoded timer intervals with `UPROPERTY(EditDefaultsOnly)` constants.

---

### 3. Duplicate Code Patterns 🔸 (Medium Priority — Remaining)

**Status:** Leave/loading logic has been extracted to base controller. Other duplications remain.

**Remaining duplications:**

a) **Ability granting inner loop** — `GrantAbilitiesFromInputSet()` in `DefaultCharacter.cpp` duplicates the for-loop body 3 times for `FutureGASet`, `SoloGASet`, and `SoloGASet` granting `FutureGASet` abilities:
```cpp
// Lines 212-246: Same loop body repeated 3 times
for (const FAbilityInputSetEntry& Entry : GASet->Abilities)
{
    if (!Entry.AbilityClass) continue;
    FGameplayAbilitySpec Spec(Entry.AbilityClass, Entry.AbilityLevel, 0);
    if (Entry.InputTag.IsValid())
        Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
    ASC->GiveAbility(Spec);
}
```

**Recommendation:** Extract a helper function:
```cpp
void GrantAbilitiesFromSet(UAbilitySystemComponent* ASC, const UAbilityInputSet* Set);
```

b) **Server time retrieval** — `UpdateAlarmUI()` and `UpdatePreAlarmUI()` have identical 8-line blocks for getting server time:
```cpp
float ServerNow = 0.f;
if (AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<AGameStateBase>() : nullptr)
    ServerNow = GS->GetServerWorldTimeSeconds();
else
    ServerNow = GetWorld()->GetTimeSeconds();
```

**Recommendation:** Extract to a one-line helper: `float GetSyncedServerTime() const;`

c) **Fully-detected StateTree event dispatch** — `AAICharacter::OnFullyDetected_Implementation()` has two near-identical code blocks for Pawn vs Actor detection (lines 231-256), differing only by tag:

**Recommendation:** Collapse into one block with a conditional tag selection.

---

### 4. Timer-Based Animation 🔸 (Medium Priority)

**Status:** Still present. `ADronePawn` uses fixed 0.02s timers for launch and mesh alignment.

**Issue:** The hardcoded 0.02s interval (50 FPS equivalent) means:
- On high-refresh displays (144Hz+), animation appears to stutter.
- On low-performance machines, the timer may fire at inconsistent intervals.
- The `LaunchAnimTimeElapsed += 0.02f` pattern doesn't account for actual elapsed time.

**Recommendation:** Use `FTimerManager::SetTimer` with a `DeltaTime`-aware callback, or switch to `UTimelineComponent` for smoother, frame-rate-independent animation.

---

### 5. Error Handling & Logging 🔸 (Medium Priority)

**Status:** Slightly improved (the `Cast<>` fix in `SetupPlayerInputComponent` now logs an error). Most other code paths still silently return.

**Key areas needing logging:**
```cpp
// ServerHandleInteract_Implementation — silent return on null target
if (!TargetActor) return;

// InitializeAbilitySystem — no log if PlayerState/ASC is null
ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
if (PS) { ... }  // Silent fail if PS is null

// Multiple RPC implementations — no validation logging
void ServerStartSprint_Implementation() { StartSprint(); }  // No log
```

**Recommendation:** At minimum, add `UE_LOG` at `Warning` level in:
1. Server RPC implementations when receiving unexpected null parameters
2. `InitializeAbilitySystem()` when ASC/PlayerState is unavailable
3. `StartAlarm()`/`StartPreAlarm()` state transitions
4. Define a custom log category: `DECLARE_LOG_CATEGORY_EXTERN(LogSplitstream, Log, All);`

---

### 6. RobotGuardCharacter BeginPlay 🔸 (Medium Priority)

**Status:** Still iterates all actors in the world.

```cpp
// RobotGuardCharacter.cpp line 19
UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
```

This returns **every actor** in the world (potentially thousands), then filters for `IRepairable`. This runs once at BeginPlay so it's not a per-frame concern, but it's still wasteful.

**Recommendation:** Create a `URepairableRegistry` subsystem (similar to the existing `UDetectorRegistry`) where `IRepairable` actors register themselves. This would make the lookup O(n) where n is the number of repairables, not all actors.

---

### 7. Build.cs Cleanup 🟡 (Low Priority)

**Status:** Still has duplicate module entries.

```csharp
// "UMG" in both Public and Private dependency arrays
PublicDependencyModuleNames.AddRange(new string[] { ..., "UMG", ... });
PrivateDependencyModuleNames.AddRange(new string[] { ..., "UMG", ... });
```

**Recommendation:** Remove `"UMG"` from `PrivateDependencyModuleNames` since it already exists in `PublicDependencyModuleNames`.

---

### 8. Potential Null Dereferences 🟡 (Low Priority)

**Status:** Still present but low-risk since components are created in constructors.

```cpp
// DronePawn.cpp BeginPlay — no null guard
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
    .AddUObject(this, &ADronePawn::OnHealthChanged);
```

Note: `AAICharacter::BeginPlay()` correctly guards with `if (AbilitySystemComponent && AttributeSet)`. `ADronePawn::BeginPlay()` does not have this guard.

**Recommendation:** Add the same null guard in `ADronePawn::BeginPlay()` for consistency:
```cpp
if (AbilitySystemComponent && AttributeSet)
{
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(...)
        .AddUObject(this, &ADronePawn::OnHealthChanged);
}
```

---

### 9. Detection Component Network Bandwidth 🔸 (Medium Priority — New)

**Issue:** `UDetectionComponent::TickComponent()` calls `DefaultPC->ClientUpdateDetectionWidget()` (a Client RPC) for **every detector** on **every player controller** each tick while detection is active. This is a `Client, Reliable` RPC, meaning every tick generates multiple reliable network packets.

```cpp
// DetectionComponent.cpp lines 207-220
for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
{
    if (ADefaultPlayerController* DefaultPC = Cast<ADefaultPlayerController>(PC))
    {
        DefaultPC->ClientUpdateDetectionWidget(Detector, ProgressPct, bIsLocked);
    }
}
```

**Impact:** With 4 players and 2 active detectors, this sends 8 reliable RPCs per tick (default ~60/sec = 480 RPCs/sec). This can saturate bandwidth and cause packet queuing.

**Recommendation:**
- Consider sending detection updates at a reduced rate (e.g., every 0.1s) rather than every tick.
- Use `Unreliable` multicast instead of per-player `Client, Reliable` RPCs for visual-only progress updates.
- Only send updates when progress actually changes meaningfully (e.g., ≥1% change).

---

### 10. SetRevealProgress Creates Material Instance Every Call 🟡 (Low Priority — New)

**Issue:** `ADronePawn::SetRevealProgress()` calls `CreateAndSetMaterialInstanceDynamic(0)` every time:

```cpp
void ADronePawn::SetRevealProgress(float RevealAlpha)
{
    if (DroneMesh)
    {
        UMaterialInstanceDynamic* DynMat = DroneMesh->CreateAndSetMaterialInstanceDynamic(0);
        // ...
    }
}
```

If called frequently, this creates a new material instance object each call, wasting memory.

**Recommendation:** Cache the `UMaterialInstanceDynamic*` and only create it once:
```cpp
if (!CachedDynMat)
    CachedDynMat = DroneMesh->CreateAndSetMaterialInstanceDynamic(0);
CachedDynMat->SetScalarParameterValue("RevealAlpha", RevealAlpha);
```

---

### 11. SecurityCamera PanUpdate Timer Precision 🟡 (Low Priority — New)

**Issue:** `ASecurityCamera::PanUpdate()` uses `PanInterval` (0.02s) as the delta time directly:

```cpp
const float DeltaYaw = PanSpeed * PanInterval * (bPanningRight ? 1.0f : -1.0f);
```

Timer callbacks don't guarantee exact intervals — the actual elapsed time may differ slightly from `PanInterval`, causing minor drift over time.

**Recommendation:** Track actual elapsed time between calls or accept the minor drift (acceptable for a camera pan).

---

## Ongoing Logic & Performance Analysis

### Pre-Alarm / Alarm System — Logic Review

The dual-era pre-alarm/alarm system is well-designed with proper separation:
- **Per-era state** via `FPerEraPreAlarmState` with soonest-instigator tracking
- **Proper escalation** from pre-alarm → alarm via `PreAlarmTimeoutPast()`/`PreAlarmTimeoutFuture()`
- **Team-filtered UI** — Only shows alarms to players in the matching era
- **Server-synced timers** — Uses `GetServerWorldTimeSeconds()` for consistent countdown display

**Minor concern:** `OnPreAlarmCanceled` is broadcast without era context, meaning both eras get the cancellation notification. Consider adding an era parameter to the delegate for more precise client-side handling.

### Detection System — Logic Review

The detection system flow is solid:
1. AI perceives player → `IDetectable::Execute_OnDetected()` → `UDetectionComponent::StartDetection()`
2. Distance-based fill rate in `TickComponent()` → `HandleFullyDetected()` → `IDetectable::Execute_OnFullyDetected()`
3. Player becomes legal (tag removed) → `OnIllegalTagChanged()` → forces `OnLost` on all active detectors via registry

**Strength:** The `UDetectorRegistry` subsystem is a clean, efficient pattern. The detection component's distance-based speed multiplier adds gameplay depth.

**Minor concern:** The detection component ticks on every frame while active and sends RPCs to all player controllers per tick (see issue #9 above).

### Inventory System — Logic Review

The inventory system is robust:
- Slot-based with per-instance effect/ability handles
- Proper cleanup on remove/drop (effects and abilities are removed)
- Team-aware dropping with `FGameplayTag` team identification
- Future item invalidation via static delegate

**Potential edge case:** `RemoveItem()` shifts items down but uses `ActiveSlotIndex == INDEX_NONE` after removal. If the active slot was the removed item, the UI should handle `INDEX_NONE` gracefully.

### Multiplayer Session Management — Logic Review

Session management is thorough:
- Double-destroy guard via `bSessionDestroyInProgress`
- Proper delegate cleanup in `CleanupOSSDelegates()`
- Net driver cleanup for leftover connections
- Graceful client kick with timer-delayed connection close for RPC flush

**Strength:** The `CleanupNetDriver()` function properly iterates and shuts down all active net drivers, which many UE5 projects forget to do.

---

## What's Done Well vs. What Needs Work

| Area | Status | Notes |
|---|---|---|
| Architecture | ✅ Excellent | Clean separation, interface-driven, subsystem usage |
| GAS Integration | ✅ Excellent | Correct ownership, per-item grants, attribute delegates |
| Multiplayer | ✅ Very Good | Server-authoritative, session cleanup, synced timers |
| Gameplay Depth | ✅ Excellent | Rich feature set, dual timeline, mini-games |
| AI System | ✅ Very Good | StateTree, progressive detection, repair queue |
| Code Organization | ✅ Very Good | Clear folder structure, base class hierarchy |
| Data-Driven Design | ✅ Very Good | DataAssets for items, abilities, input |
| Performance Patterns | ✅ Improved | DetectorRegistry, throttled traces, managed ticking |
| Code Documentation | 🔸 Needs Work | Most classes still lack doc comments |
| Error Handling | 🔸 Needs Work | Silent failures, no custom log category |
| Remaining Duplication | 🔸 Needs Work | Ability granting loops, server time helper |
| Magic Numbers | 🔸 Needs Work | Constructor values, timer intervals |
| Network Bandwidth | 🔸 Needs Work | Per-tick reliable RPCs in detection |
| Build Configuration | 🟡 Minor | Duplicate module reference |

---

## Grade: **8.0 / 10** (up from 7.5)

### Justification

**Splitstream has improved measurably since the last review**, addressing the two highest-priority issues (world-wide actor iteration and per-frame traces) and demonstrating strong software engineering practices:

**Points gained (+0.5):**
- **DetectorRegistry subsystem** (+0.2) — Eliminates the most expensive performance bottleneck with a clean, reusable pattern.
- **Throttled interact highlight** (+0.1) — Configurable interval instead of per-frame.
- **Safe cast with logging** (+0.1) — Prevents crashes and aids debugging.
- **Base controller extraction** (+0.1) — Reduces code duplication across controllers.

**Remaining deductions (-2.0):**
- **Code documentation** (-0.4) — Most classes still have no doc comments. This affects team velocity and Blueprint user experience.
- **Error handling/logging** (-0.3) — Silent failures in RPCs and initialization make multiplayer debugging difficult.
- **Remaining duplication** (-0.3) — Ability granting loops and server time retrieval are still duplicated.
- **Magic numbers/strings** (-0.3) — Hardcoded values in constructors and string-based team comparisons.
- **Network bandwidth** (-0.3) — Per-tick reliable RPCs in the detection system could cause issues with more players.
- **Minor issues** (-0.4) — Timer-based animation, Build.cs duplication, remaining null guards.

### Path to 9.0+

To reach a 9.0+ rating, focus on:
1. **Add Doxygen doc comments** to all public interfaces and key gameplay classes
2. **Define a custom log category** (`LogSplitstream`) and add logging to server RPCs and state transitions
3. **Extract helper functions** for duplicated ability granting and server time retrieval
4. **Throttle detection RPCs** to reduce network bandwidth (biggest remaining performance concern)
5. **Replace string team comparisons** with typed enum or gameplay tag comparisons

This is a strong **senior-level** UE5 project demonstrating real-world multiplayer game development practices. The fixes since the last review show a clear understanding of performance optimization principles and code quality improvement.
