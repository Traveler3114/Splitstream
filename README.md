# Echoes of Time

A multiplayer stealth heist game built in Unreal Engine 5.7 featuring unique time-travel mechanics where players navigate between Past and Future timelines to solve puzzles, evade detection, and complete objectives.

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue)](https://www.unrealengine.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue)](https://www.microsoft.com/windows)
[![Game Engine](https://img.shields.io/badge/Made%20with-Unreal%20Engine-informational)](https://www.unrealengine.com/)
[![Gameplay Ability System](https://img.shields.io/badge/GAS-Enabled-success)](https://docs.unrealengine.com/5.7/gameplay-ability-system-for-unreal-engine/)
[![Multiplayer](https://img.shields.io/badge/Multiplayer-Co--op-orange)](https://github.com/Traveler3114/Echoes-of-Time)

> **🎮 Master time, master the heist.**

![Echoes of Time Banner](docs/images/banner.png)
*⚠️ Screenshot placeholder - Add your game banner image at `docs/images/banner.png`*

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Key Features](#key-features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Controls & Input](#controls--input)
  - [Keyboard & Mouse](#keyboard--mouse)
  - [Gamepad](#gamepad)
- [Gameplay Guide](#gameplay-guide)
  - [Getting Started](#getting-started)
  - [Tips & Strategies](#tips--strategies)
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
  - [Debugging Tips](#debugging-tips)
  - [Testing Strategies](#testing-strategies)
  - [Blueprint vs C++](#blueprint-vs-c)
  - [Performance Optimizations](#performance-optimizations)
- [Configuration Files](#configuration-files)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)
- [FAQ](#faq)
- [Version History](#version-history)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)
- [Contact](#contact)

---

## Overview

**Echoes of Time** is a stealth-based heist game where players use time manipulation to complete missions. Players can toggle between two timelines—Past and Future—to discover clues, avoid detection, and solve complex puzzles. The game features procedurally generated content, ensuring each playthrough offers unique challenges.

The game utilizes Unreal Engine's Gameplay Ability System (GAS) for robust and extensible ability mechanics, supports multiplayer sessions via Steam and Epic Online Services (EOS), and features optimized network replication for smooth online play.

---

## Quick Start

Want to jump right in? Here's the fastest way to get playing:

### For Players

```bash
# 1. Clone the repository
git clone https://github.com/Traveler3114/Echoes-of-Time.git
cd Echoes-of-Time

# 2. Generate project files (Windows)
# Right-click EchoesOfTime.uproject → "Generate Visual Studio project files"

# 3. Open EchoesOfTime.uproject in Unreal Engine 5.7

# 4. Press Alt+P or click Play to start!
```

### Video Tutorial

[![Watch the Setup Tutorial](docs/images/video-thumbnail.png)](https://youtu.be/placeholder)
*⚠️ Video placeholder - Add a setup tutorial video link*

### Quick Controls

| Action | Key |
|--------|-----|
| Move | WASD |
| Look | Mouse |
| Interact | E |
| Sprint | Left Shift |
| Crouch | Left Ctrl |
| Switch Timeline | T |
| Drop Item | G |
| Inventory Slots | 1-8 |

**Full controls available in [Controls & Input](#controls--input) section.**

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

## Controls & Input

Echoes of Time supports both keyboard/mouse and gamepad input with full customization available in the in-game settings menu.

### Keyboard & Mouse

#### Movement & Camera
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Move Forward | W | Move character forward |
| Move Backward | S | Move character backward |
| Move Left | A | Strafe left |
| Move Right | D | Strafe right |
| Look Around | Mouse | Camera control |
| Sprint | Left Shift | Run faster (increases detection) |
| Crouch | Left Ctrl | Move quietly (decreases detection) |
| Jump | Space | Jump over obstacles |

#### Interactions
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Interact | E | Interact with objects, doors, NPCs |
| Use Ability | Mouse Button 2 (Right Click) | Context-sensitive ability use |
| Cancel/Back | ESC | Cancel action or open menu |

#### Timeline & Abilities
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Switch Timeline | T | Toggle between Past and Future eras |
| Past Echo (Future only) | Q | Toggle ghost character visibility |
| Hack | Hold E on hackable object | Initiate hacking minigame |
| Lockpick | Hold E on locked door | Initiate lockpicking minigame |
| Search | Hold E on searchable object | Search desks, lockers, containers |

#### Inventory & Items
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Inventory Slot 1-8 | 1, 2, 3, 4, 5, 6, 7, 8 | Quick-select inventory items |
| Drop Active Item | G | Drop currently equipped item |
| Open Inventory | I | Open full inventory screen |

#### Combat (When Equipped)
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Fire Weapon | Mouse Button 1 (Left Click) | Fire equipped weapon |
| Aim Down Sights | Mouse Button 2 (Right Click) | Precision aiming |
| Reload | R | Reload weapon |

#### UI & Menu
| Action | Key Binding | Description |
|--------|-------------|-------------|
| Pause Menu | ESC | Open pause menu |
| Tab Menu | Tab | Open objectives/mission info |
| Show Scoreboard | Hold Tab | (Multiplayer) Show player list |
| Toggle Chat | Enter | (Multiplayer) Open text chat |

### Gamepad

#### Movement & Camera
| Action | Button | Description |
|--------|--------|-------------|
| Move | Left Stick | Character movement |
| Look Around | Right Stick | Camera control |
| Sprint | Left Stick Press (L3) | Toggle sprint |
| Crouch | Right Stick Press (R3) | Toggle crouch |
| Jump | A Button (Xbox) / X (PlayStation) | Jump |

#### Interactions & Abilities
| Action | Button | Description |
|--------|--------|-------------|
| Interact | X Button (Xbox) / Square (PlayStation) | Context interaction |
| Switch Timeline | Y Button (Xbox) / Triangle (PlayStation) | Toggle timeline |
| Use Ability | Left Bumper (LB) | Context-sensitive ability |
| Drop Item | D-Pad Down | Drop active item |

#### Combat
| Action | Button | Description |
|--------|--------|-------------|
| Fire | Right Trigger (RT) | Fire weapon |
| Aim | Left Trigger (LT) | Aim down sights |
| Reload | X Button (Xbox) / Square (PlayStation) | Reload weapon |

#### Inventory
| Action | Button | Description |
|--------|--------|-------------|
| Cycle Inventory | D-Pad Left/Right | Cycle through inventory slots |
| Open Inventory | Back/Select Button | Open full inventory |

#### Menu
| Action | Button | Description |
|--------|--------|-------------|
| Pause | Start/Options Button | Open pause menu |
| Scoreboard | Back/Select Button (Hold) | Show player list |

### Input Customization

You can customize all key bindings through the in-game Settings menu:
1. Press **ESC** to open the pause menu
2. Navigate to **Settings** → **Controls**
3. Click on any action to rebind
4. Press the desired key or button
5. Click **Apply** to save changes

**Note**: Some actions are context-sensitive and will change based on what object you're looking at or what item you have equipped.

---

## Gameplay Guide

### Getting Started

#### Your First Mission

1. **Understand Your Objective**
   - Check the mission briefing at the start
   - Open the Tab menu to review objectives
   - Look for highlighted interaction points

2. **Learn the Timeline Mechanic**
   - Press **T** to switch between Past and Future
   - Objects and paths differ between timelines
   - Information discovered in one timeline helps solve puzzles in the other

3. **Stay Undetected**
   - Use **Crouch** (Left Ctrl) to reduce detection radius
   - Avoid guard line of sight (red cones)
   - Don't run near guards unless necessary
   - Hide behind cover when detected

4. **Collect Information**
   - **Search** desks and containers for clues
   - Read newspapers and documents for codes
   - Talk to NPCs for hints
   - Use computers to research dates and information

5. **Solve Puzzles**
   - Keypad codes are hidden in documents
   - Wire puzzles require connecting matching colors
   - Calendar puzzles need dates from archives
   - Use information from Past timeline in Future timeline

#### Understanding Detection

**Detection States:**
- **Safe** (Green): Not detected, can move freely
- **Suspicious** (Yellow): Guard is investigating, freeze or hide
- **Detected** (Red): Guard has spotted you, alarm will sound at 100%

**How to Avoid Detection:**
- Stay in shadows and behind cover
- Crouch when near guards or cameras
- Don't carry illegal items in restricted areas
- Disable cameras using hacking ability
- Take alternate routes through vents

**What Increases Detection:**
- Running/sprinting near guards
- Carrying illegal items (weapons, tools)
- Wearing wrong timeline clothing
- Being in restricted areas
- Standing in camera view

### Tips & Strategies

#### Stealth Mastery

**General Tips:**
- 🎯 Always scout ahead before moving through an area
- 🎯 Learn guard patrol patterns before acting
- 🎯 Use the Past timeline to see building layouts without guards
- 🎯 Crouch is your friend—use it liberally
- 🎯 If detected, break line of sight and hide until alarm clears

**Timeline Strategy:**
- 📅 Switch to Past to find historical documents and clues
- 📅 Use Future's Past Echo ability to see where people walked
- 📅 Some doors only exist in one timeline—use that to your advantage
- 📅 Items from one timeline may be needed in the other

**Item Management:**
- 🎒 Drop illegal items before entering restricted areas
- 🎒 Pick them up again on the other side
- 🎒 Keycards are essential—never drop them unless necessary
- 🎒 Tools (lockpicks, hacking devices) are reusable
- 🎒 Organize your hotbar with frequently used items in slots 1-4

**Puzzle Solving:**
- 🧩 Always check nearby documents first before searching elsewhere
- 🧩 Wire puzzles: Start from the power source and work outward
- 🧩 Keypad codes: Check newspapers, computers, and NPC conversations
- 🧩 Lever puzzles: Look for visual indicators showing the correct sequence
- 🧩 Calendar puzzles: Archive computers have all the dates you need

**Multiplayer Coordination:**
- 👥 Split up to cover more ground faster
- 👥 One player scouts while another solves puzzles
- 👥 Share discovered codes via voice chat
- 👥 Coordinate timeline switches for complex puzzles
- 👥 Revive downed teammates quickly to avoid mission failure

#### Advanced Techniques

**Ghost Character Analysis:**
- In Future timeline, use **Past Echo** (Q) to see movement patterns
- Ghost characters show where people walked in the Past
- Follow ghost paths to find hidden areas
- Use ghosts to time guard patrols perfectly

**Speed Running:**
- Memorize code locations from previous runs
- Skip optional objectives for faster completion
- Use sprint strategically only in safe zones
- Master the lockpick minigame for quick door access

**Perfect Stealth (No Detections):**
- Disable all cameras before entering areas
- Hack metal detectors to pass with illegal items
- Use vents and alternate routes exclusively
- Never carry weapons in public areas
- Complete levels without triggering any alarms for bonus rewards

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

### Debugging Tips

#### Common Debugging Scenarios

**1. Replication Issues**
```cpp
// Add to any replicated function to debug
UE_LOG(LogTemp, Warning, TEXT("Function called on: %s"), 
    HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));

// Check if actor is replicated
UE_LOG(LogTemp, Warning, TEXT("Actor %s - Role: %d, RemoteRole: %d"), 
    *GetName(), GetLocalRole(), GetRemoteRole());
```

**2. Gameplay Ability System Debugging**
```cpp
// Enable GAS verbose logging
ShowDebug AbilitySystem  // In console

// Check active gameplay tags
GetAbilitySystemComponent()->GetOwnedGameplayTags().ToStringSimple()

// Debug ability activation
UE_LOG(LogAbilitySystem, Log, TEXT("Activating ability: %s"), *GetName());
```

**3. Timeline Issues**
```cpp
// Verify timeline era
UE_LOG(LogTemp, Display, TEXT("Current Timeline: %s"), 
    CurrentTimeline == ETimelineEra::Past ? TEXT("Past") : TEXT("Future"));
```

**4. Detection System Debugging**
```cpp
// In SecurityCamera or GuardCharacter
DrawDebugLine(GetWorld(), GetActorLocation(), TargetLocation, 
    FColor::Red, false, 5.0f);  // Visualize detection rays
```

#### Useful Console Commands

```bash
# Gameplay
stat fps                    # Show FPS
stat unit                   # Show frame time breakdown
stat game                   # Show game thread stats
stat net                    # Show network stats
stat ai                     # Show AI performance

# Debugging
ShowDebug AbilitySystem     # GAS debugging
ShowDebug Animation         # Animation debugging
ShowDebug Collision         # Collision visualization
ShowDebug Navigation        # Navmesh visualization

# Networking
NetPktLoss=10              # Simulate 10% packet loss
NetPktLag=100              # Simulate 100ms latency
NetGraph 1                 # Show network graph

# Cheats (Single Player / Server)
God                        # Invincibility
Ghost                      # No collision movement
Teleport                   # Teleport to cursor location
```

#### Visual Studio Debugging

**Breakpoint Best Practices:**
- Use conditional breakpoints for specific cases
  - Right-click breakpoint → Conditions → `bIsPlayerControlled == true`
- Use Tracepoints instead of UE_LOG for temporary logging
- Set breakpoints on `check()` and `ensure()` macros

**Useful Watch Expressions:**
```cpp
// Cast to specific type in watch window
((ADefaultCharacter*)this)->CurrentTimeline

// View gameplay tags
AbilitySystemComponent->ActiveGameplayTags

// Check replication
GetLocalRole()  // ROLE_Authority on server
```

**Attach to Running Process:**
1. Launch game from Unreal Editor
2. Visual Studio → Debug → Attach to Process
3. Select `UnrealEditor-Win64-DebugGame.exe`
4. Set breakpoints and play

---

### Testing Strategies

#### Unit Testing

**C++ Tests** (not currently implemented, but recommended):
```cpp
// Example test structure
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTest, 
    "EchoesOfTime.Inventory.AddItem", 
    EAutomationTestFlags::ApplicationContextMask | 
    EAutomationTestFlags::ProductFilter)

bool FInventoryTest::RunTest(const FString& Parameters)
{
    // Test inventory adding items
    // ...
    return true;
}
```

#### Manual Testing Checklist

**Single Player:**
- [ ] All abilities activate correctly
- [ ] Timeline switching works in all levels
- [ ] All puzzle types are solvable
- [ ] Detection system responds appropriately
- [ ] Inventory management functions properly
- [ ] Save/load works correctly

**Multiplayer:**
- [ ] Host can create sessions
- [ ] Clients can join sessions
- [ ] All players see synchronized game state
- [ ] Abilities replicate correctly
- [ ] Puzzle states sync between clients
- [ ] Detection alerts all players
- [ ] Inventory items don't duplicate

**Performance:**
- [ ] 60+ FPS in normal gameplay
- [ ] No stuttering during timeline switches
- [ ] AI pathfinding doesn't cause hitches
- [ ] Network latency under 100ms feels responsive

#### Automated Testing Tools

**Unreal Insights:**
```bash
# Launch with profiling
UnrealEditor.exe EchoesOfTime.uproject -game -trace=cpu,loadtime,counters

# Open Unreal Insights to analyze
UnrealInsights.exe
```

**Gauntlet Test Automation:**
- Located in `Engine/Source/Programs/AutomationTool/Gauntlet`
- Can automate level loading and basic gameplay

**Session Frontend:**
- Unreal Editor → Window → Developer Tools → Session Frontend
- Profile game performance in real-time
- Capture network traffic and stats

---

### Blueprint vs C++

#### When to Use Blueprint

**Advantages:**
- ✅ Rapid prototyping and iteration
- ✅ Visual scripting for designers
- ✅ Hot reloading without recompilation
- ✅ Easy to tweak parameters

**Best Use Cases:**
- UI widgets and menus
- Simple actor logic
- Level-specific scripting
- Prototyping new features
- Configuring C++ classes
- Gameplay tweaking and balancing

**Example: Blueprint Subclass**
```cpp
// C++ Base Class
UCLASS(Blueprintable)
class ACustomDoor : public ADoorBase
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float OpenSpeed = 1.0f;
};

// Blueprint can inherit and set OpenSpeed in editor
// BP_CustomDoor → Set OpenSpeed to 2.0
```

#### When to Use C++

**Advantages:**
- ✅ Better performance for complex logic
- ✅ Full access to engine API
- ✅ Better version control and code review
- ✅ Compile-time error checking

**Best Use Cases:**
- Core gameplay systems
- Network replication logic
- Performance-critical code
- Complex algorithms
- Reusable components
- Gameplay Ability System implementation

**Example: C++ Component**
```cpp
// Create reusable component in C++
UCLASS()
class UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
    
    UFUNCTION(BlueprintCallable)
    void AddItem(UItemBase* Item);
    
    UFUNCTION(BlueprintCallable)
    bool RemoveItem(int32 SlotIndex);
};

// Use in Blueprint actor without writing C++
```

#### Hybrid Approach (Recommended)

**Best Practice Pattern:**
1. **C++ Base Classes** with `Blueprintable` specifier
2. **Blueprint Subclasses** for specific implementations
3. **C++ Components** for reusable functionality
4. **Blueprint Events** for level-specific triggers

**Example Workflow:**
```
DefaultCharacter.h/cpp (C++)         → BP_PlayerCharacter (Blueprint)
    ↓ Defines core functionality         ↓ Sets mesh, animations, materials
    ↓ Handles replication                 ↓ Configures ability set
    ↓ Implements ability system           ↓ Tweaks movement parameters
```

#### Performance Considerations

**Blueprint Performance Tips:**
- Use Nativized blueprints for shipping builds
- Avoid Tick for simple checks—use Timers instead
- Cache references instead of repeated casts
- Use Blueprint Interfaces for polymorphism
- Minimize Event Tick usage

**When to Refactor Blueprint to C++:**
- Event Tick with complex math operations
- Nested loops processing arrays
- Called hundreds of times per frame
- Complex collision/physics calculations
- Core gameplay systems used everywhere

---

## Configuration Files

### Overview of Config Files

All configuration files are located in the `Config/` directory:

#### `DefaultEngine.ini`
Primary engine configuration file.

**Key Settings:**
```ini
[/Script/Engine.Engine]
# Network settings
NetClientTicksPerSecond=60
MaxClientRate=100000
MaxInternetClientRate=50000

# Rendering
DefaultGraphicsPerformance=Maximum
DefaultScreenResolution=1920x1080

# Physics
PhysicsSubstepCount=2
```

**Multiplayer Configuration:**
```ini
[/Script/OnlineSubsystemEOS.NetDriverEOS]
MaxInternetClientRate=50000
MaxClientRate=100000
NetServerMaxTickRate=60

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
```

#### `DefaultGame.ini`
Game-specific settings and project configuration.

**Key Settings:**
```ini
[/Script/EngineSettings.GeneralProjectSettings]
ProjectID=(A=1234567890,B=987654321)
ProjectName=Echoes of Time
ProjectVersion=0.1.0

[/Script/UnrealEd.ProjectPackagingSettings]
BuildConfiguration=PPBC_Shipping
FullRebuild=False
```

#### `DefaultGameplayTags.ini`
Defines all gameplay tags used in the project.

**Tag Categories:**
- `Character.*` - Character states and abilities
- `Character.Status.*` - Status effects (Illegal, PastEcho, etc.)
- `Character.Ability.*` - Ability identifiers
- `Input.*` - Input action tags
- `Weapon.*` - Weapon-related tags
- `StateTree.Event.*` - AI state tree events

**Example Tags:**
```ini
GameplayTagList=(Tag="Character.Status.Illegal",DevComment="Carrying illegal items")
GameplayTagList=(Tag="Character.Ability.Hack",DevComment="Hacking ability")
GameplayTagList=(Tag="Input.Ability.Hack",DevComment="Hack input binding")
```

#### `DefaultInput.ini`
Input mappings and controller configurations.

**Enhanced Input System:**
This project uses Unreal's Enhanced Input System with Input Mapping Contexts defined as Data Assets.

**Input Configuration Location:**
- C++ Headers: `Source/EchoesOfTime/DataAssets/InputMappingSet.h`
- Blueprint Assets: `Content/Input/`

**Modifying Controls:**
1. Open input mapping context in editor
2. Add/modify input actions
3. Bind to gameplay tags in ability input set
4. Compile and test

#### `DefaultEditor.ini`
Editor-specific settings (not used at runtime).

**Useful Settings:**
```ini
[/Script/UnrealEd.EditorPerformanceSettings]
bShowFrameRateAndMemory=True
bThrottleCPUWhenNotForeground=True
```

### Modifying Configuration

**Best Practices:**
1. ⚠️ **Never commit local overrides** - Use `DefaultGame.ini` not `DefaultGameLocal.ini`
2. 📝 **Document changes** - Add comments explaining why settings were changed
3. 🧪 **Test thoroughly** - Config changes can have unintended effects
4. 🔄 **Use version control** - Track all config changes in git
5. 👥 **Communicate** - Notify team of significant config changes

**Common Modifications:**

**Increase Network Update Rate:**
```ini
[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate=60  # Increase from 30 for smoother multiplayer
```

**Change Resolution:**
```ini
[SystemSettings]
r.SetRes=1920x1080f  # f for fullscreen, w for windowed
```

**Adjust Detection Settings:**
Modify in C++ or blueprint, not in config files:
- `SecurityCamera::DetectionSpeed`
- `GuardCharacter::MaxDetectionDistance`

---

## API Reference

### Core Classes

#### Character Classes

**`ADefaultCharacter`**
Main player character with ability system and timeline switching.

```cpp
// Key Methods
void SwitchTimeline();                              // Toggle Past/Future
void ServerSwitchTimeline_Implementation();         // Server RPC
void OnRep_CurrentTimeline();                       // Timeline replication callback
UAbilitySystemComponent* GetAbilitySystemComponent(); // GAS accessor
```

**`AGuardCharacter`**
AI guard with detection and patrol behavior.

```cpp
// Key Properties
float MaxDetectionDistance;      // How far guard can detect
float DetectionAngle;            // Field of view angle
TArray<ANavNode*> PatrolPoints;  // Patrol route

// Key Methods
void OnDetectPawn(APawn* Target);      // Called when target spotted
void OnLostPawn(APawn* Target);        // Called when target lost
```

**`ACivilianCharacter`**
Non-hostile NPC that can witness illegal activity.

```cpp
// Key Properties
FString CivilianName;           // Procedurally generated name
ADeskActor* AssignedDesk;       // Where civilian works

// Key Methods
void WitnessIllegalActivity(AActor* Actor);  // Reports to guards
```

#### Actor Components

**`UInventoryComponent`**
Manages player inventory with 8 slots.

```cpp
// Key Methods
bool AddItem(UItemBase* Item);                    // Add item to inventory
bool RemoveItem(int32 SlotIndex);                 // Remove from slot
void ActivateSlot(int32 SlotIndex);               // Equip item
UItemBase* GetActiveItem();                       // Get equipped item
void DropActiveItem();                            // Drop equipped item

// Key Events
UPROPERTY(BlueprintAssignable)
FOnInventoryUpdated OnInventoryUpdated;          // UI update event
```

**`UHackComponent`**
Handles hacking interactions.

```cpp
// Key Methods
void StartHacking(AActor* Target);                // Begin hack
void CompleteHack();                              // Successful hack
void CancelHack();                                // Cancel/fail hack

// Properties
float HackDuration;                               // Time to complete
bool bRequiresHackingDevice;                      // Item requirement
```

**`ULockPickComponent`**
Manages lockpicking minigame.

```cpp
// Key Methods
void StartLockPick(ADoorBase* Door);              // Begin lockpick
void UpdateLockPickProgress(float DeltaTime);     // Update minigame
void CompleteLockPick();                          // Success
void FailLockPick();                              // Failure

// Properties
int32 LockDifficulty;                             // 1-5 difficulty
float SuccessZoneSize;                            // Minigame tolerance
```

#### Actors

**`ADoorBase`**
Base class for all doors.

```cpp
// Key Methods
void Open();                                      // Open door
void Close();                                     // Close door
void Toggle();                                    // Toggle state
void Lock();                                      // Lock door
void Unlock();                                    // Unlock door
bool CanOpen(AActor* Instigator);                // Permission check

// Properties
bool bIsLocked;                                   // Lock state
bool bRequiresKeycard;                            // Keycard requirement
ETimelineEra TimelineEra;                         // Past/Future/Both
```

**`ASecurityCamera`**
Surveillance camera with detection.

```cpp
// Key Methods
void StartDetection(AActor* Target);              // Begin detecting
void UpdateDetection(float DeltaTime);            // Increase detection %
void LoseTarget();                                // Target left view
void Disable(float Duration);                     // Hacked

// Properties
float DetectionRange;                             // Max distance
float DetectionAngle;                             // FOV
float DetectionSpeed;                             // Detection rate
bool bIsHacked;                                   // Disabled state
```

**`AItemPickup`**
Pickup item in world.

```cpp
// Key Properties
UItemBase* ItemData;                              // Data asset reference
UStaticMeshComponent* MeshComponent;              // Visual representation

// Key Methods
void OnInteract(AActor* Instigator);              // Pickup item
```

#### Gameplay Ability System

**`UDefaultAbilitySystemComponent`**
Custom ASC implementation.

```cpp
// Inherited from UAbilitySystemComponent
FGameplayAbilitySpecHandle GiveAbility(const FGameplayAbilitySpec& Spec);
bool TryActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);
void CancelAllAbilities();

// Custom additions
void GrantAbilitySet(UAbilityInputSet* AbilitySet);
void RemoveAbilitySet(UAbilityInputSet* AbilitySet);
```

**Gameplay Ability Base Classes:**
- `UDefaultGAHack` - Hacking ability
- `UDefaultGALockPick` - Lockpicking ability
- `UDefaultGASearch` - Searching ability
- `UFutureGAPastEcho` - Past echo ability (Future only)
- `UPistolGAFire` - Weapon firing
- `UPistolGAAim` - Weapon aiming

#### Interfaces

**`IInteractable`**
Interface for interactive objects.

```cpp
// Methods to implement
UFUNCTION(BlueprintNativeEvent)
void OnInteract(AActor* Instigator);              // Interaction behavior

UFUNCTION(BlueprintNativeEvent)
FText GetInteractionText();                       // UI prompt text

UFUNCTION(BlueprintNativeEvent)
bool CanInteract(AActor* Instigator);             // Permission check
```

**`IDetectable`**
Interface for objects that can be detected by guards/cameras.

```cpp
// Methods to implement
UFUNCTION(BlueprintNativeEvent)
void OnDetected(AActor* Detector);                // Detection started

UFUNCTION(BlueprintNativeEvent)
void OnLost(AActor* Detector);                    // Detection lost

UFUNCTION(BlueprintNativeEvent)
void OnFullyDetected(AActor* Detector);           // 100% detected
```

**`IRequiresItem`**
Interface for objects requiring specific items.

```cpp
// Methods to implement
UFUNCTION(BlueprintNativeEvent)
EItemType GetRequiredItemType();                  // Required item

UFUNCTION(BlueprintNativeEvent)
bool HasRequiredItem(AActor* Instigator);         // Check possession
```

### Enums and Structs

**`ETimelineEra`**
```cpp
enum class ETimelineEra : uint8
{
    Past    UMETA(DisplayName = "Past"),
    Future  UMETA(DisplayName = "Future")
};
```

**`EItemType`**
```cpp
enum class EItemType : uint8
{
    KeycardL1, KeycardL2, Fingerprint, MoneyStack,
    Tool, Gadget, Pistol, PowerCell, BodyBag, Other
};
```

**`FInventorySlot`**
```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()
    
    UPROPERTY()
    UItemBase* ItemData;                          // Item data asset
    
    UPROPERTY()
    FGuid ItemInstanceID;                         // Unique instance ID
    
    UPROPERTY()
    int32 Quantity;                               // Stack size
};
```

### Events and Delegates

**Inventory Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, 
    UInventoryComponent*, Inventory);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, 
    UItemBase*, Item, int32, SlotIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, 
    UItemBase*, Item, int32, SlotIndex);
```

**Detection Events:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionStarted, 
    AActor*, Detector);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDetectionUpdated, 
    AActor*, Detector, float, DetectionPercent);
```

---

## Troubleshooting

### Common Issues

#### Build Errors

**Error: "Unreal Engine 5.7 not found"**
```
Solution:
1. Verify UE 5.7 is installed via Epic Games Launcher
2. Right-click .uproject → "Switch Unreal Engine version"
3. Select 5.7
4. Generate project files again
```

**Error: "Missing modules" or "Plugin not found"**
```
Solution:
1. Verify all plugins are enabled in .uproject
2. Check Plugins directory exists and has correct plugins
3. Rebuild solution in Visual Studio
4. If issue persists, delete Binaries, Intermediate folders and rebuild
```

**Error: C++ compilation errors in GAS code**
```
Solution:
1. Ensure GameplayAbilities plugin is enabled
2. Verify modules in Build.cs include "GameplayAbilities", "GameplayTags", "GameplayTasks"
3. Clean and rebuild solution
```

#### Runtime Issues

**Issue: Timeline switching doesn't work**
```
Debug Steps:
1. Check CurrentTimeline variable is replicating
2. Verify OnRep_CurrentTimeline is being called
3. Ensure timeline-specific actors have correct TimelineEra set
4. Check log for "Switch Timeline" messages

Console Command:
ShowDebug State  // Displays current timeline
```

**Issue: Abilities not activating**
```
Debug Steps:
1. Verify AbilitySystemComponent is initialized
2. Check ability is granted in BeginPlay
3. Confirm gameplay tags are correctly configured
4. Ensure input is bound to correct ability tag

Console Command:
ShowDebug AbilitySystem  // Shows active abilities and tags
```

**Issue: Items not replicating**
```
Debug Steps:
1. Check ItemPickup has bReplicates = true
2. Verify inventory component is replicated
3. Confirm server is authoritative for inventory changes
4. Check OnRep_ functions are firing

Add Debug Log:
UE_LOG(LogTemp, Warning, TEXT("AddItem called on: %s"), 
    HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
```

**Issue: Detection not working**
```
Debug Steps:
1. Verify IDetectable interface is implemented
2. Check detection range/angle values
3. Ensure line of sight isn't blocked
4. Confirm gameplay tags for illegal status

Visual Debug:
DrawDebugLine in SecurityCamera/GuardCharacter code
```

#### Multiplayer Issues

**Issue: Can't create/join sessions**
```
Solution:
1. Check Steam is running (for Steam builds)
2. Verify EOS credentials are configured
3. Test in Editor: Play as "Listen Server" with 2+ players
4. Check firewall isn't blocking ports
5. Verify OnlineSubsystem is correctly configured in DefaultEngine.ini
```

**Issue: Lag or rubber-banding**
```
Solutions:
1. Check network stats: stat net
2. Reduce NetUpdateFrequency for non-critical actors
3. Verify server tick rate is adequate (60Hz recommended)
4. Use network profiler to find bandwidth issues
5. Implement client-side prediction for movement
```

**Issue: Clients see different game state**
```
Debug Steps:
1. Ensure all gameplay variables are replicated
2. Use ReplicatedUsing for notification callbacks
3. Verify server is authoritative for state changes
4. Check that RPCs are marked Reliable where needed
5. Test with network emulation: NetPktLoss, NetPktLag
```

#### Performance Issues

**Issue: Low FPS (below 60)**
```
Debug Steps:
1. Run stat unit - identify bottleneck (Game/Draw/GPU)
2. stat game - check for expensive game logic
3. stat actors - find actors with high tick time
4. Profile with Unreal Insights

Solutions:
- Disable tick on static actors
- Reduce NetUpdateFrequency
- Optimize materials (reduce complexity)
- Use LODs for distant meshes
- Cull objects outside camera view
```

**Issue: Stuttering during timeline switch**
```
Solutions:
1. Pre-load timeline-specific assets
2. Use asset streaming
3. Reduce number of actors spawned during switch
4. Profile with Unreal Insights to find hitches
```

### Error Messages

**"CHECK FAILED: AbilitySystemComponent != nullptr"**
```
Cause: Trying to use GAS before it's initialized
Solution: Ensure AbilitySystemComponent is created in constructor
          and initialized in BeginPlay
```

**"Ability failed to activate: Missing required tag"**
```
Cause: Ability requires a gameplay tag the character doesn't have
Solution: Grant required tag via gameplay effect or ensure
          character has the necessary state
```

**"Server RPC rejected: Not validated"**
```
Cause: RPC validation failed or actor not replicated
Solution: Verify bReplicates = true
          Check _Validate function returns true
          Ensure proper server authority
```

### Getting Help

If you encounter issues not covered here:

1. **Check Logs**: `Saved/Logs/EchoesOfTime.log`
2. **Enable Verbose Logging**: Add to DefaultEngine.ini
   ```ini
   [Core.Log]
   LogAbilitySystem=VeryVerbose
   LogNet=Verbose
   LogNetTraffic=Verbose
   ```
3. **Search GitHub Issues**: [Issues Page](https://github.com/Traveler3114/Echoes-of-Time/issues)
4. **Create New Issue**: Provide logs, repro steps, and system info
5. **Join Community**: [Discord Server](#) *(placeholder)*

---

## FAQ

### General Questions

**Q: What is Echoes of Time?**
A: Echoes of Time is a multiplayer stealth heist game built in Unreal Engine 5.7 featuring time-travel mechanics where players switch between Past and Future timelines to solve puzzles and complete objectives.

**Q: Is this game free?**
A: The project is currently in development. Licensing and distribution details are proprietary—contact the repository owner for more information.

**Q: What platforms are supported?**
A: Currently Windows 10/11 64-bit. Future platform support is under consideration.

**Q: Can I play single-player?**
A: Yes! The game supports both single-player and multiplayer co-op gameplay.

**Q: How many players can play together?**
A: The game is designed for 1-4 players in co-op mode.

### Gameplay Questions

**Q: How do I switch timelines?**
A: Press **T** to toggle between Past and Future timelines.

**Q: What happens if I get detected?**
A: Guards will investigate and if detection reaches 100%, an alarm sounds. Multiple alarms may result in mission failure depending on the objective.

**Q: Can I save my progress?**
A: Currently, saves are checkpoint-based within missions. Full save/load system is planned for future updates.

**Q: Are there difficulty settings?**
A: Not yet implemented, but planned for future versions with adjustable detection speeds and puzzle complexity.

**Q: Can I customize my character?**
A: Character customization is planned for future updates.

### Technical Questions

**Q: What version of Unreal Engine do I need?**
A: Unreal Engine 5.7 is required. Earlier versions are not compatible.

**Q: Do I need Visual Studio?**
A: Yes, Visual Studio 2022 is required for C++ development. For blueprint-only work, you can use the editor without VS.

**Q: Can I use Rider instead of Visual Studio?**
A: Yes, JetBrains Rider is fully supported as an alternative IDE.

**Q: Does this project use Blueprints or C++?**
A: Both! Core systems are in C++ with Blueprint subclasses for specific implementations and rapid iteration.

**Q: What networking backend does the game use?**
A: The game supports both Steam Sockets and Epic Online Services (EOS) for cross-platform multiplayer.

**Q: How much disk space is required?**
A: Approximately 10 GB for the project, plus 50-60 GB for Unreal Engine 5.7.

**Q: What graphics card do I need?**
A: Minimum: DirectX 11/12 compatible GPU with 2GB VRAM. Recommended: NVIDIA GTX 1060 / AMD RX 580 or better.

### Development Questions

**Q: Can I contribute to this project?**
A: See the [Contributing](#contributing) section for guidelines on how to contribute.

**Q: Where should I report bugs?**
A: Report bugs on the [GitHub Issues page](https://github.com/Traveler3114/Echoes-of-Time/issues).

**Q: How do I add new abilities?**
A: Create a new C++ class inheriting from `UGameplayAbility`, implement activation logic, and grant via ability set. See existing abilities in `Source/EchoesOfTime/AbilitySystem/Abilities/`.

**Q: How do I create new items?**
A: Create a Data Asset derived from `UItemBase`, configure properties, and create an `AItemPickup` instance referencing the data asset.

**Q: How do I add new puzzle types?**
A: Implement the `IInteractable` interface, create the puzzle logic, and ensure it works in both single-player and multiplayer.

**Q: Where is the documentation for specific systems?**
A: See the [API Reference](#api-reference) section and inline code comments.

### Modding Questions

**Q: Can I mod this game?**
A: Modding support is not officially implemented yet, but the project is open for review. Contact the owner regarding modding rights.

**Q: Can I use assets from this project in my own game?**
A: No, this is proprietary. See [License](#license) for details.

**Q: Can I create custom maps?**
A: Yes, you can create custom maps in the editor for personal use. Distribution requires permission.

---

## Version History

### v0.1.0 - Current Development Version (2026-01-01)

**Initial Release Features:**
- ✅ Dual timeline system (Past/Future)
- ✅ Core stealth mechanics with detection
- ✅ Multiple puzzle types (Keypad, Wire, Lever, Calendar)
- ✅ Gameplay Ability System integration
- ✅ Inventory system with 8 slots
- ✅ Multiplayer support (Steam + EOS)
- ✅ Guard and Civilian AI
- ✅ Security cameras and metal detectors
- ✅ Lockpicking and hacking abilities
- ✅ Procedural content generation
- ✅ Ghost character system

**Known Issues:**
- Save system is basic (checkpoint only)
- Some animation transitions need polish
- Performance optimization ongoing
- Tutorial system incomplete
- Character customization not yet implemented

**Technical Details:**
- Engine: Unreal Engine 5.7
- C++ Standard: C++17
- Network: Steam Sockets + EOS
- AI: State Tree system
- Abilities: Gameplay Ability System (GAS)

### Planned Features (Roadmap)

See [Roadmap](#roadmap) section for upcoming features.

---

## Roadmap

### Short-Term Goals (Next 3 Months)

**Gameplay:**
- [ ] Tutorial level with guided objectives
- [ ] Additional puzzle types (pressure plates, mirrors)
- [ ] More diverse guard behaviors (investigation, radio calls)
- [ ] Character customization (basic outfits)
- [ ] Achievement system

**Technical:**
- [ ] Enhanced save/load system (multiple saves)
- [ ] Performance profiling and optimization pass
- [ ] Network code optimization for >4 players
- [ ] Better error handling and recovery
- [ ] Automated testing framework

**Content:**
- [ ] 3 additional heist missions
- [ ] More item types and gadgets
- [ ] Additional soundtrack and SFX
- [ ] Localization (Spanish, French, German)

### Medium-Term Goals (3-6 Months)

**Gameplay:**
- [ ] Difficulty settings (Easy, Normal, Hard)
- [ ] Leaderboards and speedrun timers
- [ ] New abilities (distraction, disguise)
- [ ] NPC dialogue system
- [ ] Dynamic mission generation

**Technical:**
- [ ] Dedicated server support
- [ ] Cross-play between Steam and EOS
- [ ] Replay system
- [ ] Advanced AI behaviors (investigation, searching)
- [ ] Mod support framework

**Content:**
- [ ] 5 more missions with varied themes
- [ ] Customizable hideout/hub area
- [ ] Cinematics and story cutscenes
- [ ] More character customization options

### Long-Term Goals (6-12 Months)

**Gameplay:**
- [ ] PvP mode (Guards vs Thieves)
- [ ] Procedurally generated infinite missions
- [ ] Skill tree and progression system
- [ ] Seasonal events and challenges
- [ ] User-generated content support

**Technical:**
- [ ] Console ports (Xbox, PlayStation)
- [ ] VR support (experimental)
- [ ] Cloud save integration
- [ ] Advanced anti-cheat
- [ ] Comprehensive mod tools

**Content:**
- [ ] 10+ missions spanning different eras
- [ ] Full story campaign with voice acting
- [ ] Extensive character customization
- [ ] Multiple hideout locations
- [ ] Community workshop integration

**Community:**
- [ ] Official Discord server
- [ ] Regular devlogs and updates
- [ ] Beta testing program
- [ ] Content creator support
- [ ] Community tournaments

### Feature Requests

Have an idea for Echoes of Time? 
- [Submit a feature request](https://github.com/Traveler3114/Echoes-of-Time/issues/new?labels=enhancement)
- Vote on existing proposals
- Join discussions on GitHub Discussions

---

## Contributing

Contributions are welcome! To contribute:

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/my-feature`
3. **Commit your changes**: `git commit -am 'Add new feature'`
4. **Push to the branch**: `git push origin feature/my-feature`
5. **Create a Pull Request**

### Contribution Guidelines

#### Code Standards
- Follow Unreal Engine C++ coding standards
- Use `U` prefix for UObjects, `A` for Actors, `F` for structs, `E` for enums
- Write clear, descriptive commit messages
- Document complex logic with comments
- Ensure code compiles without warnings

#### Testing Requirements
- Test your changes in both single-player and multiplayer
- Verify no performance regressions
- Ensure all existing features still work
- Test on different hardware if possible

#### Pull Request Process
1. Update documentation if you change functionality
2. Add yourself to the Contributors list
3. Ensure CI/CD checks pass (when implemented)
4. Request review from maintainers
5. Address feedback promptly

#### What to Contribute

**We Need Help With:**
- 🐛 Bug fixes
- 📝 Documentation improvements
- 🎨 Art and animation assets
- 🔊 Sound effects and music
- 🗺️ New maps and levels
- 🧩 New puzzle types
- 🎯 Gameplay balancing
- 🌐 Localization (translations)
- ✅ Test coverage

**Please Don't:**
- Submit copyright-infringing content
- Make major architectural changes without discussion
- Add dependencies without justification
- Ignore coding standards

### Code of Conduct

Be respectful, inclusive, and professional in all interactions. Harassment, discrimination, and toxic behavior will not be tolerated.

---

## License

**This project is proprietary. All rights reserved.**

Copyright © 2026 [Repository Owner Name]

This software and its source code are provided for **review and educational purposes only**. You may not use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software without explicit written permission from the copyright holder.

For licensing inquiries, please contact the repository owner via GitHub issues or email.

### Third-Party Licenses

This project uses the following third-party assets and plugins:

**Plugins:**
- **Advanced Sessions Plugin** - MIT License
- **Advanced Steam Sessions** - MIT License
- **Gameplay Abilities Plugin** - Epic Games (UE EULA)

**Assets:**
- **Low Poly Sci-Fi Corridor** - [Marketplace License]
- **Polygon Heist Pack** - [Marketplace License]
- **Polygon Spy Pack** - [Marketplace License]

All third-party content is used in accordance with their respective licenses.

---

## Acknowledgments

### Special Thanks

- **Epic Games** - For Unreal Engine 5 and the Gameplay Ability System
- **Valve Corporation** - For Steam integration support
- **Contributors** - Everyone who has contributed to this project
- **Community** - Playtesters and feedback providers

### Technologies Used

- **Unreal Engine 5.7** - Game engine
- **Gameplay Ability System (GAS)** - Ability framework
- **State Tree** - AI behavior system
- **Enhanced Input System** - Input management
- **Steam Sockets** - Network transport layer
- **Epic Online Services (EOS)** - Cross-platform multiplayer
- **Visual Studio 2022** - Development environment
- **Git & GitHub** - Version control

### Inspiration

Echoes of Time draws inspiration from classic stealth games like:
- *Thief* series - For stealth mechanics
- *Dishonored* series - For level design and abilities
- *Payday* series - For co-op heist gameplay
- *Portal 2* - For puzzle design

### Asset Credits

**Art & Models:**
- Low Poly Sci-Fi Corridor by [Artist Name] on Unreal Marketplace
- Polygon Heist Pack by Synty Studios
- Polygon Spy Pack by Synty Studios

**Audio:**
- Sound effects from [Source]
- Music by [Composer/Source]

**Fonts & UI:**
- UI Icons from [Source]
- Fonts: [Font Names and Licenses]

*If you believe content has been used without proper attribution, please contact us immediately.*

---

## Contact

### Get in Touch

- 📧 **Email**: [Your Email] (for business/licensing inquiries)
- 💬 **Discord**: [Discord Server Link] *(Coming Soon)*
- 🐦 **Twitter**: [@YourTwitter] *(Optional)*
- 🎮 **Steam**: [Steam Page] *(When Available)*

### Repository

- 🏠 **Homepage**: [Echoes-of-Time](https://github.com/Traveler3114/Echoes-of-Time)
- 🐛 **Bug Reports**: [Report a bug](https://github.com/Traveler3114/Echoes-of-Time/issues/new?labels=bug)
- ✨ **Feature Requests**: [Request a feature](https://github.com/Traveler3114/Echoes-of-Time/issues/new?labels=enhancement)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/Traveler3114/Echoes-of-Time/discussions)
- 📖 **Wiki**: [Project Wiki](https://github.com/Traveler3114/Echoes-of-Time/wiki) *(Coming Soon)*

### Support the Project

If you enjoy Echoes of Time and want to support development:
- ⭐ Star the repository on GitHub
- 🐛 Report bugs and issues
- 💡 Suggest features and improvements
- 🔄 Share with friends and gaming communities
- 🤝 Contribute code, art, or documentation

---

<div align="center">

**🕰️ Echoes of Time 🕰️**

*Master time, master the heist.*

[![GitHub Stars](https://img.shields.io/github/stars/Traveler3114/Echoes-of-Time?style=social)](https://github.com/Traveler3114/Echoes-of-Time)
[![GitHub Forks](https://img.shields.io/github/forks/Traveler3114/Echoes-of-Time?style=social)](https://github.com/Traveler3114/Echoes-of-Time/fork)
[![GitHub Issues](https://img.shields.io/github/issues/Traveler3114/Echoes-of-Time)](https://github.com/Traveler3114/Echoes-of-Time/issues)

---

*Built with ❤️ using Unreal Engine 5.7*

Made with [Unreal Engine](https://www.unrealengine.com/) | Powered by [Gameplay Ability System](https://docs.unrealengine.com/5.7/gameplay-ability-system-for-unreal-engine/)

---

**[⬆ Back to Top](#echoes-of-time)**

</div>
