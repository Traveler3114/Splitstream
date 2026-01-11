# Echoes of Time

<div align="center">

**A Stealth-Action Game with Timeline Manipulation Mechanics**

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-blue.svg)](https://www.unrealengine.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

[Features](#features) • [Quick Start](#quick-start) • [Documentation](#documentation) • [Contributing](#contributing)

</div>

---

## Overview

**Echoes of Time** is an ambitious stealth-action game built with Unreal Engine 5.7 that features innovative timeline manipulation mechanics. Players navigate between Past and Future timelines, utilizing unique abilities and strategies to overcome challenges in a cyberpunk-inspired world.

### Key Concepts

- **Timeline Switching**: Seamlessly transition between Past and Future eras, each with distinct environments and challenges
- **Stealth Gameplay**: Use detection systems, cover mechanics, and tactical planning to avoid or neutralize threats
- **Ability System**: Leverage Unreal's Gameplay Ability System (GAS) for hacking, lockpicking, and special powers
- **Multiplayer Support**: Full networking implementation with lobby system and session management
- **AI Behavior**: Sophisticated AI using State Tree for guards, drones, civilians, and robots
- **Mini-Games**: Interactive hacking and puzzle challenges integrated into gameplay

---

## Features

### Core Gameplay

- 🕐 **Timeline Manipulation** - Switch between Past and Future to solve puzzles and access new areas
- 🎯 **Stealth Mechanics** - Detection systems, line of sight, noise propagation, and cover system
- 🔓 **Interactive Systems** - Hacking terminals, lockpicking doors, searching containers
- 👤 **Character Progression** - Ability unlocks and character customization
- 🎮 **Multiple Playable Maps** - Bank heists, corridor infiltration, and custom scenarios

### Technical Features

- 🎭 **Gameplay Ability System (GAS)** - Comprehensive ability framework with:
  - Hacking abilities
  - Lockpicking mechanics
  - Timeline echo abilities
  - Attribute-based character stats
  - Gameplay effects and tags
  
- 🤖 **Advanced AI** - State Tree-based behaviors for:
  - Guards with patrol and chase routines
  - Drones with surveillance patterns
  - Civilians with alarm systems
  - Robots with repair behaviors
  
- 🌐 **Networking** - Full multiplayer implementation:
  - Session creation and joining
  - Player state replication
  - Lobby system
  - Steam/EOS integration ready
  
- 🎨 **Visual Features**:
  - Niagara VFX system
  - Dynamic lighting with GPU Lightmass
  - Polygon art style (PolygonHeist/PolygonSpy assets)
  - Custom materials and textures

### Game Modes

- **Single Player** - Solo stealth missions
- **Multiplayer Co-op** - Team-based heists
- **Lobby System** - Pre-game character selection and loadout

---

## Quick Start

Get up and running in 15 minutes! For detailed setup instructions, see [QUICK_START.md](docs/QUICK_START.md).

### Prerequisites

| Requirement | Minimum | Recommended |
|------------|---------|-------------|
| **OS** | Windows 10 64-bit | Windows 11 64-bit |
| **RAM** | 16GB | 32GB |
| **Storage** | 100GB free | 150GB free (SSD) |
| **GPU** | NVIDIA GTX 1060 / AMD RX 580 | NVIDIA RTX 2070 / AMD RX 5700 XT |
| **CPU** | Intel i5-8400 / AMD Ryzen 5 2600 | Intel i7-9700K / AMD Ryzen 7 3700X |

### Software Requirements

- [Unreal Engine 5.7](https://www.unrealengine.com/download) (via Epic Games Launcher)
- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with:
  - Game development with C++
  - .NET desktop development
- [Git](https://git-scm.com/download/win) with Git LFS

### Installation

```bash
# 1. Clone the repository
git clone https://github.com/Traveler3114/Echoes-of-Time.git
cd Echoes-of-Time

# 2. Pull large files
git lfs pull

# 3. Generate Visual Studio project files
# Right-click EchoesOfTime.uproject → "Generate Visual Studio project files"

# 4. Open solution and build
# Open EchoesOfTime.sln in Visual Studio
# Build → Build Solution (Ctrl+Shift+B)
# Configuration: Development Editor | Win64

# 5. Launch the editor
# Press F5 in Visual Studio or double-click EchoesOfTime.uproject
```

**First Launch Note**: Shader compilation will take 20-60 minutes on first run. This is normal!

---

## Documentation

Comprehensive documentation is available in the [docs/](docs/) directory:

| Document | Description | Size |
|----------|-------------|------|
| **[INDEX.md](docs/INDEX.md)** | Complete documentation index and navigation guide | Essential |
| **[QUICK_START.md](docs/QUICK_START.md)** | 15-minute setup guide | 10 KB |
| **[BUILD_GUIDE.md](docs/BUILD_GUIDE.md)** | Detailed build instructions and troubleshooting | 18 KB |
| **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** | Technical architecture overview | 19 KB |
| **[SYSTEMS.md](docs/SYSTEMS.md)** | Detailed systems documentation | 18 KB |
| **[CODE_REFERENCE.md](docs/CODE_REFERENCE.md)** | Complete C++ API reference (274 files) | 94 KB |
| **[CONTRIBUTING.md](docs/CONTRIBUTING.md)** | Contribution guidelines and standards | 14 KB |

**Total Documentation**: ~191 KB across 7,446 lines

### Quick Links by Topic

- **Setup & Installation**: [QUICK_START.md](docs/QUICK_START.md) → [BUILD_GUIDE.md](docs/BUILD_GUIDE.md)
- **Understanding Architecture**: [ARCHITECTURE.md](docs/ARCHITECTURE.md) → [SYSTEMS.md](docs/SYSTEMS.md)
- **Code Reference**: [CODE_REFERENCE.md](docs/CODE_REFERENCE.md)
- **Contributing**: [CONTRIBUTING.md](docs/CONTRIBUTING.md)

---

## Project Structure

```
EchoesOfTime/
├── Source/EchoesOfTime/              # C++ source code (274 files)
│   ├── AbilitySystem/                # Gameplay Ability System implementation
│   │   ├── Abilities/                # All gameplay abilities
│   │   ├── AbilityTasks/             # Custom ability tasks
│   │   ├── AttributeSets/            # Character attributes
│   │   ├── GameplayCues/             # Visual/audio effects
│   │   └── GameplayEffects/          # Stat modifications
│   │
│   ├── ActorComponents/              # Reusable components
│   │   ├── DetectionComponent        # AI detection and alerts
│   │   ├── HackComponent             # Hacking interactions
│   │   ├── InventoryComponent        # Item management
│   │   ├── LockPickComponent         # Lockpicking system
│   │   ├── ProximityHackComponent    # Proximity-based hacking
│   │   └── SearchComponent           # Container searching
│   │
│   ├── Actors/                       # World actors (50+ types)
│   │   ├── Cameras, Doors, Elevators, Guards, NPCs
│   │   ├── Hackable objects, Lockers, Safes, Terminals
│   │   └── Timeline-specific actors (Past/Future variants)
│   │
│   ├── Characters/                   # Playable and AI characters
│   │   ├── DefaultCharacter          # Base player character
│   │   ├── FutureGuardCharacter      # Future timeline guard
│   │   ├── PastGuardCharacter        # Past timeline guard
│   │   └── Other AI character types
│   │
│   ├── Controllers/                  # Player controllers
│   │   └── DefaultPlayerController   # Enhanced input handling
│   │
│   ├── DataAssets/                   # Data-driven configuration
│   │   ├── Input action definitions
│   │   ├── Gameplay ability sets
│   │   └── Character attribute defaults
│   │
│   ├── GameModes/                    # Game mode logic
│   │   ├── BaseGameMode              # Core game mode
│   │   ├── DefaultGameMode           # Standard gameplay
│   │   └── LobbyGameMode             # Multiplayer lobby
│   │
│   ├── GameStates/                   # Game state management
│   │   ├── BaseGameState             # State replication
│   │   ├── DefaultGameState          # Gameplay state
│   │   └── LobbyGameState            # Lobby state
│   │
│   ├── Interfaces/                   # C++ interfaces (8 types)
│   │   ├── Hackable, Lockpickable, Searchable
│   │   └── Other gameplay interfaces
│   │
│   ├── Minigames/                    # Mini-game implementations
│   │   ├── FirewallMiniGame          # Hacking challenge
│   │   └── NeonRunnerMiniGame        # Lockpicking challenge
│   │
│   ├── Saving/                       # Save system
│   │   └── SaveDataStructs           # Save game data
│   │
│   ├── Widgets/                      # UI components
│   │   └── Various UI widgets        # HUD, menus, displays
│   │
│   ├── DefaultGameInstance.cpp/.h    # Session management
│   ├── DefaultPlayerState.cpp/.h     # Player state replication
│   ├── NavUtilityLibrary.cpp/.h      # Navigation utilities
│   └── TimelineEra.cpp/.h            # Timeline enum definition
│
├── Content/                          # Unreal assets
│   ├── Blueprints/                   # Blueprint classes
│   │   ├── AbilitySystem/            # BP ability implementations
│   │   ├── GameStates/               # BP game state classes
│   │   ├── Minigames/                # BP mini-games
│   │   └── StateTrees/               # AI behavior trees
│   │
│   ├── Maps/                         # Level files
│   │   ├── Bank/                     # Bank heist map
│   │   ├── Gym                       # Testing arena
│   │   ├── LobbyMap                  # Multiplayer lobby
│   │   ├── MainMenuMap               # Main menu
│   │   └── Various test maps
│   │
│   ├── Characters/                   # Character assets
│   ├── Materials/                    # Material definitions
│   ├── Meshes/                       # 3D models
│   ├── Animations/                   # Animation sequences
│   ├── Sounds/                       # Audio files
│   ├── Textures/                     # Texture assets
│   ├── Niagara/                      # VFX systems
│   ├── Input/                        # Enhanced Input assets
│   ├── DataAssets/                   # Data asset instances
│   └── Third-party asset packs
│
├── Plugins/                          # Engine and third-party plugins
│   ├── AdvancedSessions/             # Multiplayer sessions
│   └── AdvancedSteamSessions/        # Steam integration
│
├── Config/                           # Project configuration
│   ├── DefaultEngine.ini             # Engine settings
│   ├── DefaultGame.ini               # Game settings
│   ├── DefaultInput.ini              # Input configuration
│   └── Other config files
│
├── docs/                             # Documentation (191 KB)
│   ├── INDEX.md                      # Documentation index
│   ├── QUICK_START.md                # Quick setup guide
│   ├── BUILD_GUIDE.md                # Build instructions
│   ├── ARCHITECTURE.md               # Architecture docs
│   ├── SYSTEMS.md                    # Systems documentation
│   ├── CODE_REFERENCE.md             # Complete API reference
│   ├── CONTRIBUTING.md               # Contribution guide
│   └── images/                       # Documentation images
│
├── EchoesOfTime.uproject             # Project file (UE 5.7)
├── EchoesOfTime.sln                  # Visual Studio solution
└── README.md                         # This file
```

---

## Key Systems

### Gameplay Ability System (GAS)

The project uses Unreal's Gameplay Ability System for all character abilities:

**Abilities**:
- `DefaultGAHack` - Terminal and device hacking
- `DefaultGALockPick` - Door and container lockpicking
- `FutureGAPastEcho` - Timeline switching ability
- Custom abilities for special powers

**Attributes**:
- Health / MaxHealth
- Stamina / MaxStamina
- WalkSpeed, RunSpeed, CrouchSpeed
- Custom stat attributes

**Implementation**: See [SYSTEMS.md - Gameplay Ability System](docs/SYSTEMS.md#gameplay-ability-system)

### AI and Detection System

**AI Types**:
- **Guards** - Patrol, chase, and combat behaviors (State Tree)
- **Drones** - Surveillance and scanning patterns
- **Civilians** - Ambient behavior and alarm triggering
- **Robots** - Patrol and repair routines

**Detection Features**:
- Line of sight detection
- Noise-based alerts
- Suspicion levels
- Team communication
- Alert states (Normal, Suspicious, Alerted)

**Implementation**: See [SYSTEMS.md - AI and Detection System](docs/SYSTEMS.md#ai-and-detection-system)

### Timeline System

The core mechanic of switching between Past and Future timelines:

- **Timeline Enum**: `ETimelineEra::Past` and `ETimelineEra::Future`
- **Actor Variants**: Separate Past/Future versions of actors
- **Timeline Abilities**: `FutureGAPastEcho` for switching
- **Visual Feedback**: Ghost characters showing other timeline

**Implementation**: See [SYSTEMS.md - Timeline System](docs/SYSTEMS.md#timeline-system)

### Networking

Full multiplayer support with session management:

- **Session Management**: Create, join, and leave sessions
- **Lobby System**: Pre-game character selection
- **Replication**: All gameplay elements network-ready
- **Online Subsystems**: Steam Sockets and EOS ready

**Implementation**: See [SYSTEMS.md - Networking System](docs/SYSTEMS.md#networking-system)

### Input System

Enhanced Input System implementation:

- **Input Actions**: Defined in `Content/Input/`
- **Mapping Contexts**: Contextual input handling
- **Rebindable Keys**: Runtime key rebinding support
- **Controller Support**: Gamepad and keyboard/mouse

**Implementation**: See [SYSTEMS.md - Input System](docs/SYSTEMS.md#input-system)

---

## Development Workflow

### Daily Development

```bash
# 1. Pull latest changes
git pull origin main

# 2. Open Visual Studio and ensure latest build
# Build → Build Solution

# 3. Launch editor
# Press F5 or double-click .uproject file

# 4. Make changes (C++ or Blueprints)

# 5. Test in editor
# Alt + P (Play in Editor)

# 6. Commit changes
git add .
git commit -m "Description of changes"
git push origin your-branch
```

### Adding New Features

1. **Create Feature Branch**: `git checkout -b feature/my-feature`
2. **Implement in C++**: Add classes in appropriate `Source/` subdirectory
3. **Create Blueprints**: Extend C++ classes in `Content/Blueprints/`
4. **Test Thoroughly**: PIE, standalone, and multiplayer tests
5. **Document**: Update relevant docs if needed
6. **Submit PR**: Create pull request with description

### Testing

**In-Editor Testing**:
- `Alt + P` - Play in Editor (PIE)
- `Alt + S` - Play Standalone
- `Alt + Shift + P` - Simulate

**Multiplayer Testing**:
- Editor → Play dropdown → Number of Players: 2+
- Net Mode: Play As Listen Server

**Performance Testing**:
```
# Console commands (~)
stat fps              # Show framerate
stat unit             # Frame timing
stat game             # Game thread
showdebug ai          # AI debugging
showdebug abilitysystem  # GAS debugging
```

### Building

**Development Editor** (daily development):
```bash
Configuration: Development Editor | Win64
Build time: ~5-15 minutes (incremental)
```

**Shipping** (release builds):
```bash
Configuration: Shipping | Win64
Build time: ~30-60 minutes (full optimization)
```

See [BUILD_GUIDE.md](docs/BUILD_GUIDE.md) for detailed build instructions.

---

## Multiplayer Setup

### Hosting a Game

**In Editor**:
1. Open `LobbyMap`
2. PIE with Number of Players: 2+
3. Host creates session
4. Clients join automatically in PIE

**Standalone**:
1. Package the game
2. Host: Run executable → Create Session
3. Client: Run executable → Join Session via lobby

### Session Configuration

Sessions managed by `UDefaultGameInstance`:

```cpp
// Create session
CreateSession(LevelName, LevelSoftReference);

// Leave session
HostLeaveToMainMenu();
```

**Network Settings**: `Config/DefaultEngine.ini`

See [SYSTEMS.md - Networking System](docs/SYSTEMS.md#networking-system) for details.

---

## Troubleshooting

### Common Issues

#### Build Fails

```bash
# Solution: Clean rebuild
1. Close Visual Studio
2. Delete: Intermediate/, Binaries/, .vs/, Saved/
3. Right-click .uproject → Generate Visual Studio project files
4. Open .sln → Build → Rebuild Solution
```

#### Editor Crashes on Startup

```bash
# Solution: Clear cached files
1. Delete Intermediate/ and Saved/ folders
2. Keep Binaries/ if build succeeded
3. Regenerate project files
4. Launch editor
```

#### Shader Compilation Stuck

```bash
# Solution: Reset shader cache
1. Close editor
2. Delete Saved/ShaderDebugInfo/
3. Delete DerivedDataCache/ (if exists)
4. Reopen editor (will recompile all shaders)
```

#### Git LFS Files Missing

```bash
# Solution: Pull LFS files
git lfs install
git lfs pull
```

#### Missing Plugins

```bash
# Ensure plugins are enabled in EchoesOfTime.uproject:
- GameplayAbilities
- StateTree
- GameplayStateTree
- OnlineSubsystemEOS
- SteamSockets
```

#### Low Performance

```
# In-Editor Performance:
1. Editor Preferences → Performance
2. "Use Less CPU when in Background"
3. Lower viewport scalability: Scalability → Low
4. Disable real-time rendering in viewport when not needed
```

### Getting Help

1. **Check Logs**: `Saved/Logs/EchoesOfTime.log`
2. **Documentation**: See [BUILD_GUIDE.md - Troubleshooting](docs/BUILD_GUIDE.md#troubleshooting)
3. **Search Issues**: Check [GitHub Issues](https://github.com/Traveler3114/Echoes-of-Time/issues)
4. **Ask for Help**: Create new issue with error details

---

## Contributing

We welcome contributions! Please read our [Contributing Guide](docs/CONTRIBUTING.md) before submitting PRs.

### Quick Contribution Checklist

- [ ] Read [CONTRIBUTING.md](docs/CONTRIBUTING.md)
- [ ] Fork the repository
- [ ] Create feature branch (`git checkout -b feature/amazing-feature`)
- [ ] Follow coding standards (see CONTRIBUTING.md)
- [ ] Test your changes thoroughly
- [ ] Commit with clear messages
- [ ] Update documentation if needed
- [ ] Submit Pull Request

### Coding Standards

- **C++ Style**: Follow [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.7/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- **Naming Conventions**:
  - Classes: `PascalCase` with prefix (`AMyActor`, `UMyComponent`)
  - Variables: `PascalCase` or `camelCase`
  - Functions: `PascalCase`
  - Constants: `UPPER_SNAKE_CASE`
- **Comments**: Use clear, descriptive comments for complex logic
- **Blueprints**: Organized node graphs with comment blocks

See [CONTRIBUTING.md - Coding Standards](docs/CONTRIBUTING.md#coding-standards) for details.

---

## Technologies Used

### Core Engine

- **Unreal Engine 5.7** - Latest engine features
- **C++17** - Modern C++ standard
- **Blueprints** - Visual scripting

### Unreal Systems

- **Gameplay Ability System (GAS)** - Abilities and attributes
- **Enhanced Input System** - Modern input handling
- **State Tree** - AI behavior system
- **Navigation System** - Pathfinding and navigation mesh
- **Replication Graph** - Multiplayer networking
- **Niagara** - VFX system
- **UMG** - UI framework

### Plugins

- **Advanced Sessions** - Session management
- **Advanced Steam Sessions** - Steam integration
- **Online Subsystem EOS** - Epic Online Services
- **Gameplay Abilities** - GAS plugin
- **GPU Lightmass** - Advanced lighting

### Third-Party Assets

- **PolygonHeist** - Heist-themed assets
- **PolygonSpy** - Spy-themed assets
- **Low Poly Sci-Fi Corridor** - Environment assets

---

## Roadmap

### Current Version Features

- ✅ Core stealth gameplay
- ✅ Timeline switching mechanics
- ✅ Multiplayer support
- ✅ AI behavior systems
- ✅ Hacking and lockpicking
- ✅ Multiple playable maps

### Planned Features

- 🔄 Story mode and campaign
- 🔄 Additional abilities and upgrades
- 🔄 More AI enemy types
- 🔄 Expanded mini-games
- 🔄 Character customization
- 🔄 Achievement system
- 🔄 Steam Workshop support

### Future Considerations

- 💭 Procedural level generation
- 💭 Modding support
- 💭 Dedicated server support
- 💭 Cross-platform play

---

## Performance Optimization

### Recommended Settings

**Development**:
- Configuration: Development Editor
- Scalability: Medium to High
- Resolution: Native

**Testing**:
- Configuration: Development
- Test standalone builds frequently
- Profile with `stat` commands

**Shipping**:
- Configuration: Shipping
- Full optimization enabled
- Package size: ~5-10 GB (estimated)

### Profiling

```cpp
// Console commands for profiling
stat fps              // Framerate
stat unit             // Frame time breakdown
stat game             // Game thread timing
stat gpu              // GPU timing
stat memory           // Memory usage
stat streaming        // Asset streaming
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Third-Party Assets

This project uses the following third-party assets:

- **PolygonHeist** - Licensed from Unreal Marketplace
- **PolygonSpy** - Licensed from Unreal Marketplace
- **Low Poly Sci-Fi Corridor** - Licensed from Unreal Marketplace

Please ensure you have proper licenses for all assets if you plan to distribute this project.

---

## Credits

### Development Team

- **Lead Developer**: Traveler3114
- **Project**: Echoes of Time

### Special Thanks

- Epic Games for Unreal Engine 5.7
- Advanced Sessions plugin developers
- Unreal Engine community

### Asset Credits

- PolygonHeist asset pack
- PolygonSpy asset pack
- Low Poly Sci-Fi Corridor pack

---

## Community and Support

### Resources

- **Documentation**: [docs/](docs/) folder
- **API Reference**: [CODE_REFERENCE.md](docs/CODE_REFERENCE.md)
- **GitHub Issues**: [Issue Tracker](https://github.com/Traveler3114/Echoes-of-Time/issues)
- **Pull Requests**: [PR List](https://github.com/Traveler3114/Echoes-of-Time/pulls)

### Learning Resources

- [Unreal Engine Documentation](https://docs.unrealengine.com/5.7/)
- [Gameplay Ability System](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Enhanced Input](https://docs.unrealengine.com/5.7/en-US/enhanced-input-in-unreal-engine/)
- [State Tree](https://docs.unrealengine.com/5.7/en-US/state-tree-in-unreal-engine/)
- [Multiplayer Networking](https://docs.unrealengine.com/5.7/en-US/networking-and-multiplayer-in-unreal-engine/)

### Community

- [Unreal Slackers Discord](https://unrealslackers.org/)
- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [r/unrealengine](https://reddit.com/r/unrealengine)
- [r/gamedev](https://reddit.com/r/gamedev)

---

## FAQ

### Q: Can I use this project commercially?

A: Check the LICENSE file. You'll also need proper licenses for all third-party assets used.

### Q: What platforms are supported?

A: Currently Windows 10/11 64-bit. Other platforms may be added in the future.

### Q: How long does the first build take?

A: Initial compilation: 10-30 minutes. First editor launch with shader compilation: 20-60 minutes.

### Q: Can I contribute without C++ experience?

A: Yes! You can contribute Blueprints, levels, documentation, bug reports, and testing.

### Q: Is multiplayer fully functional?

A: Yes, the networking system is implemented with lobby support and session management.

### Q: How do I report bugs?

A: Create a GitHub Issue with:
- Detailed description
- Steps to reproduce
- Expected vs actual behavior
- Logs from `Saved/Logs/`
- System specifications

---

## Contact

- **GitHub**: [@Traveler3114](https://github.com/Traveler3114)
- **Project Repository**: [Echoes-of-Time](https://github.com/Traveler3114/Echoes-of-Time)

---

## Acknowledgments

This project would not be possible without:

- 🙏 **Unreal Engine** by Epic Games
- 🙏 **Advanced Sessions Plugin** community
- 🙏 **Gameplay Ability System** documentation and examples
- 🙏 All open-source contributors and community members

---

<div align="center">

**Made with ❤️ using Unreal Engine 5.7**

[⬆ Back to Top](#echoes-of-time)

</div>
