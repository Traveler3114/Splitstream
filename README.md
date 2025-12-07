# Echoes of Time

A multiplayer stealth heist game built in Unreal Engine 5.7 featuring unique time-travel mechanics where players navigate between Past and Future timelines to solve puzzles, evade detection, and complete objectives.

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue)](https://www.unrealengine.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Project Structure](#project-structure)
- [Gameplay Systems](#gameplay-systems)
  - [Time Travel Mechanics](#time-travel-mechanics)
  - [Stealth and Detection](#stealth-and-detection)
  - [Ability System](#ability-system)
  - [Inventory System](#inventory-system)
  - [Puzzle Systems](#puzzle-systems)
  - [AI Characters](#ai-characters)
- [Multiplayer](#multiplayer)
- [Development Guide](#development-guide)
  - [Building the Project](#building-the-project)
  - [Code Architecture](#code-architecture)
  - [Performance Optimizations](#performance-optimizations)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

**Echoes of Time** is a stealth-based heist game where players use time manipulation to complete missions. Players can toggle between two timelines—Past and Future—to discover clues, avoid detection, and solve complex puzzles. The game features procedurally generated content, ensuring each playthrough offers unique challenges.

The game utilizes Unreal Engine's Gameplay Ability System (GAS) for robust and extensible ability mechanics, supports multiplayer sessions via Steam and Epic Online Services (EOS), and features optimized network replication for smooth online play.

---

## Key Features

### Time Travel Mechanics
- **Dual Timeline System**: Switch between Past and Future eras to reveal hidden paths, objects, and information
- **Ghost Character System**: See echoes of past player movements in the Future timeline
- **Timeline-Specific Objects**: Doors, items, and obstacles that only exist or are accessible in specific eras
- **Temporal Puzzle Solving**: Use information from one timeline to solve puzzles in another

### Stealth Gameplay
- **Advanced Detection System**: Dynamic detection by guards, civilians, and security cameras
- **Progressive Detection**: Gradual detection mechanics with visual feedback
- **Metal Detectors & Alarms**: Environmental hazards that can trigger security responses
- **Illegal Item System**: Certain items trigger detection when carried in restricted areas
- **Crouching & Sprinting**: Movement options to avoid or escape detection

### Abilities & Skills
- **Hacking**: Disable security systems and access restricted computers
- **Lockpicking**: Open locked doors and lockers with mini-game mechanics
- **Searching**: Investigate objects, desks, and containers for clues and items
- **Past Echo Ability**: Activate to see ghost characters from the Past timeline while in the Future
- **Weapon System**: Basic pistol mechanics with aiming and firing

### Puzzle Systems
- **Keypad Codes**: Find and enter codes to unlock secured areas
- **Wire Puzzles**: Connect wires in the correct sequence to complete circuits
- **Lever Combinations**: Activate levers in specific orders to trigger mechanisms
- **Calendar/Date Puzzles**: Discover dates from documents to unlock systems
- **Laser Security**: Navigate around or disable laser detection grids
- **Archive Research**: Use in-game computers to research historical information

### Procedural Generation
- **Dynamic Level Content**: Procedurally spawned NPCs, items, and puzzle solutions
- **Randomized Codes & Dates**: Security codes and calendar dates change each playthrough
- **Spawn Point System**: Dynamic placement of civilians, guards, and items
- **Random Wire Sequences**: Procedurally generated wire puzzle solutions

### Multiplayer
- **Co-op Gameplay**: Work with friends to complete heists
- **Steam & EOS Integration**: Cross-platform multiplayer support
- **Lobby System**: Friend invites and session management
- **Network Optimization**: Efficient replication with variable update frequencies

---

## System Requirements

### Minimum Requirements
- **OS**: Windows 10 64-bit
- **Processor**: Quad-core Intel or AMD, 2.5 GHz or faster
- **Memory**: 8 GB RAM
- **Graphics**: DirectX 11 or 12 compatible graphics card
- **Storage**: 10 GB available space
- **Network**: Broadband Internet connection (for multiplayer)

### Development Requirements
- **Unreal Engine**: Version 5.7
- **IDE**: Visual Studio 2022 (recommended) or Rider
- **C++ Compiler**: MSVC 14.3+ or equivalent
- **.NET**: .NET Framework 4.7.2 or higher (for build tools)

---

## Installation

### For Players

1. **Clone or download** the repository:
   ```bash
   git clone https://github.com/Traveler3114/Echoes-of-Time.git
   cd Echoes-of-Time
   ```

2. **Open the project**:
   - Right-click on `EchoesOfTime.uproject` and select "Generate Visual Studio project files"
   - Open `EchoesOfTime.sln` in Visual Studio
   - Build the solution (Ctrl+Shift+B)

3. **Launch the game**:
   - Open `EchoesOfTime.uproject` in Unreal Engine 5.7
   - Click the Play button or press Alt+P

### For Developers

1. **Install Unreal Engine 5.7** from the Epic Games Launcher

2. **Clone the repository**:
   ```bash
   git clone https://github.com/Traveler3114/Echoes-of-Time.git
   cd Echoes-of-Time
   ```

3. **Generate project files**:
   ```bash
   # Windows
   right-click EchoesOfTime.uproject -> "Generate Visual Studio project files"
   
   # Or via command line (if you have UE5 installed)
   "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" EchoesOfTime Win64 Development -project="EchoesOfTime.uproject"
   ```

4. **Open and build**:
   - Open `EchoesOfTime.sln` in Visual Studio 2022
   - Set build configuration to Development Editor
   - Build the solution

5. **Launch the editor**:
   - Open `EchoesOfTime.uproject` in Unreal Engine 5.7

---

## Project Structure

```
Echoes-of-Time/
├── Config/                          # Configuration files
│   ├── DefaultEngine.ini           # Engine settings
│   ├── DefaultGame.ini             # Game-specific settings
│   ├── DefaultInput.ini            # Input bindings
│   └── DefaultGameplayTags.ini     # Gameplay tag definitions
├── Content/                         # Blueprint assets, materials, levels
│   └── Blueprints/                 # Blueprint classes
├── Plugins/                         # Third-party plugins
│   ├── AdvancedSessions/           # Multiplayer session management
│   └── AdvancedSteamSessions/      # Steam integration
├── Source/
│   └── EchoesOfTime/               # C++ source code
│       ├── AbilitySystem/          # Gameplay Ability System implementation
│       │   ├── Abilities/          # Gameplay abilities (Hack, Search, etc.)
│       │   ├── AttributeSets/      # Player attributes
│       │   ├── GameplayCues/       # Visual/audio effects
│       │   └── AbilityTasks/       # Async ability tasks
│       ├── ActorComponents/        # Reusable actor components
│       │   ├── HackComponent.cpp   # Hacking functionality
│       │   ├── LockPickComponent.cpp # Lockpicking mechanics
│       │   ├── SearchComponent.cpp # Search functionality
│       │   └── InventoryComponent.cpp # Inventory management
│       ├── Actors/                 # Actor classes
│       │   ├── TimeObjects/        # Timeline-specific actors
│       │   ├── Computers/          # Interactive computers
│       │   ├── KeypadScanner/      # Keypad puzzle actors
│       │   ├── Laser/              # Laser security system
│       │   ├── Lever/              # Lever puzzle system
│       │   ├── Wire/               # Wire puzzle system
│       │   ├── DoorBase.cpp        # Door actors
│       │   ├── SecurityCamera.cpp  # Security camera AI
│       │   ├── MetalDetector.cpp   # Metal detector triggers
│       │   └── ItemPickup.cpp      # Collectible items
│       ├── Characters/             # Character classes
│       │   ├── DefaultCharacter.cpp # Player character
│       │   ├── GuardCharacter.cpp  # Guard AI
│       │   ├── CivilianCharacter.cpp # Civilian NPCs
│       │   └── DronePawn.cpp       # Surveillance drone
│       ├── Controllers/            # Controller classes
│       │   └── DefaultPlayerController.cpp
│       ├── GameModes/              # Game mode classes
│       │   ├── DefaultGameMode.cpp # Main game mode
│       │   └── LobbyGameMode.cpp   # Multiplayer lobby
│       ├── GameStates/             # Game state classes
│       ├── Interfaces/             # C++ interfaces
│       │   ├── IInteractable.h     # Interactable objects
│       │   ├── IDetectable.h       # Detectable objects
│       │   └── IRequiresItem.h     # Item requirements
│       ├── Widgets/                # UI widgets
│       │   ├── HUD/                # In-game HUD
│       │   ├── MainMenu/           # Main menu UI
│       │   ├── Lobby/              # Multiplayer lobby UI
│       │   └── Calendar/           # Calendar puzzle UI
│       ├── DataAssets/             # Data-driven configuration
│       ├── ProceduralLevelGenerator.cpp # Procedural content generation
│       └── DefaultGameInstance.cpp # Game instance
└── EchoesOfTime.uproject           # Unreal project file
```

---

## Gameplay Systems

### Time Travel Mechanics

The core mechanic of Echoes of Time is the ability to switch between two timelines:

#### Timeline System (`ETimelineEra`)
- **Past Era**: Historical timeline where events have already occurred
- **Future Era**: Modern timeline where players can see echoes of the past

#### Ghost Character System
- Ghost characters display the recorded movements of players/NPCs from the Past timeline
- Visible in the Future timeline when using the Past Echo ability
- Semi-transparent meshes with dynamic material effects
- Used for puzzle-solving and navigation hints

#### Timeline-Specific Objects
- **Past Doors/Items**: Only accessible in the Past timeline
- **Future Doors/Items**: Only accessible in the Future timeline
- **Double Doors**: Special doors that exist in both timelines

**Key Classes**:
- `GhostCharacterActor`: Displays ghost echoes
- `PastDoor`, `FutureDoor`: Timeline-locked doors
- `PastItemPickup`, `FutureItemPickup`: Timeline-specific items
- `FutureGAPastEcho`: Ability to reveal ghost characters

---

### Stealth and Detection

Advanced detection system with multiple threat types:

#### Detection Mechanics
- **Progressive Detection**: Gradual detection filling from 0% to 100%
- **Line of Sight**: Guards and cameras require direct sight
- **Detection Angles**: Cameras and guards have limited field of view
- **Distance-Based**: Detection speed varies with distance
- **Illegal Items**: Carrying restricted items accelerates detection

#### Threat Types
- **Guards** (`GuardCharacter`): AI patrols with detection and combat capability
- **Civilians** (`CivilianCharacter`): Can witness illegal activity
- **Security Cameras** (`SecurityCamera`): Static detection with rotation
- **Metal Detectors** (`MetalDetector`): Detect weapons and illegal items

#### Detection Interface (`IDetectable`)
- `OnDetected`: Called when detection begins
- `OnLost`: Called when detection is lost
- `OnFullyDetected`: Called at 100% detection (alarm triggered)

#### Movement Options
- **Sprint**: Move quickly but increase detection speed
- **Crouch**: Reduce detection radius and move quietly
- **Walk**: Normal movement speed

**Key Classes**:
- `IDetectable`: Interface for detectable objects
- `SecurityCamera`: Camera detection logic
- `GuardCharacter`: AI guard behavior
- `DetectionWidget`: UI for detection feedback

---

### Ability System

Built on Unreal's Gameplay Ability System (GAS):

#### Core Abilities

**Hacking** (`DefaultGAHack`)
- Disable security cameras, computers, and electronic locks
- Mini-game mechanic with timer
- Requires specific items (hacking tools)
- Uses `HackComponent` and `HackAbilityTask`

**Lockpicking** (`DefaultGALockPick`)
- Open locked doors and containers
- Precision-based mini-game
- Requires lockpicks
- Uses `LockPickComponent` and `LockPickAbilityTask`

**Searching** (`DefaultGASearch`)
- Investigate desks, lockers, and containers
- Reveals items and documents
- Timer-based interaction
- Uses `SearchComponent` and `SearchAbilityTask`

**Past Echo** (`FutureGAPastEcho`)
- Toggle visibility of ghost characters in Future timeline
- No resource cost, unlimited duration
- Gameplay cue-based visual effects

**Weapon System** (`PistolGAFire`, `PistolGAAim`)
- Basic pistol firing mechanics
- Aim down sights
- Projectile-based bullets

#### Attribute System (`PlayerAttributeSet`)
- Health, stamina, and other player stats
- Uses Gameplay Effects for modifications

#### Gameplay Tags (`EOTGameplayTags`)
- `Character.Status.Illegal`: Marks player as carrying illegal items
- `Character.Status.PastEcho`: Indicates Past Echo ability is active
- `Input.*`: Input action tags for ability binding

**Key Classes**:
- `DefaultAbilitySystemComponent`: Custom ASC implementation
- `DefaultGASet`: Ability grant configuration
- `AbilityInputSet`: Input-to-ability mappings

---

### Inventory System

Dynamic inventory with hotkeys and item management:

#### Features
- **8 Inventory Slots**: Quick-access via number keys (1-8)
- **Active Item Display**: Currently equipped item visible on character
- **Item Dropping**: Press G to drop active item
- **Networked**: Fully replicated across clients
- **Illegal Item Tracking**: Automatically updates detection status

#### Item Types
- **Tools**: Lockpicks, hacking devices
- **Keycards**: Access cards for locked areas
- **Weapons**: Pistols and other firearms
- **Documents**: Newspapers, notes with clues
- **Collectibles**: Various items for puzzles

#### Item Pickup (`ItemPickup`)
- Interactable items in the world
- Static mesh display
- Automatic inventory addition on interaction

**Key Classes**:
- `InventoryComponent`: Manages player inventory
- `ItemPickup`: World items
- `FInventorySlot`: Data structure for inventory slots

---

### Puzzle Systems

Multiple puzzle types for varied gameplay:

#### Keypad Scanner
- Enter numeric codes to unlock doors
- Codes hidden in documents, computers, or NPCs
- Visual feedback for correct/incorrect entries
- **Classes**: `KeypadScanner`, `KeypadButton`, `CodeGenerator`

#### Wire Puzzles
- Connect colored wires in correct sequence
- Multiple wire colors and connection points
- Step-by-step progression
- **Classes**: `WirePuzzleManager`, `WireActor`, `WireDeviceActor`

#### Lever Combinations
- Activate levers in specific order
- Visible feedback for correct sequence
- Can trigger doors, gates, or other mechanisms
- **Classes**: `LeverManager`, `LeverActor`

#### Calendar Puzzles
- Use dates found in documents to unlock systems
- Research archives to find historical information
- Date validation against procedurally generated values
- **Classes**: `CalendarWidget`, `CalendarResultWidget`, `ArchiveComputer`

#### Laser Security
- Navigate around laser beams
- Disable using specific devices
- Triggers alarms on contact
- **Classes**: `LaserManager`, `LaserSensor`

---

### AI Characters

#### Guard AI (`GuardCharacter`)
- State Tree-based behavior
- Patrol routes using nav nodes
- Detection and pursuit
- Combat capabilities
- **Performance**: Tick disabled, NetUpdateFrequency 30Hz

#### Civilian AI (`CivilianCharacter`)
- Wander behavior around designated areas
- Can witness illegal activity
- Procedurally assigned names and desks
- **Performance**: Tick disabled, NetUpdateFrequency 10Hz

#### Security Cameras (`SecurityCamera`)
- Rotating surveillance
- Limited field of view
- Can be hacked to disable
- Wall-mounted with adjustable angle

---

## Multiplayer

### Network Architecture
- **Server-Authoritative**: All gameplay logic validated on server
- **Optimized Replication**: Variable update frequencies by actor type
  - Players: 60Hz
  - AI: 30Hz (Guards), 10Hz (Civilians)
  - Static objects: 1Hz
- **RPCs**: Client-to-Server communication for actions
- **Gameplay Cues**: Client-side visual effects

### Session Management
- **Advanced Sessions Plugin**: Robust multiplayer session creation
- **Friend System**: Invite friends via Steam or EOS
- **Lobby System**: Pre-game lobby with player list
- **Cross-Platform**: Steam and Epic Online Services support

### Multiplayer Features
- **Co-op Gameplay**: Team-based heist missions
- **Shared Objectives**: All players work toward common goals
- **Individual Inventories**: Each player has their own items
- **Synchronized Puzzles**: Puzzle states shared across clients

**Key Classes**:
- `LobbyGameMode`: Lobby session management
- `DefaultGameInstance`: Session creation/joining (Blueprint events)
- `LobbyUI`: Lobby interface
- `FriendList`, `FriendWidget`: Friend system UI

---

## Development Guide

### Building the Project

#### Prerequisites
1. Install Unreal Engine 5.7
2. Install Visual Studio 2022 with:
   - Desktop development with C++
   - Game development with C++
   - .NET Framework 4.7.2 targeting pack

#### Build Steps
```bash
# 1. Generate Visual Studio project files
# Right-click EchoesOfTime.uproject -> "Generate Visual Studio project files"

# 2. Open solution
# Open EchoesOfTime.sln in Visual Studio

# 3. Set configuration
# Configuration: Development Editor
# Platform: Win64

# 4. Build
# Build -> Build Solution (Ctrl+Shift+B)

# 5. Launch Editor
# Debug -> Start Without Debugging (Ctrl+F5)
# Or open EchoesOfTime.uproject directly
```

#### Common Build Issues
- **Missing Unreal Engine**: Verify UE 5.7 is installed and associated
- **SDK Errors**: Install latest Windows 10 SDK
- **Compilation Errors**: Clean solution and rebuild

---

### Code Architecture

#### Design Patterns
- **Component-Based**: Reusable components for abilities and interactions
- **Interface-Driven**: `IInteractable`, `IDetectable`, `IRequiresItem`
- **Data-Driven**: `DataAssets` for configuration
- **Event-Driven**: Delegates for UI updates and state changes

#### Key Architectural Decisions
1. **Gameplay Ability System**: Chosen for scalable, networked abilities
2. **State Tree AI**: Modern AI behavior system (replaces old Behavior Trees)
3. **Procedural Generation**: Ensures replayability and varied content
4. **Timeline Enum**: `ETimelineEra` for Past/Future distinction

#### C++ Coding Standards
- **Unreal Naming Conventions**:
  - `A` prefix for Actors (`ADefaultCharacter`)
  - `U` prefix for UObjects (`UInventoryComponent`)
  - `F` prefix for structs (`FInventorySlot`)
  - `E` prefix for enums (`ETimelineEra`)
  - `I` prefix for interfaces (`IInteractable`)
- **UFUNCTION Specifiers**: `BlueprintCallable`, `Server`, `Client`, `Reliable`
- **UPROPERTY Specifiers**: `Replicated`, `ReplicatedUsing`, `EditAnywhere`, `BlueprintReadOnly`

#### Network Replication Best Practices
- Always validate on server using `HasAuthority()`
- Use `ServerRPC_Implementation` for server functions
- Replicate only necessary data
- Use `ReplicatedUsing` for notification callbacks

---

### Performance Optimizations

Several optimizations have been implemented to maintain smooth performance:

#### Tick Optimizations
- **Tick Disabled**: Many actors don't need per-frame updates
  - `DefaultPlayerController`, `GuardCharacter`, `CivilianCharacter`
  - `KeypadScanner`, `CodeGenerator`, `DoorBase`
- **Timer-Based Updates**: Replace Tick with timers where possible
  - Detection updates: 100ms intervals (10Hz)
  - Interaction highlight: 100ms intervals (10Hz)
  - Code expiration checks: 500ms intervals (2Hz)

#### Network Optimizations
- **Variable Update Frequencies**:
  - Static actors: 1Hz (`NetUpdateFrequency = 1.0f`)
  - Infrequent actors: 10Hz (Civilians)
  - Moderate actors: 20Hz (Items)
  - Active AI: 30Hz (Guards)
  - Players: 60Hz (high responsiveness)

#### Other Optimizations
- **Cached Gameplay Tags**: Static const tags to avoid repeated queries
- **TMap Lookups**: O(1) key-to-slot mappings instead of if-else chains
- **Throttled Updates**: Accumulators for time-based throttling
- **Component Tick Intervals**: Set intervals on active components only

**Memory**: Repository memories contain extensive performance optimization details.

---

## Contributing

Contributions are welcome! To contribute:

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/my-feature`
3. **Commit your changes**: `git commit -am 'Add new feature'`
4. **Push to the branch**: `git push origin feature/my-feature`
5. **Create a Pull Request**

### Contribution Guidelines
- Follow Unreal Engine C++ coding standards
- Write clear commit messages
- Test your changes in multiplayer if network-related
- Update documentation for new features
- Ensure code compiles without warnings

---

## License

This project is proprietary. All rights reserved.

For licensing inquiries, please contact the repository owner.

---

## Acknowledgments

- **Unreal Engine 5** by Epic Games
- **Advanced Sessions Plugin** for multiplayer functionality
- **Gameplay Ability System** for robust ability mechanics
- **Steam** and **Epic Online Services** for multiplayer backend

---

## Contact

- **Repository**: [Echoes-of-Time](https://github.com/Traveler3114/Echoes-of-Time)
- **Issues**: [Report a bug or request a feature](https://github.com/Traveler3114/Echoes-of-Time/issues)

---

*Echoes of Time - Master time, master the heist.*
