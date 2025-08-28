# Echoes of Time

A co-op multiplayer Unreal Engine 5 project featuring time-travel mechanics, team-based gameplay, inventory systems, lockpicking, and a robust lobby system. Designed for extensibility, network play, and clean separation of gameplay, UI, and networking logic.

---

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Setup Instructions](#setup-instructions)
- [Gameplay Overview](#gameplay-overview)
- [Key Systems](#key-systems)
    - [Lobby System](#lobby-system)
    - [Inventory System](#inventory-system)
    - [Lockpicking System](#lockpicking-system)
    - [Time Objects (Past/Future)](#time-objects-pastfuture)
    - [Ghost/Echo System](#ghostecho-system)
- [Extending & Contribution](#extending--contribution)
- [Known Issues & TODOs](#known-issues--todos)
- [License](#license)

---

## Features

- **Lobby with Team Selection:** Players join a lobby, select team (Past/Future), ready up, and start the match.
- **Inventory Management:** Fully replicated, with support for item instance IDs, active slot switching, and item usage.
- **Lockpicking Minigame:** GAS-driven ability, real-time widget feedback, multiplayer-safe input handling.
- **Time Objects:** Items and doors exist in both timelines, with future objects linked/influenced by past actions.
- **Ghost Reveal System:** Ghosts are visible to players under certain conditions (e.g., Past Echo ability, security cameras).
- **Security Cameras:** Detect player/guard actors using interface-based detection, with visualization and cone logic.
- **Extensible UI:** Modular widgets for friend lists, lobby info, inventory, lockpicking, and more.

---

## Project Structure

```
/Source
  /Actors
    - DoorBase.cpp/h
    - ItemPickup.cpp/h
    - SecurityCamera.cpp/h
    - TimeObjects/
      - PastDoor.cpp/h
      - FutureDoor.cpp/h
      - PastItemPickup.cpp/h
      - FutureItemPickup.cpp/h
  /AbilitySystem
    - DefaultGALockPick.cpp/h
    - FutureGAPastEcho.cpp/h
    - AbilityTasks/
      - LockPickAbilityTask.cpp/h
  /Components
    - InventoryComponent.cpp/h
    - LockPickComponent.cpp/h
  /Controllers
    - DefaultPlayerController.cpp/h
    - LobbyPlayerController.cpp/h
  /Widgets
    - HUD/
      - CharacterHUD.cpp/h
      - CharacterOverlay.cpp/h
      - LockPickWidget.cpp/h
    - Lobby/
      - LobbyUI.cpp/h
      - PlayerLobbyInfo.cpp/h
      - OpenFriendsListButton.cpp/h
      - FriendList.cpp/h
      - FriendWidget.cpp/h
  /Interfaces
    - IInteractable.h
    - ICameraDetectable.h
    - IGhostMirrorSource.h
    - IGhostRevealable.h
  /GameModes
    - DefaultGameMode.cpp/h
    - LobbyGameMode.cpp/h
    - LobbyGameState.cpp/h
  /Misc
    - NavNode.cpp/h
    - DefaultGameInstance.cpp/h
```

---

## Setup Instructions

1. **Requirements**
   - Unreal Engine 5.x
   - Visual Studio 2022 or compatible (for C++ compilation)

2. **Clone and Build**
   ```sh
   git clone <repo-url>
   cd EchoesOfTime
   # Open EchoesOfTime.uproject in Unreal Editor
   # Or build using your IDE
   ```

3. **Configure Plugins**
   - Enable **GameplayAbilities** and **Networking** plugins in Unreal.
   - If using OnlineSubsystem for sessions, configure in `DefaultEngine.ini`.

4. **Editor Setup**
   - Open the map `/Game/Maps/MainMenuMap` or `/Game/Maps/LobbyMap`.
   - Set `DefaultGameMode` and `DefaultPawnClass` in project settings if not already done.

---

## Gameplay Overview

- **Join Lobby:** Players connect to the lobby, see other players, and can ready up/change team.
- **Team Assignment:** Teams are "Past" and "Future". Team affects spawn location, available items, and objects.
- **Start Game:** When host clicks "Start" and all players are ready (configurable), server travels to game map.
- **Inventory/Interaction:** Players collect, use, and drop items; interact with objects (doors, scanners, etc.).
- **Lockpicking:** Interact with certain doors/items to trigger a lockpicking minigame.
- **Time Mechanics:** Items/doors in "future" may depend on what happens to their "past" counterpart.

---

## Key Systems

### Lobby System

- `LobbyGameMode`, `LobbyGameState`, `LobbyPlatformActor`, and associated widgets.
- Players can switch teams, ready up, and be auto-assigned to lobby slots.
- Host controls match start and can kick players.

### Inventory System

- `InventoryComponent` attaches to characters.
- Handles item slot management, replication, and UI updates.
- Supports item instance IDs for time-based invalidation (e.g., picking up in the past invalidates in the future).

### Lockpicking System

- GAS-based (`DefaultGALockPick`, `LockPickAbilityTask`).
- Widget overlays guide input; server and client states are kept in sync.
- Secure against cheating by running logic on server.

### Time Objects (Past/Future)

- `PastItemPickup`, `FutureItemPickup`, `PastDoor`, `FutureDoor`.
- Actions on past items/doors propagate to their future counterparts.
- "Invalidation" events remove items from inventory or world if timeline changes.

### Ghost/Echo System

- `GhostCharacterActor`, interfaces for visibility and mirroring.
- Controlled by ability tags (`PastEcho`), security camera detection, or other triggers.

---

## Extending & Contribution

- **Add New Items:** Inherit from `UItemBase`, set up mesh, icon, and logic. Register in inventory as needed.
- **Add New Abilities:** Inherit from `UGameplayAbility`, register in `DefaultAbilities` in `DefaultPlayerState`.
- **Expand UI:** Create new UUserWidget-derived classes and bind to game state/components.
- **Contribute:** Fork, branch, submit PRs. See [CONTRIBUTING.md](CONTRIBUTING.md) for code style and workflow guidelines (add this file if open source).

---

## Known Issues & TODOs

- **DefaultGameInstance** is a stub—expand with session logic if needed.
- **FriendList/FriendWidget** features are incomplete.
- Comment/documentation pass needed on most files.
- Some "OnRep_" functions could be more granular for performance.
- Need more unit tests and blueprint integration tests.

---

## License

[MIT](LICENSE) or custom license—add your chosen license file.

---

## Acknowledgments

- Unreal Engine community, Epic Games
- GameplayAbilities plugin authors
- [Your contributors here]

---
