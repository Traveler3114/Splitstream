# Ability System (GAS) Integration

This document details how Splitstream integrates Epic's **Gameplay Ability System** (GAS).

---

## 1. Ability System Component Setup

### Player Characters

The `UAbilitySystemComponent` (ASC) lives on `ADefaultPlayerState`, not on the character pawn. This is the recommended pattern for multiplayer games with respawning/repossession:

```cpp
// DefaultPlayerState constructor
AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
AbilitySystemComponent->SetIsReplicated(true);
AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
```

**Initialization flow:**
1. `ADefaultCharacter::PossessedBy()` → calls `InitializeAbilitySystem()`.
2. `InitializeAbilitySystem()`:
   - Gets the ASC from the player state.
   - Calls `ASC->InitAbilityActorInfo(PlayerState, Character)`.
   - Registers attribute change delegates (walk speed, run speed, crouch speed).
   - Registers gameplay tag event for `Character.Status.Illegal`.
   - Applies `AttributeInitGE` (initial attribute values).
3. On clients, `OnRep_PlayerState()` triggers re-initialization.

### AI Characters

AI characters (`AAICharacter`, `ADronePawn`) own their ASC directly:

```cpp
AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));
```

---

## 2. Attribute Set

### `UPlayerAttributeSet` (AttributeSets/PlayerAttributeSet.h)

| Attribute | Type | Description |
|---|---|---|
| `Health` | `FGameplayAttributeData` | Character health; death at ≤ 0 |
| `WalkSpeed` | `FGameplayAttributeData` | Normal movement speed |
| `RunSpeed` | `FGameplayAttributeData` | Sprint speed |
| `CrouchSpeed` | `FGameplayAttributeData` | Crouch movement speed |

All attributes are replicated via `ReplicatedUsing` and use the `ATTRIBUTE_ACCESSORS` macro for standard getters/setters.

**Callbacks:**
- `PreAttributeChange()` — Validates/clamps attribute values before changes.
- `PostGameplayEffectExecute()` — Post-processing after gameplay effects modify attributes.
- `OnRep_*()` — Client-side replication callbacks.

**Speed integration:**
When attributes change, `ADefaultCharacter` updates `CharacterMovementComponent::MaxWalkSpeed`:
- `OnWalkSpeedChanged()` — Updates walk speed when not sprinting/crouching.
- `OnRunSpeedChanged()` — Updates speed during sprint.
- `OnCrouchSpeedChanged()` — Updates speed while crouching and `MaxWalkSpeedCrouched`.

---

## 3. Gameplay Tags

### Declared Tags (`SplitstreamGameplayTags.h`)

All native gameplay tags are declared using `UE_DECLARE_GAMEPLAY_TAG_EXTERN` and defined in `SplitstreamGameplayTags.cpp`:

#### Character Tags
| Tag | Purpose |
|---|---|
| `Character.Status.Block.Movement` | Blocks player movement input |
| `Character.Status.Block.Look` | Blocks player look input |
| `Character.Status.PastEcho` | PastEcho ability active |
| `Character.Status.LockPicking` | Lock picking in progress |
| `Character.Status.Hacking` | Hacking in progress |
| `Character.Status.ProximityHacking` | Proximity hack in range |
| `Character.Status.Searching` | Searching in progress |
| `Character.Status.Aiming` | Weapon aiming |
| `Character.Status.Firing` | Weapon firing |
| `Character.Status.Illegal` | Player is in illegal status (triggers detection) |
| `Character.Status.Illegal.Action` | Illegal due to action |
| `Character.Status.Illegal.Area` | Illegal due to area |
| `Character.Status.Teleporting` | Teleportation in progress |

#### Ability Input Tags
| Tag | Purpose |
|---|---|
| `Character.Ability.Future.PastEcho` | PastEcho ability input |
| `Character.Ability.LockPick` | Lock pick ability input |
| `Character.Ability.Hack` | Hack ability input |
| `Character.Ability.Search` | Search ability input |
| `Character.Ability.Solo.Teleport` | Solo teleport ability input |

#### Weapon Tags
| Tag | Purpose |
|---|---|
| `Weapon.Ability.Pistol.Fire` | Pistol fire |
| `Weapon.Ability.Pistol.Aim` | Pistol aim |

#### Gameplay Cue Tags
| Tag | Purpose |
|---|---|
| `GameplayCue.PastEcho.Activated` | Visual activation of PastEcho |
| `GameplayCue.PastEcho.Deactivated` | Visual deactivation of PastEcho |

#### StateTree Event Tags
| Tag | Purpose |
|---|---|
| `StateTree.Event.FullyDetected.Pawn` | AI fully detected a pawn |
| `StateTree.Event.FullyDetected.Actor` | AI fully detected a non-pawn actor |
| `StateTree.Event.RepairNeeded` | Robot guard dispatched for repair |

#### Guard Tags
| Tag | Purpose |
|---|---|
| `Guard.Status.Repair` | Guard is currently repairing |

---

## 4. Abilities

### Ability Granting

Abilities are granted via data-driven sets:

**`UAbilityInputSet`** — Maps abilities to input tags:
```cpp
struct FAbilityInputSetEntry
{
    TSubclassOf<UGameplayAbility> AbilityClass;
    int32 AbilityLevel = 1;
    FGameplayTag InputTag;
};
```

**`UDefaultGASet`** — Simple array of abilities to grant without input binding:
```cpp
TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
```

**Grant flow in `ADefaultCharacter`:**
1. `PossessedBy()` → `GrantAbilitiesFromInputSet()` + `GrantAbilitiesFromDefaultSet()`.
2. `GrantAbilitiesFromInputSet()` checks team tags to grant team-specific abilities:
   - `FutureGASet` → granted to `Team.Future` players.
   - `SoloGASet` → granted to `Team.Solo` players (gets both Solo + Future abilities).
3. Input tags are added to each `FGameplayAbilitySpec` via `GetDynamicSpecSourceTags()`.

### Ability Input Binding

`ADefaultCharacter::SetupPlayerInputComponent()` binds input actions from `UInputMappingSet`:
```cpp
for (const FInputActionTagMapping& MapEntry : InputMappingSet->Mappings)
{
    EnhancedInputComponent->BindAction(MapEntry.InputAction, ETriggerEvent::Started,
        this, &ADefaultCharacter::HandleAbilityInput, MapEntry.InputTag);
    EnhancedInputComponent->BindAction(MapEntry.InputAction, ETriggerEvent::Completed,
        this, &ADefaultCharacter::HandleAbilityInputReleased, MapEntry.InputTag);
}
```

`HandleAbilityInput()` finds specs with matching dynamic tags and calls `TryActivateAbility()`.
`HandleAbilityInputReleased()` cancels active abilities with the matching tag.

### Concrete Abilities

#### `UDefaultGAHack`
- Activates hacking on the nearest `UHackComponent`.
- Creates `UHackAbilityTask` for async progress tracking.
- Shows `UHackWidget` for UI feedback.
- Adds `Character.Status.Hacking` tag during execution.

#### `UDefaultGALockPick`
- Activates lock picking on the nearest `ULockPickComponent`.
- Creates `ULockPickAbilityTask` for async pin interaction.
- Shows `ULockPickWidget` for UI feedback.
- Adds `Character.Status.LockPicking` tag during execution.

#### `UDefaultGASearch`
- Activates searching on the nearest `USearchComponent`.
- Creates `USearchAbilityTask` for async progress tracking.
- Shows `USearchWidget` for UI feedback.
- Adds `Character.Status.Searching` tag during execution.

#### `UFutureGAPastEcho`
- **Toggle ability** for Future team players.
- On first activation: adds `Character.Status.PastEcho` tag, executes `GameplayCue.PastEcho.Activated`.
- On second activation: removes tag, executes `GameplayCue.PastEcho.Deactivated`.

#### `USoloGATeleport`
- Solo-mode teleportation ability.
- Adds `Character.Status.Teleporting` tag during execution.

### Ability Tasks

Custom `UAbilityTask` subclasses for progressive interactions:
- `UHackAbilityTask` — Monitors `UHackComponent` progress.
- `ULockPickAbilityTask` — Manages lock picking pin interaction.
- `USearchAbilityTask` — Monitors `USearchComponent` progress.

---

## 5. Gameplay Cues

### `UGCN_PastEchoActivated`
- Triggered by `GameplayCue.PastEcho.Activated`.
- Finds all `AGhostCharacterActor` instances and calls `SetIsPastEchoAbilityActive(true)`.
- Makes ghost representations visible to the activating player.

### `UGCN_PastEchoDeactivated`
- Triggered by `GameplayCue.PastEcho.Deactivated`.
- Calls `SetIsPastEchoAbilityActive(false)` on all ghosts.
- Hides ghost representations.

---

## 6. Gameplay Effects

### Attribute Initialization
Each character type has an `AttributeInitGE` property — a `TSubclassOf<UGameplayEffect>` applied on spawn to set initial attribute values.

### Illegal Area Effects
`AIllegalArea` applies a gameplay effect that grants `Character.Status.Illegal.Area` when players enter the trigger volume. The effect is removed when the player exits.

### Item Passive Effects
Items (`UItemBase`) can define `GrantedGameplayEffects` — effects applied while the item is in inventory. These are tracked per-instance via `FInventorySlot::GrantedGameplayEffectHandles` and removed when the item is dropped or consumed.

---

## 7. Team System Integration

Team assignment flows through GAS:
1. `ADefaultPlayerState::TeamName` is replicated.
2. `UpdateTeamGameplayTag()` adds/removes loose gameplay tags: `Team.Past`, `Team.Future`, `Team.Solo`.
3. Abilities are granted based on team tags.
4. Alarm system filters by team tag to show era-specific alarms.
5. `ADefaultGameMode::ChoosePlayerStart_Implementation()` uses team tags to select spawn points (`PlayerStart_Past` / `PlayerStart_Future`).
