# Networking & Multiplayer

This document covers the multiplayer architecture, replication strategy, and session management in **Splitstream**.

---

## 1. Network Architecture

Splitstream uses a **listen-server** model:
- One player acts as the host, running both client and server.
- Other players connect as remote clients.
- All authoritative logic runs on the server (GameMode, GameState mutations).
- Clients receive state updates via property replication and RPCs.

### Seamless Travel

The project uses `bUseSeamlessTravel = true` (set in `ALobbyGameMode` constructor). This means:
- `ServerTravel()` transitions all connected players together.
- Player states persist across map transitions (via `CopyProperties()`).
- `ADefaultPlayerState::CopyProperties()` explicitly copies `TeamName` to the new player state.

---

## 2. Replication Strategy

### Replicated Properties by Class

#### `ADefaultPlayerState`
| Property | Replication | Purpose |
|---|---|---|
| `DisplayName` | `ReplicatedUsing = OnRep_Meta` | Player display name |
| `AvatarIndex` | `ReplicatedUsing = OnRep_Meta` | Selected avatar |
| `bIsReady` | `ReplicatedUsing = OnRep_Ready` | Lobby ready state |
| `TeamName` | `ReplicatedUsing = OnRep_TeamName` | Team assignment (Past/Future/Solo) |

#### `ADefaultCharacter`
| Property | Replication | Purpose |
|---|---|---|
| `bIsSprinting` | `ReplicatedUsing = OnRep_SprintState` | Sprint state for speed sync |
| `Pitch` | `ReplicatedUsing = OnRep_Pitch` | Camera pitch for third-person view |

#### `ADefaultGameState`
| Property | Replication | Purpose |
|---|---|---|
| `AlarmEndTime` | `ReplicatedUsing = OnRep_AlarmStarted` | Alarm countdown end |
| `bAlarmActive` | `ReplicatedUsing = OnRep_AlarmActive` | Alarm active flag |
| `AlarmInstigator` | `Replicated` | Actor that caused the alarm |
| `AlarmEra` | `Replicated` | Which timeline the alarm is in |
| `PastPreAlarm` | `ReplicatedUsing = OnRep_PastPreAlarm` | Past era pre-alarm state |
| `FuturePreAlarm` | `ReplicatedUsing = OnRep_FuturePreAlarm` | Future era pre-alarm state |
| `GuardRepairCountdowns` | `ReplicatedUsing = OnRep_GuardRepairCountdowns` | Repair ETA data |
| `CurrentMoneyCollected` | `ReplicatedUsing = OnRep_CurrentMoneyCollected` | Objective progress |

#### `ALobbyGameState`
| Property | Replication | Purpose |
|---|---|---|
| `TotalPlayers` | `ReplicatedUsing = OnRep_ReadyAggregates` | Total player count |
| `ReadyPlayers` | `ReplicatedUsing = OnRep_ReadyAggregates` | Ready player count |
| `bAllPlayersReady` | `ReplicatedUsing = OnRep_ReadyAggregates` | All ready flag |

#### `UInventoryComponent`
| Property | Replication | Purpose |
|---|---|---|
| `Slots` | `ReplicatedUsing = OnRep_Slots` | Inventory content |
| `ActiveSlotIndex` | `ReplicatedUsing = OnRep_ActiveSlotIndex` | Currently equipped slot |

#### `ADronePawn`
| Property | Replication | Purpose |
|---|---|---|
| `DetectedActor` | `ReplicatedUsing = OnRep_DetectedActor` | Spotlight color change |

#### `AGuardCharacter`
| Property | Replication | Purpose |
|---|---|---|
| `GuardName` | `ReplicatedUsing = OnRep_GuardName` | Name text render |
| `bIsInCameraView` | `ReplicatedUsing = OnRep_IsInCameraView` | Ghost visibility |

#### `AGhostCharacterActor`
| Property | Replication | Purpose |
|---|---|---|
| `GhostTargetLocation` | `ReplicatedUsing = OnRep_GhostTargetLocation` | Smooth ghost position |
| `GhostTargetRotation` | `ReplicatedUsing = OnRep_GhostTargetRotation` | Smooth ghost rotation |

---

## 3. Remote Procedure Calls (RPCs)

### Server RPCs (Client → Server)

| Class | RPC | Purpose |
|---|---|---|
| `ADefaultPlayerState` | `ServerSetDisplayName` | Set player name |
| `ADefaultPlayerState` | `ServerSetAvatarIndex` | Set avatar |
| `ADefaultPlayerState` | `ServerSetReady` | Set ready state |
| `ADefaultPlayerState` | `ServerToggleReady` | Toggle ready |
| `ADefaultPlayerState` | `ServerSetTeam` | Set team |
| `ADefaultCharacter` | `ServerStartSprint` | Begin sprinting |
| `ADefaultCharacter` | `ServerStopSprint` | Stop sprinting |
| `ADefaultCharacter` | `ServerHandleInteract` | Validate + execute interaction |
| `ADefaultCharacter` | `ServerCameraRotationUpdate` | Sync camera pitch |
| `UInventoryComponent` | `ServerSetActiveSlot` | Change active inventory slot |
| `UInventoryComponent` | `ServerDropActiveItem` | Drop item at location |
| `UInventoryComponent` | `ServerAddItem` | Add item to inventory |
| `ALobbyPlayerController` | `ServerStartGame` | Request match start |
| `ALobbyPlayerController` | `ServerKickPlayer` | Kick a player |
| `ULockPickComponent` | `ServerTrySetPin` | Submit pin angle |
| `ADefaultPlayerController` | `ServerExecuteAction` | Generic server action dispatch |

### Client RPCs (Server → Client)

| Class | RPC | Purpose |
|---|---|---|
| `ADefaultPlayerController` | `ClientEnterSpectatorMode` | Switch to spectator |
| `ADefaultPlayerController` | `ClientShowCalendarWidget` | Display calendar UI |
| `ADefaultPlayerController` | `ClientUpdateDetectionWidget` | Update detection indicator |
| `ADefaultPlayerController` | `ClientShowLoadingScreen` | Show loading widget |
| `ALobbyPlayerController` | `ClientSetStartButtonEnabled` | Enable/disable start button |
| `ALobbyPlayerController` | `ClientShowLoadingScreen` | Show loading widget |

### NetMulticast RPCs

| Class | RPC | Purpose |
|---|---|---|
| `UHackComponent` | `MulticastResetHackElapsed` | Reset hack progress on all clients |
| `USearchComponent` | `MulticastResetSearchElapsed` | Reset search progress on all clients |

---

## 4. Session Management

### `UDefaultGameInstance`

Extends `UAdvancedFriendsGameInstance` (from AdvancedSessions plugin).

**Session creation:**
- `CreateSession()` — BlueprintImplementableEvent for session creation (implemented in Blueprint).

**Host leave flow:**
1. `HostLeaveToMainMenu()` — Tells all remote clients to `ClientTravel()` to the main menu, then calls `RequestDestroySessionAndCleanup()`.
2. `RequestDestroySessionAndCleanup()` — Binds `OnDestroySessionComplete` delegate and calls `Session->DestroySession(NAME_GameSession)`.
3. `OnDestroySessionComplete()` — Cleans up delegates and net drivers, then optionally `ServerTravel()` to menu.
4. `CleanupNetDriver()` — Iterates all `ActiveNetDrivers` in the world context and shuts them down.
5. `CleanupOSSDelegates()` — Clears the destroy session delegate.

**Guard against double-destroy:**
- `bSessionDestroyInProgress` flag prevents re-entrant session destruction.

### `ALobbyGameMode` — Session Flow

**Match start:**
1. Host presses Start → `ALobbyPlayerController::ServerStartGame()` → `ALobbyGameMode::StartGameIfAllowed()`.
2. All clients receive `ClientShowLoadingScreen()`.
3. After 0.25s delay, `DoServerTravelToMatch()` calls `ServerTravel()` with `?listen`.

**Player kick:**
1. Host calls `ServerKickPlayer()` → `LobbyGameMode::KickPlayerByPlayerState()`.
2. Kicked client receives `ClientShowLoadingScreen()` + `ClientTravel()` to main menu.
3. After 0.35s delay, the server closes the connection.

---

## 5. Server Action Interface

### `IServerActionInterface`

A generic interface for dispatching server-side actions with a flexible payload:

```cpp
struct FServerActionPayload
{
    bool    BoolValue;
    float   FloatValue;
    int32   IntValue;
    UObject* ObjectValue;
    FString StringValue;
};

void ExecuteServerAction(const FServerActionPayload& Payload);
```

Used by `ADefaultPlayerController::ServerExecuteAction()` to route arbitrary actions to any object implementing the interface. Currently used by `ULockPickComponent` and `ATerminal`.

---

## 6. Net Relevancy

### `AGhostCharacterActor::IsNetRelevantFor()`

Ghost actors override net relevancy to control which clients receive updates, optimizing bandwidth for cross-timeline ghost mirroring.

---

## 7. Input Mapping Persistence

User keybind customizations are saved via `UUserSettingsSaveGame`:
- `FSavedKeybind` stores `ActionName` + `FKey` pairs.
- On load, `UDefaultGameInstance::LoadUserSettings()` duplicates the default `InputMappingContext` and remaps keys.
- The runtime `InputMappingContext` is pushed to `EnhancedInputLocalPlayerSubsystem` on character `BeginPlay()`.
