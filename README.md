# Echoes of Time

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/license-All%20Rights%20Reserved-red.svg)

**Echoes of Time** is a multiplayer stealth-action game built with Unreal Engine 5.7 that features time manipulation mechanics, puzzle-solving, and cooperative gameplay. Players navigate through different timelines (Past and Future) while engaging in stealth missions, hacking systems, and solving puzzles.

---

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Project Structure](#project-structure)
- [Key Systems](#key-systems)
- [Development Workflow](#development-workflow)
- [Gameplay Systems](#gameplay-systems)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Features

### Core Gameplay
- **Timeline Switching**: Seamlessly switch between Past and Future timelines
- **Multiplayer Support**: Cooperative gameplay with advanced session management
- **Stealth Mechanics**: Detection system, line-of-sight, and AI patrol behaviors
- **Puzzle Systems**: Hacking, lockpicking, and environmental puzzles
- **Mini-Games**: Firewall hacking and Neon Runner challenges

### Technical Features
- **Gameplay Ability System (GAS)**: Modular ability framework
- **Advanced Sessions**: Robust multiplayer networking with Steam integration
- **State Tree AI**: Advanced AI behaviors and decision-making
- **Component-Based Architecture**: Reusable systems for inventory, detection, and interaction
- **Drone Gameplay**: Controllable drone with unique mechanics
- **Online Subsystem**: Epic Games Services and Steam Sockets integration

---

## Prerequisites

### Required Software

#### 1. Unreal Engine 5.7
- Download from [Epic Games Launcher](https://www.unrealengine.com/download)
- Make sure to install Unreal Engine 5.7 specifically
- Include the following components during installation:
  - Core Engine Components
  - Starter Content
  - Templates and Feature Packs

#### 2. Visual Studio 2022 (Windows)
Download and install Visual Studio 2022 with the following workloads:
- **Game development with C++**
- **Desktop development with C++**
- **.NET desktop development**

Required Individual Components (as specified in `.vsconfig`):
- `Microsoft.VisualStudio.Component.VC.14.38.17.8.x86.x64` (MSVC v143)
- `Microsoft.VisualStudio.Component.VC.14.44.17.14.x86.x64` (MSVC v143)
- `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`
- `Microsoft.VisualStudio.Component.VC.Llvm.Clang`
- `Microsoft.VisualStudio.Component.Windows11SDK.22621`
- `Microsoft.Net.Component.4.6.2.TargetingPack`
- `Component.Unreal.Ide` (Unreal Engine IDE Integration)
- `Component.Unreal.Debugger` (Unreal Engine Debugger)

#### 3. Git
- Download from [git-scm.com](https://git-scm.com/)
- Required for cloning the repository and version control

### System Requirements

#### Minimum Requirements
- **OS**: Windows 10 64-bit (Version 20H2 or later)
- **Processor**: Quad-core Intel or AMD, 2.5 GHz or faster
- **Memory**: 16 GB RAM
- **Graphics**: DirectX 11 or DirectX 12 compatible graphics card
- **Storage**: 100 GB available space (for engine + project)

#### Recommended Requirements
- **OS**: Windows 11 64-bit
- **Processor**: 8-core Intel or AMD, 3.0 GHz or faster
- **Memory**: 32 GB RAM
- **Graphics**: NVIDIA RTX 2070 or AMD equivalent
- **Storage**: 200 GB SSD

---

## Installation

### Step 1: Clone the Repository

```bash
git clone https://github.com/Traveler3114/Echoes-of-Time.git
cd Echoes-of-Time
```

### Step 2: Install Plugins

The project includes third-party plugins in the `Plugins` directory:

- **AdvancedSessions**: Enhanced multiplayer session management
- **AdvancedSteamSessions**: Steam-specific session features

These plugins are already included in the repository and will be compiled with the project.

### Step 3: Generate Project Files

Right-click on `EchoesOfTime.uproject` and select:
- **Generate Visual Studio project files**

This will create the necessary `.sln` file and project structure.

> **Note**: If you don't see this option, make sure Unreal Engine 5.7 is properly installed and associated with `.uproject` files.

### Step 4: Open the Project

**Option A: Through Visual Studio**
1. Open `EchoesOfTime.sln` in Visual Studio 2022
2. Set the build configuration to `Development Editor`
3. Set the platform to `Win64`
4. Press `F5` or click **Debug → Start Debugging**

**Option B: Through Epic Games Launcher**
1. Open Epic Games Launcher
2. Go to Unreal Engine → Library
3. Click "Add" and browse to `EchoesOfTime.uproject`
4. Double-click the project to launch

---

## Building the Project

### Building from Visual Studio

#### Debug Builds
```
Configuration: DebugGame Editor
Platform: Win64
```
- Use for debugging with full symbols
- Slower performance but easier to debug

#### Development Builds (Recommended)
```
Configuration: Development Editor
Platform: Win64
```
- Best balance of performance and debugging capabilities
- Default configuration for development

#### Shipping Builds
```
Configuration: Shipping
Platform: Win64
```
- Optimized for final release
- No editor, maximum performance

### Build Steps

1. Open `EchoesOfTime.sln` in Visual Studio 2022
2. Select build configuration (e.g., `Development Editor`)
3. Select platform: `Win64`
4. Right-click on **EchoesOfTime** in Solution Explorer
5. Click **Build**

Build output will be located in:
- Editor builds: `Binaries/Win64/UnrealEditor-EchoesOfTime.dll`
- Game builds: `Binaries/Win64/EchoesOfTime.exe`

### Building from Command Line

```bash
# Navigate to engine directory
cd "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles"

# Build the project
Build.bat EchoesOfTime Win64 Development -project="PATH_TO_PROJECT\EchoesOfTime.uproject"
```

### Packaging the Game

1. Open the project in Unreal Editor
2. Go to **File → Package Project → Windows → Windows (64-bit)**
3. Select output directory
4. Wait for packaging to complete

---

## Project Structure

```
Echoes-of-Time/
├── Config/                      # Project configuration files
│   ├── DefaultEngine.ini        # Engine settings
│   ├── DefaultGame.ini          # Game-specific settings
│   ├── DefaultInput.ini         # Input bindings
│   ├── DefaultEditor.ini        # Editor preferences
│   └── DefaultGameplayTags.ini  # Gameplay tags definitions
│
├── Content/                     # Unreal Engine assets
│   ├── Animations/              # Animation assets
│   ├── Blueprints/              # Blueprint classes
│   ├── Characters/              # Character assets
│   ├── DataAssets/              # Data-driven configuration
│   ├── Input/                   # Input mapping contexts
│   ├── Maps/                    # Level files
│   ├── Materials/               # Material assets
│   ├── Meshes/                  # Static and skeletal meshes
│   ├── Niagara/                 # VFX systems
│   ├── Sounds/                  # Audio files
│   ├── Textures/                # Texture assets
│   ├── PolygonHeist/            # Asset pack content
│   └── PolygonSpy/              # Asset pack content
│
├── Plugins/                     # Third-party and custom plugins
│   ├── AdvancedSessions/        # Multiplayer session management
│   └── AdvancedSteamSessions/   # Steam integration
│
├── Source/                      # C++ source code
│   ├── EchoesOfTime/            # Main game module
│   │   ├── AbilitySystem/       # Gameplay Ability System
│   │   ├── ActorComponents/     # Reusable actor components
│   │   ├── Actors/              # Custom actor classes
│   │   ├── Characters/          # Character implementations
│   │   ├── Controllers/         # Player and AI controllers
│   │   ├── DataAssets/          # C++ data asset definitions
│   │   ├── GameModes/           # Game mode implementations
│   │   ├── GameStates/          # Game state management
│   │   ├── Interfaces/          # C++ interfaces
│   │   ├── Minigames/           # Mini-game systems
│   │   ├── Saving/              # Save/load system
│   │   └── Widgets/             # UI widget classes
│   ├── EchoesOfTime.Target.cs   # Build target (Game)
│   └── EchoesOfTimeEditor.Target.cs  # Build target (Editor)
│
├── docs/                        # Documentation
│   └── images/                  # Documentation images
│
├── .gitignore                   # Git ignore rules
├── .vsconfig                    # Visual Studio configuration
├── EchoesOfTime.uproject        # Unreal project file
└── README.md                    # This file
```

---

## Key Systems

### 1. Gameplay Ability System (GAS)

Located in `Source/EchoesOfTime/AbilitySystem/`

The project uses Unreal's Gameplay Ability System for character abilities:

**Key Components:**
- **DefaultAbilitySystemComponent**: Core ability system management
- **PlayerAttributeSet**: Character attributes (health, stamina, speed)
- **Abilities**: Hacking, lockpicking, time manipulation
- **EOTGameplayTags**: Gameplay tag definitions

**Example Abilities:**
- `DefaultGAHack`: Hacking gameplay ability
- `DefaultGALockPick`: Lockpicking gameplay ability
- `FutureGAPastEcho`: Timeline echo ability

### 2. Timeline System

The core mechanic of switching between Past and Future timelines:

**Key Files:**
- `TimelineEra.h`: Enum defining Past/Future states
- Timeline-specific logic in character and game state classes

**Usage:**
```cpp
ETimelineEra::Past    // Past timeline
ETimelineEra::Future  // Future timeline
```

### 3. Character System

Located in `Source/EchoesOfTime/Characters/`

**Character Types:**
- **DefaultCharacter**: Main player character with full abilities
- **DronePawn**: Controllable drone with unique mechanics
- **AICharacter**: Base class for AI-controlled characters
- **GuardCharacter**: Patrolling guard with detection
- **RobotGuardCharacter**: Future timeline robot guard
- **CivilianCharacter**: Non-hostile NPCs

### 4. Component-Based Systems

Located in `Source/EchoesOfTime/ActorComponents/`

Reusable components for various gameplay features:

- **InventoryComponent**: Item management
- **DetectionComponent**: AI vision and awareness
- **HackComponent**: Hackable object logic
- **ProximityHackComponent**: Proximity-based hacking
- **LockPickComponent**: Lockpicking system
- **SearchComponent**: Search behavior for AI

### 5. Multiplayer System

**Networking Features:**
- Advanced Sessions plugin for lobby management
- Steam Sockets for P2P networking
- Epic Online Services (EOS) integration
- Replicated game state and player state

**Key Classes:**
- `LobbyGameMode`: Lobby management
- `BaseGameState`: Replicated game state
- `DefaultPlayerState`: Player-specific replicated data

### 6. Interface System

Located in `Source/EchoesOfTime/Interfaces/`

C++ interfaces for polymorphic behavior:

- **IInteractable**: Objects players can interact with
- **IDetectable**: Objects that can be detected by AI
- **IRepairable**: Repairable objects (future timeline)
- **IKeycardUnlockable**: Keycard-locked objects
- **IPuzzleCompletionReceiver**: Puzzle completion callbacks

### 7. Mini-Game Systems

Located in `Source/EchoesOfTime/Minigames/`

- **FirewallMiniGame**: Hacking mini-game
- **NeonRunnerMiniGame**: Timing-based challenge

---

## Development Workflow

### Daily Development

1. **Pull Latest Changes**
   ```bash
   git pull origin main
   ```

2. **Open Project**
   - Launch through Visual Studio or Epic Games Launcher
   - Wait for shader compilation (first time only)

3. **Make Changes**
   - Edit C++ code in Visual Studio
   - Edit Blueprints/Assets in Unreal Editor

4. **Compile C++ Changes**
   - In Visual Studio: `Ctrl + Shift + B` (Build Solution)
   - In Unreal Editor: **Hot Reload** button (for small changes)

5. **Test Changes**
   - Use PIE (Play In Editor): `Alt + P`
   - Use Standalone: `Alt + S`

6. **Commit Changes**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin your-branch
   ```

### Creating New Features

#### Adding a New C++ Class

1. **In Unreal Editor:**
   - **Tools → New C++ Class**
   - Select parent class
   - Name your class
   - Click **Create Class**

2. **Visual Studio will reopen** with new files added
3. Implement your class
4. Compile the project
5. Refresh Unreal Editor to see changes

#### Adding New Gameplay Ability

1. Create new C++ class inheriting from `UGameplayAbility`
2. Implement `ActivateAbility()` and `EndAbility()`
3. Create Blueprint child class for designers
4. Add to `DefaultGASet` data asset

### Debugging

#### C++ Debugging

1. Set breakpoints in Visual Studio
2. Attach to process: **Debug → Attach to Process**
3. Select `UnrealEditor.exe`
4. Or press `F5` to start with debugger

#### Blueprint Debugging

1. Add breakpoints in Blueprint graphs
2. Enter PIE mode
3. Execution will pause at breakpoints

#### Useful Console Commands

```
// Display framerate
stat fps

// Show collision
show collision

// Network stats
stat net

// AI debugging
showdebug ai

// Gameplay debugger
showdebug abilitysystem
```

### Hot Reload

For quick iterations on C++ code:

1. Make small changes to `.cpp` files
2. Click **Live Coding** button in editor
3. Changes compile without restarting editor

> **Note**: Hot reload has limitations. Major changes require full restart.

---

## Gameplay Systems

### Input System

Uses Unreal's Enhanced Input System:

- **Input Mapping Contexts**: Defined in `Content/Input/`
- **Input Actions**: Movement, interaction, abilities
- **Data Assets**: `InputMappingSet` for configuration

### Inventory System

- Managed by `InventoryComponent`
- Supports multiple item slots
- Items can be equipped and dropped
- Replicated for multiplayer

### AI System

- Uses State Tree for decision-making
- Patrol behaviors with waypoints
- Detection system with line-of-sight
- Alert states: Idle, Investigating, Alert, Combat

### Save System

Located in `Source/EchoesOfTime/Saving/`

- Save game data persistence
- Checkpoint system
- Player progress tracking

---

## Troubleshooting

### Common Issues

#### "Missing Modules" Error

**Problem**: "The following modules are missing or built with a different engine version"

**Solution**:
1. Right-click `EchoesOfTime.uproject`
2. Select **Generate Visual Studio project files**
3. Open solution and rebuild

#### Shader Compilation Taking Forever

**Problem**: Initial shader compilation takes a long time

**Solution**:
- This is normal for first launch
- Use `DerivedDataCache` from another machine if available
- Consider upgrading to SSD storage

#### Plugin Not Found

**Problem**: Missing plugin errors

**Solution**:
1. Ensure plugins are in `Plugins/` directory
2. Check `.uproject` has correct plugin references
3. Rebuild project files

#### Out of Memory During Build

**Problem**: Compilation fails with out of memory error

**Solution**:
- Close unnecessary applications
- Use `/maxcpucount:1` in build command
- Increase system page file size

### Getting Help

- Check Unreal Engine documentation: [docs.unrealengine.com](https://docs.unrealengine.com)
- Unreal Engine Forums: [forums.unrealengine.com](https://forums.unrealengine.com)
- Discord community channels

---

## Contributing

### Code Style

Follow Epic's coding standards:
- Use `PascalCase` for classes: `AMyActor`
- Use `camelCase` for variables: `myVariable`
- Prefix classes with appropriate letter:
  - `A` for Actors: `ADefaultCharacter`
  - `U` for UObjects: `UInventoryComponent`
  - `F` for Structs: `FInventorySlot`
  - `E` for Enums: `ETimelineEra`
  - `I` for Interfaces: `IInteractable`

### Pull Request Process

1. Create a feature branch: `git checkout -b feature/my-feature`
2. Make changes and commit
3. Push to your fork
4. Create Pull Request with clear description
5. Wait for code review

### Testing Guidelines

- Test in single-player and multiplayer
- Verify both Past and Future timelines
- Check AI behavior changes
- Test with different player counts

---

## License

Copyright © 2025 Traveler3114. All Rights Reserved.

This project is proprietary software. Unauthorized copying, distribution, or modification is strictly prohibited.

---

## Credits

### Development Team
- **Project Lead**: Traveler3114

### Third-Party Assets & Plugins

- **Advanced Sessions Plugin**: Joshua Statzer
- **Unreal Engine 5.7**: Epic Games
- **Polygon Heist Pack**: Synty Studios
- **Polygon Spy Pack**: Synty Studios
- **Low Poly Sci-Fi Corridor**: [Asset Pack]

### Special Thanks

- Epic Games for Unreal Engine
- The Unreal Engine community

---

## Additional Resources

### Useful Links

- [Unreal Engine Documentation](https://docs.unrealengine.com/5.7/en-US/)
- [Gameplay Ability System Documentation](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Multiplayer Networking](https://docs.unrealengine.com/5.7/en-US/networking-and-multiplayer-in-unreal-engine/)
- [Enhanced Input System](https://docs.unrealengine.com/5.7/en-US/enhanced-input-in-unreal-engine/)

### Learning Resources

- [Unreal Engine C++ Tutorials](https://www.unrealengine.com/en-US/onlinelearning-courses)
- [Tom Looman's Tutorials](https://www.tomlooman.com/)
- [Unreal Slackers Discord](https://unrealslackers.org/)

---

**Last Updated**: January 2025  
**Version**: 1.0  
**Unreal Engine**: 5.7
