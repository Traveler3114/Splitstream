# Code Review

This document provides a comprehensive code review of the **Splitstream** codebase under `Source/Splitstream/`.

---

## Summary

Splitstream demonstrates a solid understanding of Unreal Engine 5 multiplayer game development. The project successfully integrates the Gameplay Ability System (GAS), a server-authoritative multiplayer architecture, custom AI systems, and multiple interactive puzzle mechanics. The codebase is well-organized with a clear directory structure and makes good use of UE5 patterns like interfaces, components, and data assets.

---

## Strengths

### 1. Architecture & Organization ✅
- **Clean directory structure** — Code is well-organized into logical folders (Characters, Controllers, GameModes, GameStates, Interfaces, etc.).
- **Interface-driven design** — `IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, `IGhostMirrorSource`, `IPuzzleCompletionReceiver`, and `IServerActionInterface` provide excellent decoupling.
- **Component-based reusability** — `UDetectionComponent`, `UHackComponent`, `USearchComponent`, `ULockPickComponent`, and `UInventoryComponent` are modular and reusable.
- **Data-driven design** — `UItemBase`, `UAbilityInputSet`, `UDefaultGASet`, `UInputMappingSet` allow designers to configure gameplay without C++ changes.

### 2. GAS Integration ✅
- **Correct ASC ownership** — ASC on `PlayerState` (not pawn) is the recommended pattern for multiplayer with pawn respawning.
- **Native gameplay tags** — All tags declared via `UE_DECLARE_GAMEPLAY_TAG_EXTERN` for compile-time safety.
- **Ability input binding** — Clean data-driven mapping from input actions to abilities via dynamic spec tags.
- **Attribute change delegates** — Properly bound to update movement speeds and HUD.
- **Gameplay Cue usage** — PastEcho uses Gameplay Cues correctly for visual-only client-side effects.

### 3. Multiplayer Architecture ✅
- **Server-authoritative** — Game logic runs on server; clients receive replicated state.
- **Proper replication** — Uses `ReplicatedUsing`, `DOREPLIFETIME`, Server/Client/NetMulticast RPCs appropriately.
- **Seamless travel** — `bUseSeamlessTravel` with `CopyProperties()` to persist player state across maps.
- **Session cleanup** — Robust session destruction with guard against double-destroy.

### 4. Gameplay Depth ✅
- **Dual-timeline system** — Creative game design with Past/Future eras.
- **Ghost mirroring** — Innovative cross-timeline awareness system.
- **Multiple puzzle types** — Wires, levers, keypads, lasers, locks, hacking.
- **Full mini-games** — Space invader and endless runner mini-games with custom rendering.
- **Procedural generation** — Randomized civilian names, codes, puzzle sequences, and dates.

### 5. AI System ✅
- **StateTree integration** — Modern UE5 AI behavior system.
- **Custom detection** — Distance-weighted progressive detection meter.
- **Drone cone detection** — Custom bounds-point-in-cone algorithm with LOS checks.
- **Repair system** — Robot guard queue-based repair with StateTree event dispatch.

---

## Areas for Improvement

### 1. Code Documentation 🔸 (Medium Priority)

**Issue:** Most classes lack header-level documentation comments. Only `UFutureGAPastEcho` and a few others have meaningful doc comments.

**Recommendation:** Add `/** ... */` Doxygen-compatible comments to all `UCLASS`, `UPROPERTY`, and `UFUNCTION` declarations. This is especially important for interfaces and public API methods.

```cpp
// Before:
UFUNCTION(BlueprintCallable, Category = "Detection")
void StartDetection(AActor* Detector);

// After:
/** 
 * Begins building detection progress for the given detector actor.
 * Detection fills at a rate modified by distance from the detector.
 * @param Detector The actor performing the detection (e.g., guard, camera).
 */
UFUNCTION(BlueprintCallable, Category = "Detection")
void StartDetection(AActor* Detector);
```

---

### 2. Magic Numbers & Hardcoded Strings 🔸 (Medium Priority)

**Issue:** Several hardcoded values throughout the codebase:

```cpp
// DefaultCharacter.cpp
GetCharacterMovement()->MaxWalkSpeed = 300.f;    // Magic number
GetCharacterMovement()->JumpZVelocity = 300.f;   // Magic number

// NavUtilityLibrary.cpp
float Speed = 300.f; // Default speed if unknown

// DefaultGameMode.cpp
FString LobbyURL = TEXT("/Game/Maps/LobbyMap");   // Hardcoded path

// DefaultPlayerState.cpp — String comparison for team
if (TeamName == "Past" && PastTag.IsValid())       // Hardcoded string
```

**Recommendation:**
- Move magic numbers to `UPROPERTY` constants or `constexpr` values.
- Use `FName` or `FGameplayTag` instead of raw string comparisons for team names.
- Consider an `ETeamType` enum instead of `FString TeamName`.

---

### 3. Tick Performance 🔴 (High Priority)

**Issue:** `ADefaultCharacter::Tick()` runs `UpdateInteractHighlight()` every frame for locally controlled characters, performing a line trace each tick.

```cpp
void ADefaultCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (IsLocallyControlled())
    {
        UpdateInteractHighlight();  // Line trace every frame
    }
}
```

**Recommendation:** Reduce frequency using a timer or frame-skip counter (e.g., every 3-5 frames). Line traces are relatively cheap, but this is a good practice for multiplayer performance.

---

### 4. TActorIterator in Tag Change Handler 🔴 (High Priority)

**Issue:** `ADefaultCharacter::OnIllegalTagChanged()` iterates **all actors in the world** twice when the illegal tag changes:

```cpp
for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
{
    if (ActorItr->GetClass()->ImplementsInterface(UDetectable::StaticClass()))
    {
        // ...
    }
}
```

**Recommendation:** Maintain a cached list of detectable actors (registered on BeginPlay, unregistered on EndPlay) or use a subsystem to track them. This avoids full-world iteration which scales poorly with actor count.

---

### 5. `CastChecked` Usage 🔸 (Medium Priority)

**Issue:** `CastChecked<UEnhancedInputComponent>` in `SetupPlayerInputComponent()` will crash if the cast fails.

```cpp
if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
```

**Recommendation:** Use `Cast<>` with null check instead, or ensure via project settings that `EnhancedInputComponent` is always used. In a shipped game, `CastChecked` crashes are unrecoverable.

---

### 6. Duplicate Code Patterns 🔸 (Medium Priority)

**Issue:** Several patterns are duplicated across classes:

- **Leave-to-main-menu logic** — Nearly identical in `ADefaultPlayerController::RequestLeaveToMainMenu()` and `ALobbyPlayerController::RequestLeaveToMainMenu()`.
- **Loading screen display** — `ClientShowLoadingScreen()` is implemented identically in both `ADefaultPlayerController` and `ALobbyPlayerController`.
- **Ability granting** — `GrantAbilitiesFromInputSet()` duplicates the inner loop for `FutureGASet` when granting to `SoloGASet` players.
- **Server time retrieval** — `UpdateAlarmUI()` and `UpdatePreAlarmUI()` have identical server-time-fetch code.

**Recommendation:** Extract shared logic into `ABaseGameMode` / base controller or utility functions.

---

### 7. Timer-Based Animation 🔸 (Medium Priority)

**Issue:** `ADronePawn` uses a fixed 0.02s timer for launch and mesh alignment animations instead of Tick or Timelines:

```cpp
GetWorld()->GetTimerManager().SetTimer(LaunchMoveTimerHandle, this, &ADronePawn::MoveUpForLaunch, 0.02f, true);
```

**Recommendation:** Use `UTimelineComponent` for smoother, frame-rate-independent animation, or use `Tick()` with `DeltaTime`. The 0.02s hardcoded interval means animation speed varies with timer precision.

---

### 8. Error Handling & Logging 🔸 (Medium Priority)

**Issue:** Most functions silently return on null checks without logging. This makes debugging difficult:

```cpp
if (!TargetActor) return;  // Silent fail
```

Only `ServerExecuteAction_Implementation` has proper logging:
```cpp
UE_LOG(LogTemp, Warning, TEXT("ServerExecuteAction_Implementation called!"));
```

**Recommendation:** Add `UE_LOG` statements (at `Warning` or `Error` level) for unexpected null cases, especially in server RPCs and game-critical paths. Define a custom log category:
```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogSplitstream, Log, All);
```

---

### 9. Memory Management 🟡 (Low Priority)

**Issue:** Raw `UObject*` pointers used for widget instances without `UPROPERTY()` in some cases, risking garbage collection.

Most widget references are correctly marked with `UPROPERTY()`, but some local/temporary pointers in functions could be at risk if the function yields (e.g., async operations).

**Recommendation:** Ensure all `UObject*` stored beyond a single function scope are marked `UPROPERTY()`. Use `TWeakObjectPtr<>` for non-owning references.

---

### 10. Build.cs Cleanup 🟡 (Low Priority)

**Issue:** `Splitstream.Build.cs` has duplicate entries:

```csharp
// "UMG" appears in both PublicDependencyModuleNames and PrivateDependencyModuleNames
// "OnlineSubsystem" appears twice in PrivateDependencyModuleNames
PrivateDependencyModuleNames.Add("OnlineSubsystem");  // duplicate
```

**Recommendation:** Remove duplicate module references. Move modules to the appropriate public/private section based on actual usage.

---

### 11. Inconsistent Naming Conventions 🟡 (Low Priority)

**Issue:** Some naming inconsistencies:
- Interface files use `I` prefix (`IInteractable`) but also have `U` prefix classes (`UInteractable`).
- Some `.cpp` files have empty implementations (e.g., `TimelineEra.cpp`, `IGhostMirrorSource.cpp`).
- Mix of `Default` prefix (`DefaultCharacter`, `DefaultGameMode`) and specific names (`GuardCharacter`, `LobbyGameMode`).

**Recommendation:** The `Default` prefix is acceptable as it indicates the project's base implementations. Empty `.cpp` files for UINTERFACE and UENUM are required by UHT (Unreal Header Tool) and are correct. The naming is generally consistent.

---

### 12. Potential Null Dereferences 🔸 (Medium Priority)

**Issue:** Some code paths don't guard against null:

```cpp
// AICharacter.cpp line 63 — No null check on AbilitySystemComponent
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
    .AddUObject(this, &AAICharacter::OnHealthChanged);

// DronePawn.cpp line 306 — Same pattern
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
    .AddUObject(this, &ADronePawn::OnHealthChanged);
```

These are called in `BeginPlay()` where the components should always exist (they're created in the constructor), but defensive coding is recommended for robustness.

---

### 13. RobotGuardCharacter BeginPlay 🔸 (Medium Priority)

**Issue:** `ARobotGuardCharacter::BeginPlay()` calls `UGameplayStatics::GetAllActorsOfClass(AActor::StaticClass())` which returns **every actor** in the world, then filters for `IRepairable`:

```cpp
TArray<AActor*> FoundActors;
UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
```

**Recommendation:** Use `TActorIterator` with a more specific base class, or maintain a registry of repairables (e.g., a subsystem or static list). Getting all actors is expensive and includes irrelevant actors.

---

## What's Done Well vs. What Needs Work

| Area | Status | Notes |
|---|---|---|
| Architecture | ✅ Excellent | Clean separation, interface-driven |
| GAS Integration | ✅ Excellent | Correct ownership, proper tag usage |
| Multiplayer | ✅ Very Good | Server-authoritative, proper replication |
| Gameplay Depth | ✅ Excellent | Rich feature set, creative design |
| AI System | ✅ Good | StateTree + custom detection |
| Code Organization | ✅ Very Good | Clear folder structure |
| Data-Driven Design | ✅ Very Good | DataAssets for items, abilities, input |
| Code Documentation | 🔸 Needs Work | Missing class/function docs |
| Performance Patterns | 🔸 Needs Work | World iteration, per-frame traces |
| Error Handling | 🔸 Needs Work | Silent failures, no custom log category |
| Code Duplication | 🔸 Needs Work | Leave/loading logic repeated |
| Magic Numbers | 🔸 Needs Work | Hardcoded values scattered |
| Build Configuration | 🟡 Minor | Duplicate module references |

---

## Grade: **7.5 / 10**

### Justification

**Splitstream is a well-architected, feature-rich Unreal Engine 5 multiplayer game** that demonstrates strong understanding of:
- UE5 multiplayer patterns (replication, RPCs, seamless travel)
- Gameplay Ability System integration
- Interface-driven extensibility
- Component-based architecture
- Data-driven design

The codebase loses points primarily for:
- **Lack of code documentation** (-0.5) — Most classes have no doc comments.
- **Performance concerns** (-0.5) — World-wide actor iteration in gameplay-critical code paths.
- **Code duplication** (-0.5) — Several patterns repeated across controllers and game modes.
- **Hardcoded values** (-0.5) — Magic numbers and string comparisons instead of typed constants.
- **Error handling** (-0.5) — Silent failures make debugging difficult in multiplayer scenarios.

This is a solid **mid-to-senior level** UE5 project. With documentation improvements, performance optimization for the actor iteration patterns, and extraction of shared logic into base classes/utilities, this codebase could easily reach an **8.5-9.0** rating.
