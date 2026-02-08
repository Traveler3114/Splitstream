# Splitstream — Code Architecture Review

> **Date**: February 2026  
> **Scope**: All C++ source under `Source/Splitstream/` (~130 classes, ~275 files)  
> **Engine**: Unreal Engine 5.7

---

## Table of Contents

- [Executive Summary](#executive-summary)
- [Architecture Overview](#architecture-overview)
- [What Is Good](#what-is-good)
- [What Needs Improvement](#what-needs-improvement)
- [Detailed Findings](#detailed-findings)
  - [Readability](#readability)
  - [Scalability](#scalability)
  - [Networking & Replication](#networking--replication)
  - [Code Duplication](#code-duplication)
  - [Null Safety & Defensive Programming](#null-safety--defensive-programming)
  - [Performance Considerations](#performance-considerations)
- [Severity Summary Table](#severity-summary-table)
- [Recommendations Roadmap](#recommendations-roadmap)

---

## Executive Summary

**Overall verdict: This is a well-structured, above-average UE5 C++ codebase.** The architecture demonstrates strong knowledge of Unreal conventions, good use of GAS (Gameplay Ability System), clean interface-driven design, and solid multiplayer networking patterns. For a project of this scope (stealth-action game with timeline mechanics, multiplayer, AI, mini-games), the code organization is professional and maintainable.

**Rating: 7.5 / 10**

| Area | Rating | Notes |
|------|--------|-------|
| Architecture | ⭐⭐⭐⭐ | Clean module boundaries, good use of UE5 framework classes |
| Readability | ⭐⭐⭐½ | Good naming, but some large functions and inconsistent formatting |
| Scalability | ⭐⭐⭐ | Interface-driven design is solid, but some code duplication limits extensibility |
| Networking | ⭐⭐⭐⭐ | Correct authority checks, proper replication patterns |
| Safety | ⭐⭐⭐½ | Most null checks present, a few missing in critical paths |

---

## Architecture Overview

### Class Hierarchy (Strengths)

```
ACharacter
├── ADefaultCharacter (Player: IInteractable, IAbilitySystemInterface, IDetectable)
└── AAICharacter (Abstract base: IInteractable, IAbilitySystemInterface, IDetectable)
    ├── AGuardCharacter
    ├── ACivilianCharacter
    └── ARobotGuardCharacter

AActor
├── ADoorBase (Abstract: IInteractable, IUnlockable)
├── ADoubleDoorBase (Abstract: IInteractable, IUnlockable)
├── ATerminal (IRepairable, IInteractable, IServerActionInterface)
├── ASecurityCamera (IDetectable)
└── 50+ specialized actors

UActorComponent
├── UDetectionComponent
├── UInventoryComponent
├── UHackComponent
├── ULockPickComponent
├── USearchComponent
└── UProximityHackComponent
```

### Module Organization

```
Source/Splitstream/
├── AbilitySystem/         ← GAS: abilities, tasks, attributes, cues, tags
├── ActorComponents/       ← Reusable components (Detection, Inventory, Hack, etc.)
├── Actors/                ← World actors (doors, cameras, terminals, pickups, etc.)
│   ├── TimeObjects/       ← Past/Future timeline variants
│   ├── Lever/             ← Lever puzzle system
│   ├── Wire/              ← Wire puzzle system
│   ├── Laser/             ← Laser puzzle system
│   └── ...
├── Characters/            ← Player + AI characters
├── Controllers/           ← Player controllers
├── DataAssets/            ← Data-driven configuration
├── GameModes/             ← Game mode hierarchy
├── GameStates/            ← Game state hierarchy
├── Interfaces/            ← 8 core interfaces
├── Minigames/             ← Mini-game logic
├── Saving/                ← Save system
└── Widgets/               ← UI widgets
```

---

## What Is Good

### 1. ✅ Interface-Driven Design (Excellent)

The codebase uses 8 well-defined UE5 interfaces (`IInteractable`, `IDetectable`, `IRepairable`, `IUnlockable`, `IServerActionInterface`, `IGhostRevealable`, `IGhostMirrorSource`, `IPuzzleCompletionReceiver`) to enforce contracts across unrelated actor types. This is a **best practice** that allows any actor to participate in game systems without tight coupling.

```cpp
// Example: Any actor can be interactable — doors, terminals, items, characters
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface { ... };

class IInteractable {
    UFUNCTION(BlueprintNativeEvent) void Interact(AActor* Interactor);
    UFUNCTION(BlueprintNativeEvent) void CancelInteract(AActor* Interactor);
    UFUNCTION(BlueprintNativeEvent) void SetHighlighted(bool bHighlight);
    UFUNCTION(BlueprintNativeEvent) bool IsProgressiveInteract();
};
```

### 2. ✅ Component-Based Architecture (Very Good)

Gameplay behaviors are properly decomposed into reusable components:
- `UDetectionComponent` — progressive detection with distance-based speed
- `UInventoryComponent` — slot-based inventory with GAS integration
- `UHackComponent`, `USearchComponent`, `ULockPickComponent` — interaction mechanics

These components can be attached to any actor, following UE5's composition model.

### 3. ✅ Gameplay Ability System Usage (Very Good)

GAS is implemented correctly with:
- Native gameplay tags via `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG`
- Proper ASC initialization on `PossessedBy` / `OnRep_PlayerState`
- Data-driven ability sets (`UAbilityInputSet`, `UDefaultGASet`)
- Per-item effect/ability granting with tracked handles for cleanup
- Tag-based ability activation via input mapping

### 4. ✅ Naming Conventions (Good)

The codebase follows UE5 naming conventions consistently:
- `A` prefix for actors, `U` for UObjects/components
- `b` prefix for booleans (`bIsOpen`, `bIsSprinting`, `bAlarmActive`)
- `F` prefix for structs (`FDetectionState`, `FInventorySlot`, `FServerActionPayload`)
- `E` prefix for enums (`EItemType`, `ETimelineEra`, `ELockDifficulty`)
- `I` prefix for interfaces (`IInteractable`, `IDetectable`)
- Clear delegate naming (`FOnDetectionBegan`, `FOnRepairRequested`)

### 5. ✅ Networking Patterns (Good)

- Correct `HasAuthority()` guards on state mutations
- `DOREPLIFETIME` for replicated properties
- `ReplicatedUsing` with `OnRep_` callbacks
- Server RPCs (`UFUNCTION(Server, Reliable)`) for client-to-server actions
- Client RPCs for UI updates (`ClientUpdateDetectionWidget`, `ClientShowCalendarWidget`)
- Generic `IServerActionInterface` pattern for extensible client→server actions

### 6. ✅ Data-Driven Design (Good)

Using `UDataAsset` for configuration (`UItemBase`, `UInputMappingSet`, `UAbilityInputSet`, `UDefaultGASet`) allows designers to tweak gameplay without code changes.

### 7. ✅ Consistent `#pragma once` Usage

All headers use `#pragma once` consistently (no mixed header guard patterns).

### 8. ✅ Forward Declarations

Headers use forward declarations to minimize include dependencies:
```cpp
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
```

---

## What Needs Improvement

### 1. ⚠️ Code Duplication (Medium Severity)

**`ADoorBase` vs `ADoubleDoorBase`**: These two classes share ~90% identical code (Interact, CancelInteract, SetHighlighted, guard overlap logic, lock handling). The only difference is single vs. dual mesh.

**`ItemBase::OnDroppedWithTeam`**: Three nearly identical code blocks for Past/Future/Default pickup spawning — only the class type differs. This should use a template/helper method.

**Recommendation**: Extract shared door logic into a common base or utility, and refactor `OnDroppedWithTeam` to eliminate the triplicated spawn logic.

### 2. ⚠️ `ADefaultCharacter` is Too Large (Medium Severity)

At ~900 lines, `DefaultCharacter.cpp` handles:
- Movement (Move, Look, Jump, Sprint, Crouch)
- Input binding (SetupPlayerInputComponent)
- Ability system (InitializeAbilitySystem, GrantAbilities)
- Inventory management (UpdateEquippedItemMesh, DropActiveItem)
- Interaction system (HandleInteract*, UpdateInteractHighlight)
- Detection system events
- Camera/aim management
- Replication

**Recommendation**: Consider decomposing some responsibilities into dedicated components (e.g., an interaction handler component or a movement state component) to follow Single Responsibility Principle more closely.

### 3. ⚠️ Missing Null Checks (Medium Severity)

Several functions lack null checks on critical pointers:

```cpp
// GrantAbilitiesFromDefaultSet() — PS can be null
void ADefaultCharacter::GrantAbilitiesFromDefaultSet()
{
    if (!DefaultGASet) return;
    ADefaultPlayerState* PS = GetPlayerState<ADefaultPlayerState>();
    UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent(); // ← Crash if PS is null!
    ...
}
```

```cpp
// GrantAbilitiesFromInputSet() — SoloGASet can be null  
if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Team.Solo")))
{
    for (const FAbilityInputSetEntry& Entry : SoloGASet->Abilities) // ← Crash if SoloGASet is null!
```

### 4. ⚠️ Inconsistent Formatting (Low Severity)

Mixed indentation styles appear in several files:
- Most code uses spaces, but some files use tabs (e.g., `DoorBase.h` line 26, `AICharacter.h` line 90)
- Inconsistent brace placement in a few places
- Some commented-out code left in production files

### 5. ⚠️ Leftover Debug Comments (Low Severity)

Several files contain TODO-style comments or placeholder comments that should be cleaned up:
- `DoorBase.h:26` — `// Add this property` (leftover from development)
- `ItemBase.h:48` — `// In ItemBase.h, add:` (instruction comment left in)
- `ItemBase.cpp:33` — `// Pseudocode for ItemBase.cpp` (inaccurate comment)
- `DefaultCharacter.cpp:359-360` — Commented-out positioning code

### 6. ⚠️ `OnIllegalTagChanged` Uses TActorIterator (Performance)

`ADefaultCharacter::OnIllegalTagChanged` iterates **every actor in the world** when the illegal status tag changes. This is O(N) where N is all actors — could be expensive in large levels.

```cpp
for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr) { ... }
```

**Recommendation**: Maintain a registry of detectable actors or use a more targeted query (e.g., iterate only `AAICharacter` instances).

### 7. ⚠️ Missing `SPLITSTREAM_API` on `IDetectable` (Low Severity)

```cpp
class IDetectable  // ← Missing SPLITSTREAM_API
{
    GENERATED_BODY()
    ...
};
```

While this works for in-module usage, it prevents the interface from being properly exported if other modules need to use it.

### 8. ⚠️ `DoubleDoorBase::OnGuardOpenEndOverlap` Missing Check (Bug)

`DoorBase::OnGuardOpenEndOverlap` checks for both `AGuardCharacter` and `ACivilianCharacter` overlaps before closing, but `DoubleDoorBase::OnGuardOpenEndOverlap` only checks for `AGuardCharacter`, potentially closing the door on a civilian.

---

## Detailed Findings

### Readability

| Aspect | Assessment |
|--------|-----------|
| Function naming | ✅ Clear and descriptive (`HandleInteractHoldStart`, `UpdateEquippedItemMesh`) |
| Variable naming | ✅ Follows UE conventions (`bIsOpen`, `AbilitySystemComponent`) |
| Comment quality | ⚠️ Some leftover dev comments, but section headers are good (`// ---- MOVEMENT ----`) |
| File organization | ✅ Logical directory structure matching UE5 conventions |
| Function length | ⚠️ Some functions are too long (`DetectionUpdate` in SecurityCamera: 130+ lines, `DefaultCharacter.cpp`: 900+ lines) |
| Header includes | ✅ Good use of forward declarations to minimize dependencies |

### Scalability

| Aspect | Assessment |
|--------|-----------|
| Adding new interactable actors | ✅ Implement `IInteractable` — zero changes to existing code |
| Adding new item types | ✅ Create new `UItemBase` data asset — data-driven |
| Adding new abilities | ✅ Create new `UGameplayAbility`, add to data asset sets |
| Adding new door types | ⚠️ Must duplicate significant code from DoorBase/DoubleDoorBase |
| Adding new timeline eras | ⚠️ `ETimelineEra` enum + many hardcoded Past/Future checks would need updating |
| Adding new AI types | ✅ Extend `AAICharacter` abstract base — clean inheritance |
| Adding new detection sources | ✅ Implement `IDetectable` — event-driven system |
| Adding new puzzle types | ✅ `IPuzzleCompletionReceiver` + manager pattern is extensible |

### Networking & Replication

| Aspect | Assessment |
|--------|-----------|
| Authority checks | ✅ Consistently used (`HasAuthority()`) |
| Property replication | ✅ `DOREPLIFETIME` used correctly |
| RPC patterns | ✅ Server/Client/Multicast RPCs used appropriately |
| Session management | ✅ Clean destroy/cleanup flow in `DefaultGameInstance` |
| Net driver cleanup | ✅ Thorough cleanup in `CleanupNetDriver` |
| Bandwidth concerns | ⚠️ `DetectionComponent` sends widget updates to ALL player controllers every tick |

### Code Duplication

| Duplicated Code | Location | Lines Duplicated |
|-----------------|----------|-----------------|
| Door logic (Interact, Lock, Guard overlap) | `DoorBase` ↔ `DoubleDoorBase` | ~150 lines |
| Item drop/spawn logic | `ItemBase::OnDroppedWithTeam` | ~60 lines (3× repeated) |
| Detection handling | `DefaultCharacter` ↔ `AICharacter` | ~20 lines |
| Highlighted mesh logic | Multiple actors | ~5 lines per actor |

### Null Safety & Defensive Programming

| Issue | File | Line | Severity |
|-------|------|------|----------|
| No null check on `PS` before `PS->GetAbilitySystemComponent()` | `DefaultCharacter.cpp` | 187 | 🔴 High |
| No null check on `SoloGASet` before iterating | `DefaultCharacter.cpp` | 220 | 🔴 High |
| `HighlightedActor` used without validity check | `DefaultCharacter.cpp` | 553 | 🟡 Medium |
| `ArrowComp` used without null check in `ComputeOpenDirection` | `DoorBase.cpp` | 82 | 🟡 Medium |

### Performance Considerations

| Concern | Location | Impact |
|---------|----------|--------|
| `TActorIterator<AActor>` iterates all actors | `DefaultCharacter::OnIllegalTagChanged` | 🟡 Could be slow in large levels |
| Detection widget updates sent every tick to all PCs | `DetectionComponent::TickComponent` | 🟡 Bandwidth concern with many detectors |
| `UpdateInteractHighlight` runs every frame with line trace | `DefaultCharacter::Tick` | 🟢 Acceptable for single player character |
| `SecurityCamera::DetectionUpdate` does sphere overlap + line traces | `SecurityCamera.cpp` | 🟢 Timer-based (0.2s interval) — fine |

---

## Severity Summary Table

| ID | Severity | Category | Finding |
|----|----------|----------|---------|
| 1 | 🔴 High | Safety | Missing null check on `PS` in `GrantAbilitiesFromDefaultSet` |
| 2 | 🔴 High | Safety | Missing null check on `SoloGASet` in `GrantAbilitiesFromInputSet` |
| 3 | 🟡 Medium | Duplication | `DoorBase` / `DoubleDoorBase` share ~90% identical code |
| 4 | 🟡 Medium | Duplication | `ItemBase::OnDroppedWithTeam` has 3× identical spawn blocks |
| 5 | 🟡 Medium | Architecture | `DefaultCharacter.cpp` at 900+ lines — too many responsibilities |
| 6 | 🟡 Medium | Performance | `OnIllegalTagChanged` iterates all world actors |
| 7 | 🟡 Medium | Bug | `DoubleDoorBase::OnGuardOpenEndOverlap` missing civilian overlap check |
| 8 | 🟢 Low | Readability | Leftover dev comments (`// Add this property`, `// Pseudocode`) |
| 9 | 🟢 Low | Readability | Inconsistent tab/space indentation in some files |
| 10 | 🟢 Low | API | Missing `SPLITSTREAM_API` on `IDetectable` class |

---

## Recommendations Roadmap

### Immediate (Do Now)
1. **Fix null safety issues** (#1, #2) — potential crashes
2. **Fix `DoubleDoorBase` guard overlap bug** (#7) — incorrect behavior
3. **Clean up leftover comments** (#8) — low effort, high readability gain

### Short-Term (Next Sprint)
4. **Refactor `ItemBase::OnDroppedWithTeam`** — extract spawn helper to eliminate triplication
5. **Add `SPLITSTREAM_API`** to `IDetectable` for proper module export

### Medium-Term (Next Milestone)
6. **Extract common door logic** into a shared base class or utility to reduce DoorBase/DoubleDoorBase duplication
7. **Optimize `OnIllegalTagChanged`** — use a cached set of detectable actors instead of world iteration
8. **Consider decomposing `DefaultCharacter`** — move interaction handling into a component

### Long-Term (Architecture)
9. **Consider conditional replication** for detection widget updates (only send to relevant clients)
10. **Add code formatting standard** — enforce consistent indentation with `.clang-format`
