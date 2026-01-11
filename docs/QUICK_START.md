# Quick Start Guide

Get up and running with Splitstream development in 15 minutes.

---

## Prerequisites

✅ **Windows 10/11 64-bit**  
✅ **16GB RAM minimum** (32GB recommended)  
✅ **100GB free disk space** (SSD recommended)  
✅ **NVIDIA GTX 1060 or equivalent**

---

## Installation (5 Steps)

### 1. Install Unreal Engine 5.7

1. Download [Epic Games Launcher](https://www.epicgames.com/store/download)
2. Sign in → **Unreal Engine** tab → **Install Engine**
3. Select version **5.7** → Install (~60GB)

### 2. Install Visual Studio 2022

1. Download [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/)
2. Install with these workloads:
   - ✅ **Game development with C++**
   - ✅ **Desktop development with C++**
   - ✅ **.NET desktop development**

### 3. Install Git

Download and install [Git for Windows](https://git-scm.com/download/win)

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

### 4. Clone Repository

```bash
git clone https://github.com/Traveler3114/Splitstream.git
cd Splitstream
git lfs pull
```

### 5. Generate and Build

1. **Right-click** `Splitstream.uproject` → **Generate Visual Studio project files**
2. **Double-click** `Splitstream.sln` to open Visual Studio
3. Set configuration to **Development Editor** | **Win64**
4. Press **Ctrl + Shift + B** to build (10-30 minutes first time)
5. Press **F5** to launch editor (shader compilation: 20-60 minutes first time)

---

## First Run Checklist

After opening the editor:

- [ ] **Wait for shader compilation** to complete (be patient!)
- [ ] **Verify Content Browser** shows all assets
- [ ] **Test Play (Alt+P)** in any map
- [ ] **Check Output Log** for errors

---

## Quick Commands

### Building
```bash
# Build from command line
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
    Splitstream Win64 Development ^
    -project="PATH\TO\Splitstream.uproject"

# Clean build
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Clean.bat" ^
    Splitstream Win64 Development ^
    -project="PATH\TO\Splitstream.uproject"
```

### Playing
```bash
# Editor
- Alt + P: Play in Editor (PIE)
- Alt + S: Play Standalone
- Alt + Shift + P: Simulate

# Multiplayer (2 players)
Editor → Play Dropdown → Number of Players: 2
```

### Debugging
```bash
# Visual Studio
F5: Start with Debugger
F9: Toggle Breakpoint
F10: Step Over
F11: Step Into

# Console Commands (in-game)
stat fps            # Show FPS
stat unit           # Frame timing
showdebug ai        # AI debugging
showdebug abilitysystem  # GAS debugging
```

---

## Project Structure (Key Directories)

```
Splitstream/
├── Source/Splitstream/     # C++ source code
│   ├── Characters/          # Player & AI characters
│   ├── AbilitySystem/       # Gameplay abilities
│   ├── ActorComponents/     # Reusable components
│   ├── GameModes/           # Game mode logic
│   └── Interfaces/          # C++ interfaces
│
├── Content/                 # Unreal assets (Blueprints, Materials, etc.)
│   ├── Blueprints/
│   ├── Maps/                # Level files
│   ├── Characters/
│   └── DataAssets/
│
├── Plugins/                 # Third-party plugins
│   ├── AdvancedSessions/    # Multiplayer
│   └── AdvancedSteamSessions/
│
└── Config/                  # Project configuration
    ├── DefaultEngine.ini
    ├── DefaultGame.ini
    └── DefaultInput.ini
```

---

## Development Workflow

### Making Changes

**1. C++ Changes:**
```bash
1. Edit code in Visual Studio
2. Build: Ctrl + Shift + B
3. Restart editor (or use Hot Reload for small changes)
4. Test: Alt + P
```

**2. Blueprint Changes:**
```bash
1. Edit Blueprints in Unreal Editor
2. Compile: Click "Compile" button
3. Save: Ctrl + S
4. Test: Alt + P
```

### Git Workflow

```bash
# Daily routine
git pull origin main
# Make changes
git add .
git commit -m "Description of changes"
git push origin your-branch

# Create feature branch
git checkout -b feature/my-feature
# Make changes and commit
git push origin feature/my-feature
# Create Pull Request on GitHub
```

---

## Common Tasks

### Adding a New C++ Class

1. **Editor** → **Tools** → **New C++ Class**
2. Choose parent class (e.g., `Actor`, `Character`, `ActorComponent`)
3. Name your class (e.g., `MyNewActor`)
4. Click **Create Class**
5. Visual Studio reopens with new files
6. Implement your class
7. Build and restart editor

### Adding a New Gameplay Ability

1. Create C++ class inheriting from `UGameplayAbility`:
   ```cpp
   UCLASS()
   class UMyAbility : public UGameplayAbility
   {
       GENERATED_BODY()
   
   public:
       virtual void ActivateAbility(...) override;
       virtual void EndAbility(...) override;
   };
   ```

2. Create Blueprint child in editor
3. Add to character's `DefaultGASet` DataAsset

### Creating a New Level

1. **File** → **New Level**
2. Choose template (Default, Empty, etc.)
3. Build level with BSP or meshes
4. Add lighting and post-process
5. **File** → **Save Current Level As...**
6. Save to `Content/Maps/`

---

## Testing

### Single Player Test
```
1. Open any map in editor
2. Press Alt + P (Play in Editor)
3. Test gameplay
4. Press Esc to stop
```

### Multiplayer Test
```
1. Editor → Play dropdown (next to Play button)
2. Number of Players: 2
3. Net Mode: Play As Listen Server
4. Click Play
5. Test with 2 instances
```

### Performance Check
```
# In-game console (~)
stat fps       # Show framerate
stat unit      # Frame time
stat game      # Game thread timing
```

---

## Troubleshooting

### "Missing Modules" Error
```
Solution:
1. Right-click Splitstream.uproject
2. Generate Visual Studio project files
3. Rebuild in Visual Studio
```

### Editor Crashes on Start
```
Solution:
1. Delete Intermediate/ and Saved/ folders
2. Delete Binaries/ folder
3. Regenerate project files
4. Rebuild
```

### Shader Compilation Stuck
```
Solution:
1. Close editor
2. Delete Saved/ShaderDebugInfo/
3. Reopen editor
```

### Build Fails
```
Solution:
1. Clean solution (Visual Studio → Build → Clean Solution)
2. Close Visual Studio
3. Delete Intermediate/, Binaries/, .vs/ folders
4. Regenerate project files
5. Rebuild
```

### Low FPS in Editor
```
Solutions:
1. Editor Preferences → Engine → Use Less CPU when in Background
2. Lower preview quality: Scalability → Low
3. Disable real-time rendering: Viewport → Realtime off
```

---

## Next Steps

Once you're set up:

1. **Read the Full Documentation:**
   - [README.md](../README.md) - Complete overview
   - [ARCHITECTURE.md](ARCHITECTURE.md) - Code architecture
   - [BUILD_GUIDE.md](BUILD_GUIDE.md) - Detailed build instructions

2. **Explore the Codebase:**
   - Start with `Source/Splitstream/Characters/DefaultCharacter.cpp`
   - Check out `Source/Splitstream/AbilitySystem/`
   - Look at interfaces in `Source/Splitstream/Interfaces/`

3. **Learn Key Systems:**
   - Gameplay Ability System (GAS)
   - Enhanced Input System
   - State Tree AI
   - Multiplayer Networking

4. **Join the Community:**
   - Review existing Pull Requests
   - Check Issues for bugs/features
   - Ask questions in discussions

---

## Essential Resources

### Documentation
- [UE5 C++ API Reference](https://docs.unrealengine.com/5.7/en-US/API/)
- [Gameplay Ability System](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Enhanced Input](https://docs.unrealengine.com/5.7/en-US/enhanced-input-in-unreal-engine/)

### Learning
- [Unreal Engine Learning](https://www.unrealengine.com/en-US/onlinelearning-courses)
- [Tom Looman's UE Tutorials](https://www.tomlooman.com/)
- [CodeLikeMe YouTube](https://www.youtube.com/@CodeLikeMe)

### Community
- [Unreal Slackers Discord](https://unrealslackers.org/)
- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [r/unrealengine](https://reddit.com/r/unrealengine)

---

## Quick Reference Card

### Hotkeys
| Action | Hotkey |
|--------|--------|
| Play in Editor | `Alt + P` |
| Play Standalone | `Alt + S` |
| Simulate | `Alt + Shift + P` |
| Compile (Blueprint) | `Ctrl + K` |
| Save | `Ctrl + S` |
| Build (VS) | `Ctrl + Shift + B` |
| Start Debugging (VS) | `F5` |
| Content Browser | `Ctrl + Space` |
| Console | `~` (tilde) |

### Console Commands
```bash
exit                 # Close game/editor
quit                 # Close game/editor
stat fps             # Show FPS
stat unit            # Frame timing
stat game            # Game thread
stat memory          # Memory usage
r.SetRes 1920x1080f  # Set resolution (fullscreen)
r.SetRes 1920x1080w  # Set resolution (windowed)
showdebug            # Debug HUD
```

### File Paths
```
Engine: C:\Program Files\Epic Games\UE_5.7\
Project: [YourPath]\Splitstream\
Binaries: [ProjectPath]\Binaries\Win64\
Content: [ProjectPath]\Content\
Source: [ProjectPath]\Source\Splitstream\
Logs: [ProjectPath]\Saved\Logs\
```

---

## Support

Having issues? Here's how to get help:

1. **Check Documentation:**
   - Read [BUILD_GUIDE.md](BUILD_GUIDE.md)
   - Search [Troubleshooting](#troubleshooting) section

2. **Check Logs:**
   - `Saved/Logs/` directory
   - Output Log in editor

3. **Search Existing Issues:**
   - Check GitHub Issues tab
   - Search for similar problems

4. **Ask for Help:**
   - Create GitHub Issue with details
   - Include error messages and logs
   - Describe steps to reproduce

---

**Welcome to Splitstream development!** 🎮

Start with a simple task, explore the codebase, and don't hesitate to ask questions. Happy coding!

---

**Last Updated**: January 2025
