# Project context

## Stack
- **Engine**: Unreal Engine 5.7
- **Language**: C++ (C++17-style, no trailing return types)
- **UI**: UMG / Slate
- **Ability System**: Gameplay Ability System (GAS) � GameplayAbilities, GameplayTags, GameplayTasks
- **Input**: Enhanced Input (EnhancedInput module, UEnhancedPlayerInput, UEnhancedInputComponent)
- **AI**: StateTree (StateTreeModule, GameplayStateTreeModule) + AI Perception (AIModule)
- **VFX**: Niagara
- **Networking**: Listen-server, SteamSockets, OnlineSubsystemEOS, OnlineSubsystemSteam
- **Rendering**: Forward Shading, DX12/DX11, no Lumen/Nanite/RayTracing
- **Session management**: AdvancedSessions / AdvancedSteamSessions plugins
- **Loading screen**: CommonLoadingScreen plugin (Lyra-style)

## Project structure

```
Splitstream/
+-- Config/                  # UE5 config (DefaultEngine.ini, DefaultGame.ini, etc.)
+-- Content/                 # Assets (maps, meshes, textures, blueprints, data assets)
�   +-- DataAssets/          # Data asset instances (ability sets, widget data, map list)
�   +-- Maps/                # Level maps (Bank, PolygonBank, MainMenu, Lobby, Transition)
�   +-- Meshes/              # Static meshes
�   +-- Movies/              # Video assets
�   +-- Textures/            # Textures
+-- docs/                    # Project documentation (architecture, systems, code review, etc.)
+-- Plugins/                 # External plugins (AdvancedSessions, CommonLoadingScreen)
+-- Source/Splitstream/      # All C++ source
�   +-- AbilitySystem/       # GAS: abilities, tasks, attribute sets, gameplay cues, tags
�   �   +-- Abilities/       # Concrete GameplayAbility classes
�   �   �   +-- Weapons/     # Weapon-specific abilities
�   �   +-- AbilityTasks/    # Async ability tasks (hack, search)
�   �   +-- AttributeSets/   # PlayerAttributeSet (Health, WalkSpeed, RunSpeed, CrouchSpeed)
�   �   +-- GameplayCues/    # GameplayCue notifiers (PastEcho)
�   �   +-- SplitstreamGameplayTags.h/.cpp  # All gameplay tag declarations
�   +-- ActorComponents/     # Reusable actor components (Interaction, Detection, Hack, etc.)
�   +-- Actors/              # World-placed actors (cameras, doors, items, terminals, vents, etc.)
�   +-- AI/                  # AI characters, controllers, StateTree tasks
�   +-- Controllers/         # (legacy � logic moved to Player/)
�   +-- DataAssets/          # Data asset definitions (ItemBase, InputMappingSet, AbilitySets)
�   �   +-- AbilitySets/     # AbilityInputSet, DefaultGASet
�   �   +-- WidgetData/      # MenuTabsData, SettingsTabsData, MapListData
�   +-- GameModes/           # Server-authoritative game modes (Base, Default, Lobby)
�   +-- GameStates/          # Replicated game states (Base, Default, Lobby)
�   +-- Interfaces/          # UE C++ interfaces (IInteractable, IDetectable, IUnlockable, etc.)
�   +-- Minigames/           # Standalone mini-game logic
�   +-- Player/              # Player characters, controllers, and state
�   �   +-- Characters/      # DefaultCharacter (main player pawn)
�   �   +-- Controllers/     # Base, Default, Lobby, MainMenu player controllers
�   +-- Saving/              # Save game classes (UUserSettingsSaveGame)
�   +-- Subsystems/          # World subsystems (DetectorRegistry, RepairableRegistry)
�   +-- Widgets/             # UMG widget classes (HUD, menus, lobby, mini-games)
+-- Splitstream.uproject     # Project descriptor
+-- Splitstream.sln          # Visual Studio solution
+-- Source/Splitstream.Target.cs / Editor.Target.cs  # Build targets
```

## Conventions - Asset Naming

### Content asset prefixes
| Prefix | Type | Convention | Examples |
|--------|------|-----------|---------|
| `BP_` | Blueprint class | `BP_{ClassName}` | `BP_DefaultCharacter`, `BP_LobbyGameState` |
| `WBP_` | Widget Blueprint | `WBP_{Screen}{Feature}Widget` | `WBP_PauseMenuWidget`, `WBP_DetectionWidget` |
| `DA_` | Data Asset | `DA_{Feature}{Variant}` | `DA_Pistol`, `DA_KeycardL1`, `DA_MapList` |
| `IMC_` | Input Mapping Context | `IMC_{Feature}` | `IMC_Default`, `IMC_FirewallMinigame` |
| `IA_` | Input Action | `IA_{ActionName}` | `IA_Interact`, `IA_Jump` |
| `SM_` | Static Mesh | `SM_{Feature}` | `SM_Vent`, `SM_Terminal`, `SM_Cup` |
| `SK_` | Skeletal Mesh | `SK_{Type}_{Variant}` | `SK_Wep_Pistol_01` |
| `M_` | Material | `M_{Feature}` | `M_Ghost`, `M_Outline`, `M_GarageDoor` |
| `MI_` | Material Instance | `MI_{Variant}` | `MI_FutureVan`, `MI_Teleporter` |
| `T_` | Texture (2D) | `T_{Feature}` | `T_Terminal`, `T_KeycardScanner`, `T_Denied` |
| `NS_` | Niagara System | `NS_{Feature}` | `NS_LaserBeam`, `NS_Rain` |
| `ABP_` | Animation Blueprint | `ABP_{Character}` | `ABP_DefaultCharacter` |
| `GCN_` | Gameplay Cue Notifier | `GCN_{Feature}{State}` | `GCN_PastEchoActivated`, `GCN_PastEchoDeactivated` |
| `STTask_` | StateTree Task | `STTask_{Action}` | `STTask_WalkAround`, `STTask_Interact` |

### File and folder naming
- Top-level Content/ subdirectories: **plural PascalCase** (`Animations`, `Blueprints`, `DataAssets`, `Materials`, `Meshes`, `Textures`, `Sounds`, `Maps`)
- Per-actor subdirectories: **PascalCase** (`Camera/`, `Door/`, `Lever/`, `Locker/`, `Terminal/`, `Van/`)
- Widget subdirectories: **screen/context PascalCase** (`HUD/`, `MainMenu/`, `Lobby/`, `Settings/`, `Minigames/`)
- Map files: **PascalCase without prefix** (`MainMenuMap.umap`, `FutureBank.umap`, `PersistentBank.umap`)
- Data Assets inside DataAssets/: grouped in subdirectories by function (`Items/`, `AbilitySets/`, `WidgetData/`)
- Animation assets: directional suffix (`_Fwd`, `_Bwd`, `_Left`, `_Right`, `_FR_45`, `_BL_45`), weapon prefix (`MF_Pistol_`, `MF_Unarmed_`)

## Conventions - Content Directory

The Content/ tree follows consistent organization patterns:

```
Content/
+-- Animations/              # Animation Blueprints, sequences, blend spaces
|   +-- {Character}/         # Per-character animation sets
+-- Assets/                  # Marketplace/imported meshes (organized by pack)
+-- Blueprints/              # Blueprint subclasses organized by C++ category
|   +-- Actors/              # BP subclasses of C++ actors
|   +-- AI/                  # AI Blueprints
|   +-- AbilitySystem/       # GameplayCue BPs, ability BPs
|   +-- GameModes/           # GameMode/GameState BPs
|   +-- GameStates/
|   +-- Player/Characters/   # Player character BPs
|   +-- Widgets/             # Widget Blueprints (WBP_)
|       +-- HUD/             # In-game HUD widgets
|       +-- MainMenu/        # Main menu screens
|       +-- Lobby/           # Lobby UI
|       +-- Settings/        # Settings screens
|       +-- Minigames/       # Mini-game widgets
+-- DataAssets/              # Data asset instances
|   +-- Items/               # Item definitions (DA_KeycardL1, DA_Pistol, etc.)
|   +-- AbilitySets/         # Ability set data
|   +-- WidgetData/          # Menu/settings/map list config
+-- Input/                   # Input Mapping Contexts + Actions
|   +-- Actions/             # Individual input action assets
|   +-- IMC_*.uasset         # Input mapping contexts
+-- Maps/                    # Level maps
+-- Materials/               # Material instances (per-actor subdirectories)
+-- Meshes/                  # Static meshes (per-actor subdirectories)
+-- Textures/                # Textures (per-actor subdirectories + Icons/, MapImages/)
+-- Niagara/                 # Niagara VFX systems
+-- Sounds/                  # Audio assets
```

## Conventions - Code Style



### Namespaces
No custom namespaces. UE type prefixes provide categorization implicitly:
| Prefix | Category | Example |
|--------|----------|---------|
| U | UObject subclass | UInteractionComponent, UDefaultAbilitySystemComponent |
| A | AActor subclass | ADefaultCharacter, ADefaultGameMode |
| F | Struct or non-UObject class | FInventorySlot, FDetectionState |
| I | Interface | IInteractable, IDetectable, IUnlockable |
| S | StateTree struct | FSTTask_WalkAround |

### Naming
| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase with UE prefix | ADefaultCharacter, UInteractionComponent |
| Methods | PascalCase | BeginPlay(), HandleInstantInteract() |
| Member variables | PascalCase (no prefix) | InteractDistance, HighlightedActor |
| Bool members | -prefix PascalCase | IsHoldingInteract, AlarmActive, DropPreviewIsValid |
| Properties (UPROPERTY) | PascalCase, inline init | UPROPERTY() float InteractDistance = 150.f; |
| Local variables | PascalCase | FHitResult Hit, FVector Start, AActor* HitActor |
| Parameters | PascalCase | AActor* Interactor, loat Duration |
| Function params | PascalCase (bool: b-prefix) | ool bHighlight, FVector& OutHit |
| Gameplay tags | TAG_Prefix_SubCategory_Detail | TAG_Character_Status_Illegal, TAG_AI_Event_FullyDetected_Pawn |
| Enums | PascalCase, optional E prefix | ETimelineEra, EItemType |
| Delegates | FOn prefix | FOnInventoryChanged, FOnDetectionBegan |

### Include order
**.cpp files:**
1. Primary header (the matching .h)
2. Project-internal headers ("ActorComponents/InventoryComponent.h")
3. Engine/UE headers ("GameFramework/Pawn.h", "Camera/CameraComponent.h")
4. Generated header last (already in the primary .h)

**.h files:**
1. #pragma once
2. #include "CoreMinimal.h" (as needed)
3. Engine/UE type includes
4. Project includes
5. #include "Foo.generated.h" (always last)

Forward declarations for class references in headers to minimize includes.

### Null checks
```cpp
// Good
if (!TargetActor) return;
if (!IsValid(Detector)) return;  // checks both null AND pending-kill
if (!WorldContext || !Repairable) return;

// Bad
if (IsValid(Detector) == false) return;  // overly verbose
```

### String handling
- FString � mutable strings, dynamic content
- FText � display text (localizable)
- FName � names, tags, socket names, action names
- FGameplayTag � semantic tags (strongly preferred over string comparisons)
- TEXT() macro for string literals: TEXT("/Game/Maps/MainMenuMap")

### Collection usage
| Collection | Use for |
|-----------|--------|
| TArray | Primary sequence container |
| TMap | Key-value lookups |
| TSet | Unique elements (registries) |
| TArray<TWeakObjectPtr<AActor>> | Safe actor references with auto-cleanup |
| TSubclassOf<> | Blueprint-visible class references |
| TSoftObjectPtr | Soft references to assets (map list) |
| TObjectPtr | UObject pointers (UE5 standard) |

### Constructor patterns
```cpp
ADefaultPlayerState::ADefaultPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UDefaultAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
}
```
- Public constructors (called by UE reflection system)
- Inline member initialization in headers
- PrimaryActorTick.bCanEverTick = false; by default
- Minimal constructor body � logic in BeginPlay() or Initialize()

### Comment style
```cpp
/**
 * Interface for actors that can be interacted with by players.
 * Supports instant and progressive (hold) interactions, item-gating, and highlighting.
 * @param Interactor The actor initiating the interaction.
 */
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
void Interact(AActor* Interactor);
```
- Doxygen /** */ on interfaces and key classes
- Inline /** */ above struct fields
- Section comments: // ---- ALARM STATE ----
- // Fill out your copyright notice � auto-generated, can be replaced

### Other style rules
- #pragma once (never #ifndef guards)
- GENERATED_BODY() in every UCLASS/USTRUCT/UENUM/UINTERFACE
- UPROPERTY() on every reflected member (with specifiers: EditAnywhere, BlueprintReadOnly, ReplicatedUsing=, Category=)
- UFUNCTION() with networking specifiers (Server, Reliable, Client, Reliable, BlueprintNativeEvent, BlueprintCallable)
- irtual/override used consistently
- Pointer style: AActor* (asterisk on type, not AActor *)
- Braces: opening brace on same line (Java-style)
- SPLITSTREAM_API on every UCLASS/USTRUCT/UENUM in the module
- Cast<> with null check (not CastChecked<>)
- const methods where appropriate

### Blueprint/C++ Integration

The project uses two UE5 patterns for C++/Blueprint code splitting:

| Specifier | C++ Body | Blueprint Override | When to Use |
|-----------|----------|--------------------|-------------|
| `BlueprintImplementableEvent` | No body (declaration only) | Must implement | C++ fires the event, Blueprint handles visuals/animations (e.g., StartAimCamera, OpenDoor) |
| `BlueprintNativeEvent` | Yes (via _Implementation suffix) | Optionally override | C++ provides a default, Blueprint can customize. Used on all interfaces (e.g., IInteractable::Interact_Implementation) |
| `BlueprintCallable` | Yes (normal C++ body) | Can call from BP graphs | Any function that needs to be accessible from Blueprint graphs |

**Interface convention:** All interfaces (IInteractable, IDetectable, IUnlockable, etc.) use BlueprintNativeEvent so actors can either rely on the C++ default or implement their own behavior in Blueprint.

**Concrete class convention:** BlueprintImplementableEvent is used on concrete actor classes (ADefaultCharacter, ADoorBase) where C++ triggers purely visual/animation concerns best done in Blueprint.

**Rule of thumb:** If the function modifies gameplay state, keep it in C++. If it is purely visual/animation, use BlueprintImplementableEvent. If it could go either way, use BlueprintNativeEvent.

## Conventions � Architecture

### Interface-driven design
UE interfaces (UINTERFACE + native class) for cross-cutting capabilities:
- IInteractable � interaction system (interact, cancel, highlight, item-gating)
- IDetectable � detection pipeline (detected, lost, fully detected)
- IUnlockable � keycard-locked objects
- IRepairable � robot-repairable objects
- IGhostMirrorSource / IGhostRevealable � ghost mirroring across timelines
- IPuzzleCompletionReceiver � puzzle completion/reset
- IServerActionInterface � generic server-side action dispatch

Interfaces use BlueprintNativeEvent to allow Blueprint implementations.

### Component-based composition
Reusable actor components rather than deep inheritance:
- UInteractionComponent � line trace, highlight, hold/instant interact
- UDetectionComponent � progressive detection with distance-weighted fill rates
- UHackComponent, ULockPickComponent, USearchComponent, UProximityHackComponent � mini-game interactions
- UInventoryComponent � item storage, active item, drop preview

### GAS conventions

#### ASC lives on PlayerState
- `ADefaultPlayerState` owns the AbilitySystemComponent - persists across pawn respawns
- Custom `UDefaultAbilitySystemComponent` - ability granting, input routing
- Attribute change delegates stored with `FDelegateHandle` for cleanup
- Gameplay tags as communication layer between subsystems

#### Attribute set pattern
All attributes follow a strict 4-step declaration pattern using a custom ATTRIBUTE_ACCESSORS macro:

```cpp
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \\
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \\
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \\
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \\
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
```

Each attribute in `UPlayerAttributeSet` follows:
1. `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_<Name>)`
2. `FGameplayAttributeData <Name>;`
3. `ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, <Name>)`
4. `UFUNCTION()` + `void OnRep_<Name>(const FGameplayAttributeData& OldValue);`

Current attributes: Health, WalkSpeed, RunSpeed, CrouchSpeed

#### Ability granting via DataAssets
- Abilities are never hard-coded per class. Instead, they are granted via DataAsset references:
  - `UAbilityInputSet` - input-tagged ability sets (Future, Past, Solo eras)
  - `UDefaultGASet` - non-input abilities (passive effects, always-on)
- `UDefaultAbilitySystemComponent.GrantAbilitiesFromSet()` and `GrantAbilitiesFromDefaultSet()`
- `UItemBase` supports per-instance GAS effect/ability grants (for items like keycards, pistols)

#### Input routing via tags
- Input is routed through `FGameplayTag`, not action names
- `HandleAbilityInputPressed(FGameplayTag)` / `HandleAbilityInputReleased(FGameplayTag)`
- `UInputMappingSet` maps Input Actions to gameplay tags

#### Attribute initialization
- Single GE class passed to `InitializeAttributes(TSubclassOf<UGameplayEffect>)`
- Called on the ASC when the player state is ready

**UItemBase** (at `DataAssets/ItemBase.h`) supports per-instance GAS effect/ability grants for items like keycards, pistols, and power cells.
- Gameplay tags as communication layer between subsystems
- UItemBase supports per-instance GAS effect/ability grants

### Data-driven configuration
Data assets for configurable game content:
- UItemBase � item definitions with pickup class, GAS effects, abilities
- UAbilityInputSet / UDefaultGASet � ability sets for teams/weapons
- UInputMappingSet � input action-to-tag mappings
- UMenuTabsData, USettingsTabsData � menu tab configuration
- UMapListData � available map list
- UUserSettingsSaveGame � persisted keybinds, mouse sensitivity

### Server-authoritative multiplayer
- All game logic runs on server
- ReplicatedUsing + OnRep_ callbacks for state replication
- Server, Reliable / Client, Reliable RPCs
- DOREPLIFETIME / DOREPLIFETIME_CONDITION macros
- Seamless travel via CopyProperties() on PlayerState
- Session lifecycle: Create ? Join ? Destroy (with cleanup)

### Game mode hierarchy
```
ABaseGameMode --? ADefaultGameMode
               +-? ALobbyGameMode

ABaseGameState --? ADefaultGameState
               +-? ALobbyGameState

ABasePlayerController --? ADefaultPlayerController
                      +--? ALobbyPlayerController
                      +--? AMainMenuPlayerController
```

### AI architecture
- StateTree-driven behavior (UStateTreeComponent)
- Custom StateTree tasks: FSTTask_WalkAround, FSTTask_StandIdle, FSTTask_Interact
- AI Perception with sight config (sight range, peripheral vision angle)
- Progressive detection pipeline with fill rate per detector
- Repair system: NPCRobotGuard queues repairs via URepairableRegistry

### World subsystems
- UDetectorRegistry � tracks all detection actors (replaces world-wide iteration)
- URepairableRegistry � tracks repairable actors with registration/unregistration delegates

### Logging
```cpp
UE_LOG(LogSplitstream, Warning, TEXT("..."));
UE_LOG(LogSplitstream, Verbose, TEXT("..."));
UE_LOG(LogSplitstream, Error, TEXT("..."));
```
- Define log category in Splitstream.h: DECLARE_LOG_CATEGORY_EXTERN(LogSplitstream, Log, All);
- Verbose logging recommended for RPC entry/exit traces

### Timelines / eras
- ETimelineEra enum: Past, Future
- Ghost mirroring system: past-echo projection, ghost reveal toggle
- Era-specific gameplay: alarm timelines per era, era-bound interactables


### Inheritance depth rule
- Maximum three tiers: UE framework base -> project base -> concrete class
  - AGameMode -> ABaseGameMode -> ADefaultGameMode
  - AActor -> ADoorBase -> AFutureDoor / APastDoor
  - ACharacter -> AAICharacter -> AGuardCharacter -> ARobotGuardCharacter
- Never exceed three tiers. If more specialization is needed, use composition (components) or interface implementation instead of deeper inheritance.

### Composition vs inheritance decision table

| Situation | Use | Example |
|-----------|-----|---------|
| Shared behavior across unrelated actor types | UActorComponent | UDetectionComponent on player, AI, and vent actors |
| "Is-a" relationship with shared base logic | Inheritance (max 3 tiers) | ADoorBase -> AFutureDoor/APastDoor |
| Cross-cutting capability (any actor can do X) | UE Interface (I prefix) | IInteractable, IDetectable, IRepairable |
| Per-frame or tick-based logic | UActorComponent with TickComponent | UDetectionComponent tracks detection progress per detector |
| Global registry / cross-actor discovery | UWorldSubsystem + delegates | URepairableRegistry, UDetectorRegistry |

### Component communication strategies
Components communicate using five strategies, listed in order of preference:

1. **Direct UPROPERTY reference** â€” owner holds a known component pointer (fastest, tightest coupling). Used when the owner explicitly creates and manages a specific component.
   ```cpp
   UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
   UInteractionComponent* InteractionComponent;
   ```

2. **FindComponentByClass** â€” sibling discovery on the same owning actor (loose, within-actor scope).
   ```cpp
   UInventoryComponent* Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
   ```

3. **Interface dispatch** â€” call methods on an actor that implements an interface, without knowing its concrete type.
   ```cpp
   if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
       IInteractable::Execute_Interact(Target, Instigator);
   ```

4. **PlayerController RPC bridge** â€” route through the owning Pawn's Controller for Client RPCs (UI updates).
   ```cpp
   ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(CachedOwner->GetController());
   if (PC) PC->ClientUpdateDetectionWidget(Detector, Progress, bIsLocked);
   ```

5. **Subsystem + delegate broadcast** â€” global communication via UWorldSubsystem with multicast delegates.
   ```cpp
   // Subsystem broadcasts
   GetWorld()->GetSubsystem<URepairableRegistry>()->RegisterRepairable(this);
   // Listener binds in BeginPlay
   Registry->OnRepairableRegistered.AddDynamic(this, &ARobotGuardCharacter::HandleRepairableRegistered);
   ```

### Interface dispatch pattern
Always use the ImplementsInterface + Execute_ pattern. Never cast to a concrete type just to call interface methods.

```cpp
// Good
if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    IInteractable::Execute_Interact(Target, Instigator);

// Bad â€” creates compile-time coupling to a concrete type
if (ADoorBase* Door = Cast<ADoorBase>(Target))
    Door->Interact(Instigator);
```

Interfaces always use BlueprintNativeEvent (never BlueprintImplementableEvent) so that C++ provides a default implementation via _Implementation and Blueprints can optionally override.

### Past/Future twin pattern
Actors that exist in both timelines follow a consistent pattern:

1. **Shared abstract base** â€” common logic in a base class (e.g., ADoorBase, AVentBase, APowerGenerator)
2. **Future subclass + Past subclass** â€” AFutureDoor / APastDoor, AFutureVent / APastVent
3. **Cross-era communication** â€” Past actor holds a BlueprintAssignable delegate; Future actor holds a TSoftObjectPtr to its Past counterpart and binds a handler
   ```cpp
   // In AFutureDoor::BeginPlay
   if (PastDoor.IsValid())
       PastDoor->OnDoorStateChanged.AddDynamic(this, &AFutureDoor::HandlePastDoorStateChanged);
   ```
4. **Rule**: the Past actor emits state changes; the Future actor listens and mirrors. Era-agnostic logic goes in the base class; era-specific logic goes in the subclass.

### Server authority + state flow

```
Server mutates replicated property
    -> Replication system propagates to clients
        -> OnRep_ callback fires
            -> BlueprintAssignable delegate broadcasts
                -> UI / audio / gameplay reacts
```

**Rules:**
- All state-modifying methods must be server-only, guarded with if (!HasAuthority()) return;
- Never mutate a replicated property on the client (unless cosmetic)
- Server RPCs (Server, Reliable) must validate all parameters at the top of the _Implementation method
- Client RPCs (Client, Reliable) are for UI/visual-only updates â€” never for gameplay state
- ReplicatedUsing with explicit OnRep_ callback is the standard; avoid bare Replicated
- Bind attribute change delegates with FDelegateHandle for proper cleanup

### Delegate patterns
- **BlueprintAssignable delegates** on GameState/PlayerState for UI binding (not hard-coded widget references)
- **Subsystem delegates** for global registries (e.g., URepairableRegistry.OnRepairableRegistered)
- **Bind in BeginPlay / SetupOverlay**, RemoveDynamic before re-binding to prevent duplicates
- **Past-era delegates** for twin communication (Past emits, Future listens)
- **GAS delegates** for attribute changes and gameplay tag events
- Never hold a direct widget reference in gameplay code; use delegates to notify UI
## Conventions � Gameplay Tags

### Format
- Pattern: TAG_Prefix_SubCategory_Detail
- Declared in AbilitySystem/SplitstreamGameplayTags.h using UE_DECLARE_GAMEPLAY_TAG_EXTERN
- Defined in AbilitySystem/SplitstreamGameplayTags.cpp using UE_DEFINE_GAMEPLAY_TAG
- Always prefer tag comparisons over string comparisons

### Tag hierarchy
| Prefix | Category | Examples |
|--------|----------|---------|
| TAG_Character_ | Player character states | Status_Illegal, Status_LockPicking, Status_Private_Area |
| TAG_Character_Ability_ | Character abilities | Future_PastEcho, Solo_Teleport, Hack, Search |
| TAG_AI_ | AI behaviors and events | Behavior_WalkAround, Event_FullyDetected_Pawn, Status_Repair |
| TAG_Weapon_Ability_ | Weapon abilities | Pistol_Fire, Pistol_Aim |
| TAG_World_ | World objects and locations | NavNode_Ground, Floor_Basement, Object_WaterMachine |
| TAG_GameplayCue_ | Gameplay cues | PastEcho_Activated, PastEcho_Deactivated |

## Conventions � Networking

### Replication
```cpp
UPROPERTY(ReplicatedUsing = OnRep_AlarmState)
bool bAlarmActive;

UFUNCTION()
void OnRep_AlarmState();

void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
```
- ReplicatedUsing with explicit OnRep_ callback for state changes
- DOREPLIFETIME / DOREPLIFETIME_CONDITION in GetLifetimeReplicatedProps
- Server-only logic guarded by if (!HasAuthority()) return; at function start
- Avoid RPC spam � batch state changes where possible

### RPC conventions
```cpp
UFUNCTION(Server, Reliable)
void ServerHandleInteract(AActor* TargetActor);

void AMyActor::ServerHandleInteract_Implementation(AActor* TargetActor)
{
    if (!TargetActor) return;
    // server-side logic
}
```
- Server, Reliable for gameplay-critical actions
- Client, Reliable for HUD updates, UI triggers
- All RPC implementations prefixed with class name in _Implementation
- Validate all parameters at top of server RPC

### Seamless travel
- ADefaultPlayerState::CopyProperties() handles state preservation across map transitions
- Session ID, team, display name, avatar persisted
- Lobby ? Game ? Main Menu travel paths


## Game Flow / Session Lifecycle

### Map transition chain
```
MainMenuMap -+-> LobbyMap -+-> PersistentBank (+ FutureBank + PastBank) -+-> MainMenuMap
```

### Session lifecycle
Managed by UDefaultGameInstance (inherits UAdvancedFriendsGameInstance):

1. **Create session** - Host creates a session in the lobby, other players join via AdvancedSessions
2. **Transition to game** - Seamless travel from LobbyMap to PersistentBank
3. **Gameplay** - All game logic runs server-authoritative on the bank map
4. **Return to menu** - Host leaves to MainMenuMap, triggers session destroy

### Seamless travel specifics
- ADefaultPlayerState::CopyProperties() preserves state across map transitions:
  - Session ID, TeamTag, DisplayName, AvatarIndex
  - ASC and attribute set persist (lives on PlayerState)

### Session cleanup
```cpp
// In UDefaultGameInstance
void HostLeaveToMainMenu(const FString& MainMenuMapPath);
void RequestDestroySessionAndCleanup(bool bWaitForTravel, const FString& PendingMapPath);
void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
void CleanupNetDriver();
void CleanupOSSDelegates();
```
- State members: TravelAfterSessionDestroy, SessionDestroyInProgress
- Session creation uses AdvancedSessions plugin (BlueprintImplementableEvent)
## Key files

| File | Purpose |
|------|---------|
| Splitstream.uproject | Project descriptor (UE 5.7, module defs, plugins) |
| Source/Splitstream/Splitstream.Build.cs | Module build rules, dependency modules |
| Source/Splitstream/Splitstream.h/.cpp | Module implementation, log category |
| Config/DefaultEngine.ini | Engine settings, rendering, OSS, net driver |
| Config/DefaultGame.ini | Game settings, GAS paths, packaging |
| Config/DefaultGameplayTags.ini | All gameplay tag definitions |
| Source/Splitstream/DefaultPlayerState.h/.cpp | ASC owner, team/avatar/ready state |
| Source/Splitstream/Player/Characters/DefaultCharacter.h/.cpp | Main player pawn |
| Source/Splitstream/Player/Controllers/DefaultPlayerController.h/.cpp | HUD, pause, alarm UI, detection |
| Source/Splitstream/GameModes/DefaultGameMode.h/.cpp | Alarm/pre-alarm timers, player spawn |
| Source/Splitstream/GameStates/DefaultGameState.h/.cpp | Alarm/pre-alarm state, guard repair, money |
| Source/Splitstream/AbilitySystem/SplitstreamGameplayTags.h/.cpp | All gameplay tag declarations |
| Source/Splitstream/AbilitySystem/DefaultAbilitySystemComponent.h/.cpp | Custom ASC with input routing |
| Source/Splitstream/DataAssets/ItemBase.h/.cpp | Item definitions with GAS grants |
| Source/Splitstream/Interfaces/IInteractable.h | Interaction interface |
| Source/Splitstream/Interfaces/IDetectable.h | Detection pipeline interface |
| Source/Splitstream/ActorComponents/InteractionComponent.h/.cpp | Line-trace interaction system |
| Source/Splitstream/ActorComponents/DetectionComponent.h/.cpp | Progressive detection system |
| Source/Splitstream/Subsystems/DetectorRegistry.h/.cpp | World subsystem for detection actors |
| Source/Splitstream/GameModes/LobbyGameMode.h/.cpp | Lobby session management |
| Source/Splitstream/Saving/UserSettingsSaveGame.h/.cpp | Saved keybinds and settings |
| docs/code-review.md | Full codebase review (grades the project 9.0/10) |

## Dependencies

### Engine modules (Build.cs)
Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, NavigationSystem, StateTreeModule, GameplayStateTreeModule, Niagara, UMG, RHI, RenderCore, GameplayAbilities, GameplayTags, GameplayTasks (public) + Slate, SlateCore, OnlineSubsystem, OnlineSubsystemUtils, NetCore (private)

### Plugins
| Plugin | Source | Purpose |
|--------|--------|---------|
| AdvancedSessions | Marketplace | Extended Steam session/friend management |
| AdvancedSteamSessions | Marketplace | Steam-specific session features |
| CommonLoadingScreen | Epic/Lyra | Pre-load loading screen |
| OnlineSubsystemEOS | Epic | Epic Online Services |
| SteamSockets | Epic | Steam networking transport |

### Content packs
Low_Poly_Sci-Fi_Corridor, PolygonHeist (marketplace assets)

## Adding New Features

### Add a new actor
1. Create C++ class in the appropriate `Actors/` subdirectory (e.g., `Actors/Security/`)
2. Use `UCLASS()`, `GENERATED_BODY()`, `SPLITSTREAM_API`
3. Add `UPROPERTY()` for editable properties, `UFUNCTION()` for reflected methods
4. Implement relevant interfaces (IInteractable, IDetectable, IUnlockable, IRepairable, etc.)
5. Create Blueprint subclass (`BP_` prefix in `Blueprints/Actors/`)
6. Place in level or spawn via GameMode

### Add a new ability
1. Declare a new `FGameplayTag` in `AbilitySystem/SplitstreamGameplayTags.h` using `UE_DECLARE_GAMEPLAY_TAG_EXTERN`
2. Define it in `AbilitySystem/SplitstreamGameplayTags.cpp` using `UE_DEFINE_GAMEPLAY_TAG`
3. Add the tag hierarchy to `Config/DefaultGameplayTags.ini`
4. Create a `UGameplayAbility` subclass in `AbilitySystem/Abilities/`
5. Add it to the appropriate `UAbilityInputSet` or `UDefaultGASet` DataAsset
6. If input-driven, register the Input Action in `IMC_Default` and add to `UInputMappingSet`

### Add a new widget
1. Create a `UUserWidget` subclass in `Widgets/` under the appropriate subdirectory
2. Add `UPROPERTY(meta = (BindWidget))` for child widgets in the Blueprint
3. Create Blueprint subclass (`WBP_` prefix in `Blueprints/Widgets/`)
4. Add to the owning controller or HUD via `CreateWidget<...>()` + `AddToViewport()`
5. Register input mode (Show/Hide mouse cursor, input mode)

### Add a new map
1. Create the level in `Content/Maps/`
2. Add the map reference to `UMapListData` DataAsset (`DA_MapList`)
3. Add to `MapsToCook` list in `Config/DefaultGame.ini`
4. If time-era based, create Future/Past/Persistent variants

### Add a new gameplay tag
1. Add `UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NewCategory_TagName);` in `SplitstreamGameplayTags.h`
2. Add `UE_DEFINE_GAMEPLAY_TAG(TAG_NewCategory_TagName, \"TAG_NewCategory.TagName\");` in `SplitstreamGameplayTags.cpp`
3. Add the tag to the hierarchy in `Config/DefaultGameplayTags.ini`

### Add a new interface
1. Create header in `Interfaces/` with `UINTERFACE(MinimalAPI, Blueprintable)` and native class
2. Use `BlueprintNativeEvent` for methods that should be overridable in Blueprints
3. Implement `_Implementation` methods in C++ for default behavior
4. Use `Cast<IMyInterface>(Actor)` pattern for interface checks in C++

## Off-limits areas
These areas must not be modified without explicit human instruction:

| Area | Why |
|------|-----|
| Plugins/ directory | Third-party plugin code � do not modify |
| Content/ marketplace packs (PolygonHeist, Low_Poly_Sci-Fi_Corridor) | Licensed assets � modification may violate license |
| Config/DefaultEngine.ini SteamDevAppId | Test Steam App ID � sensitive for production |
| Config/DefaultEngine.ini AndroidFileServer SecurityToken | Security concern for shipping builds |
| DerivedDataCache/, Intermediate/, Saved/, Binaries/ | Build artifacts � gitignored |
| Source/Splitstream.Target.cs / Editor.Target.cs | Build configuration � requires engine-level reasoning |
| JWT / auth pipeline (if added) | Security � could lock all users out |
| Seamless travel CopyProperties() | Affects all map transitions � easy to break silently |
| Attribute set replication | Financial/balance impact � affects all gameplay |

## Conventions � General Rules

### What to avoid
- Avoid custom namespaces (use UE type prefixes instead)
- Avoid CastChecked<> (use Cast<> with null check)
- Avoid IsPendingKillPending() (use IsValid())
- Avoid string comparisons where gameplay tags can be used
- Avoid putting logic in constructors beyond component creation
- Avoid ticking components by default (opt-in via CanEverTick = true)
- Avoid adding dependencies to Build.cs without evaluating the cost
- Avoid hard-coding map paths (use UMapListData data asset)
- Avoid using PrimaryActorTick.bCanEverTick = true on actor if components handle it
- Avoid deleting the log category without checking all usages

### What to always do
- Always use #pragma once (never #ifndef guards)
- Always include "Foo.generated.h" as the last include in .h files
- Always use UPROPERTY() / UFUNCTION() / UCLASS() / USTRUCT() / UENUM() / UINTERFACE() macros
- Always use SPLITSTREAM_API on exported class declarations
- Always use GENERATED_BODY() in reflected types
- Always use TEXT() macro for string literals in C++
- Always guard server-only code with if (!HasAuthority()) return;
- Always use ReplicatedUsing with explicit OnRep_ callbacks
- Always validate parameters at top of server RPC implementations
- Always prefer FGameplayTag over FString/FName for semantic identification
- Always initialize UPROPERTY members inline in the header
- Always disable tick unless necessary (CanEverTick = false)
- Always forward-declare in headers where possible to minimize includes
- Always use TObjectPtr for UObject pointers in UE5
- Always check IsValid() for UObject validity (handles both null and pending-kill)
- Always clean up attribute change delegate handles on destruction
- Always check the off-limits list before modifying sensitive areas

### Naming summary
| Element | Convention | Example |
|---------|-----------|---------|
| UObject subclasses | U + PascalCase | UInteractionComponent |
| Actor subclasses | A + PascalCase | ADefaultCharacter |
| Structs | F + PascalCase | FInventorySlot |
| Interfaces | I + PascalCase | IInteractable |
| Enums | PascalCase (opt. E prefix) | ETimelineEra |
| Methods | PascalCase | HandleInstantInteract() |
| Member variables | PascalCase | InteractDistance |
| Bool members |  + PascalCase | IsHoldingInteract |
| Properties | PascalCase with UPROPERTY | DropPreviewMesh |
| Delegates | FOn + PascalCase | FOnDetectionBegan |
| Gameplay tags | TAG_ + PascalCase | TAG_Character_Status_Illegal |
| Parameters | PascalCase | AActor* Interactor |
| Local variables | PascalCase | FHitResult Hit |

## Running / building
- **Open project**: Double-click Splitstream.uproject (UE 5.7)
- **Generate project files**: Right-click .uproject ? "Generate Visual Studio project files"
- **Build**: Open Splitstream.sln ? Build solution (Development Editor config)
- **Package**: Project Settings ? Packaging ? Build configuration ? Shipping
- **Dedicated server**: Add -server to target, build SplitstreamServer target
- **Steam**: Set SteamDevAppId=480 in DefaultEngine.ini (dev only)

## Existing documentation
See the docs/ folder for in-depth coverage of specific areas:
- rchitecture.md � module structure, build deps, directory layout, core patterns
- gameplay-systems.md � detection, alarms, timelines, puzzles, stealth
- bility-system.md � GAS integration, attributes, abilities, tags
- 
etworking.md � replication, session management, travel flow
- ctors-and-components.md � actor and component reference
- ui-widgets.md � HUD, menus, mini-game widgets
- code-review.md � full codebase review (9.0/10), known issues, improvement path

## Notes
- Project uses BuildSettingsVersion.V6 and IncludeOrderVersion.Unreal5_7
- GAS lives on PlayerState, not the pawn � correct for multiplayer with respawning
- No EF Core / SQL � all data is asset-driven (DataAssets) or session-based
- Marketplace content may have license restrictions on redistribution
- Known security gap: AndroidFileServer SecurityToken exposed in config � remove for shipping
- SteamDevAppId=480 is the test App ID � update for production

- **No automated tests** exist in the project. No test module, no Spec/functional tests, no test target.
  If tests are added, use UE Automation Spec (`FAutomationTestBase`) with a separate test module.
  Integration tests should assert preconditions and avoid silent passes on empty data.
