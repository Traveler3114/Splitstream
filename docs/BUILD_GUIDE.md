# Build and Development Guide

This guide provides detailed instructions for building, testing, and developing Splitstream.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [First-Time Setup](#first-time-setup)
- [Building the Project](#building-the-project)
- [Running the Game](#running-the-game)
- [Development Workflows](#development-workflows)
- [Testing](#testing)
- [Debugging](#debugging)
- [Packaging](#packaging)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Software Requirements

Before you begin, ensure you have the following installed:

#### 1. Unreal Engine 5.7

**Installation Steps:**
1. Download [Epic Games Launcher](https://www.epicgames.com/store/download)
2. Sign in with your Epic Games account
3. Navigate to **Unreal Engine** tab
4. Click **Install Engine**
5. Select version **5.7** from the dropdown
6. Choose installation location (requires ~60GB)
7. Click **Install**

**Required Engine Components:**
- Core Engine Components (required)
- Starter Content (recommended)
- Engine Source Code (optional, for debugging)

#### 2. Visual Studio 2022

**Download:** [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/)

**Required Workloads:**
- ✅ Game development with C++
- ✅ .NET desktop development
- ✅ Desktop development with C++

**Required Individual Components:**

Open Visual Studio Installer → Modify → Individual Components:

- ✅ `MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.38)`
- ✅ `MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.44)`
- ✅ `C++ Clang Compiler for Windows`
- ✅ `Windows 11 SDK (10.0.22621.0)`
- ✅ `.NET Framework 4.6.2 targeting pack`
- ✅ `Unreal Engine IDE integration`
- ✅ `Unreal Engine Test Adapter`

**Visual Studio Configuration:**

The project includes a `.vsconfig` file that automates component installation:

1. Open project directory
2. Double-click `.vsconfig`
3. Visual Studio Installer will open with required components selected
4. Click **Modify** to install

#### 3. Git

**Download:** [Git for Windows](https://git-scm.com/download/win)

**Configuration:**
```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

#### 4. Git LFS (Large File Storage)

Required for handling large binary assets:

```bash
git lfs install
```

### System Requirements

#### Minimum:
- **OS:** Windows 10 64-bit (20H2 or later)
- **CPU:** Intel Core i5 / AMD Ryzen 5
- **RAM:** 16 GB
- **GPU:** NVIDIA GTX 1060 / AMD RX 580 (4GB VRAM)
- **Storage:** 100 GB available (SSD recommended)

#### Recommended:
- **OS:** Windows 11 64-bit
- **CPU:** Intel Core i7 / AMD Ryzen 7 (8+ cores)
- **RAM:** 32 GB
- **GPU:** NVIDIA RTX 2070 / AMD RX 5700 XT (8GB VRAM)
- **Storage:** 200 GB SSD

---

## First-Time Setup

### Step 1: Clone the Repository

```bash
# Clone the repository
git clone https://github.com/Traveler3114/Splitstream.git

# Navigate to project directory
cd Splitstream
```

### Step 2: Initialize Git LFS

```bash
# Pull LFS files
git lfs pull
```

This downloads large binary assets (meshes, textures, animations).

### Step 3: Verify Plugin Structure

Ensure plugins are present:

```bash
# Check plugins directory
ls Plugins/
```

Expected output:
```
AdvancedSessions/
AdvancedSteamSessions/
```

### Step 4: Generate Project Files

**Method 1: Right-Click Method (Recommended)**

1. Navigate to project directory in File Explorer
2. Right-click on `Splitstream.uproject`
3. Select **Generate Visual Studio project files**
4. Wait for generation to complete

**Method 2: Command Line**

```bash
# Navigate to Unreal Engine installation
cd "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET"

# Run project file generator
UnrealBuildTool.exe -projectfiles -project="PATH_TO_PROJECT\Splitstream.uproject" -game -engine
```

**Expected Output:**
- `Splitstream.sln` (Solution file)
- `Splitstream.vcxproj` (Project file)
- `.vs/` directory

### Step 5: Open Solution in Visual Studio

```bash
# Open solution file
start Splitstream.sln
```

Or double-click `Splitstream.sln` in File Explorer.

### Step 6: Initial Build

1. In Visual Studio, set build configuration:
   - Configuration: **Development Editor**
   - Platform: **Win64**

2. Build the solution:
   - Press `Ctrl + Shift + B`
   - Or **Build → Build Solution**

3. Wait for compilation (10-30 minutes on first build)

**Build Output:**
```
Build succeeded.
0 Warning(s)
0 Error(s)
Time Elapsed: [time]
```

### Step 7: First Launch

1. In Visual Studio, press `F5` (Start Debugging)
2. Or **Debug → Start Debugging**
3. Unreal Editor will launch
4. Wait for shader compilation (20-60 minutes on first launch)

---

## Building the Project

### Build Configurations

#### Development Editor (Recommended for Development)

**Use Case:** Day-to-day development

**Configuration:**
- Build Configuration: `Development Editor`
- Platform: `Win64`

**Command Line:**
```bash
cd "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles"
Build.bat Splitstream Win64 Development -project="PATH_TO_PROJECT\Splitstream.uproject"
```

**Features:**
- ✅ Editor functionality
- ✅ Good performance
- ✅ Some debugging symbols
- ✅ Hot reload support
- ❌ Not suitable for shipping

#### Debug Game Editor (For Deep Debugging)

**Use Case:** Debugging complex issues

**Configuration:**
- Build Configuration: `DebugGame Editor`
- Platform: `Win64`

**Features:**
- ✅ Full debugging symbols
- ✅ All assertions enabled
- ✅ Memory tracking
- ❌ Very slow performance
- ❌ Large binaries

#### Shipping (Release Build)

**Use Case:** Final distribution

**Configuration:**
- Build Configuration: `Shipping`
- Platform: `Win64`

**Features:**
- ✅ Maximum performance
- ✅ Optimized code
- ✅ No editor
- ✅ Minimal logging
- ❌ No debugging support

### Build Commands

#### Visual Studio

```
1. Select configuration dropdown (top toolbar)
2. Choose "Development Editor" and "Win64"
3. Right-click "Splitstream" in Solution Explorer
4. Click "Build"
```

**Keyboard Shortcuts:**
- `Ctrl + Shift + B` - Build Solution
- `F7` - Build Current Project
- `Ctrl + Break` - Cancel Build

#### Command Line

**Build Development Editor:**
```bash
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
    Splitstream Win64 Development ^
    -project="C:\Path\To\Splitstream\Splitstream.uproject"
```

**Build Shipping:**
```bash
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
    Splitstream Win64 Shipping ^
    -project="C:\Path\To\Splitstream\Splitstream.uproject"
```

**Clean Build:**
```bash
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Clean.bat" ^
    Splitstream Win64 Development ^
    -project="C:\Path\To\Splitstream\Splitstream.uproject"
```

### Build Optimization

#### Faster Compilation

**1. Use Incremental Builds:**
- Only changed files recompile
- Default in Visual Studio

**2. Disable Unneeded Plugins:**
Edit `Splitstream.uproject`, set plugin `Enabled: false`

**3. Use Live Coding (Hot Reload):**
- For small C++ changes
- Click **Live Coding** button in editor
- No full recompile needed

**4. Parallel Compilation:**
Visual Studio automatically uses all CPU cores.

Command line:
```bash
Build.bat Splitstream Win64 Development -maxcpucount:8
```

**5. Unity Build:**
Already enabled in UBT. Combines cpp files for faster compilation.

#### Shader Compilation

**Initial Compilation:**
- Happens on first launch
- Can take 30-60 minutes
- Compiles shaders for all materials

**Speed Up:**
1. Copy `DerivedDataCache` from another machine
2. Use SSD for project
3. Close unnecessary programs during compilation

---

## Running the Game

### Editor (PIE - Play In Editor)

**Method 1: Visual Studio**
```
1. Press F5 (Start Debugging)
2. Editor opens
3. Click Play button or press Alt+P
```

**Method 2: Editor Only**
```
1. Open Splitstream.uproject directly
2. Wait for editor to load
3. Click Play or press Alt+P
```

**PIE Options:**
- `Alt + P` - Play from current camera position
- `Alt + S` - Play as standalone game
- `Alt + Shift + P` - Simulate (no player spawn)

### Standalone Game

**From Editor:**
```
File → Package Project → Windows → Windows (64-bit)
```

**From Build:**
```
Navigate to: Binaries/Win64/
Run: Splitstream.exe
```

### Multiplayer Testing

**Listen Server + Client:**

1. **Editor → Play Dropdown:**
   - Number of Players: 2
   - Net Mode: Play As Listen Server
   - Click Play

2. **Command Line (Multiple Instances):**
```bash
# Server
Splitstream.exe -log -server

# Client 1
Splitstream.exe -log -game 127.0.0.1

# Client 2
Splitstream.exe -log -game 127.0.0.1
```

---

## Development Workflows

### Daily Development Cycle

1. **Pull Latest Changes**
   ```bash
   git pull origin main
   ```

2. **Build Latest Code**
   - Open solution in Visual Studio
   - Press `Ctrl + Shift + B`

3. **Run Editor**
   - Press `F5` in Visual Studio
   - Or open `.uproject` file

4. **Make Changes**
   - Edit C++ in Visual Studio
   - Edit Blueprints/Assets in Editor

5. **Test Changes**
   - Use PIE (Alt+P) for quick tests
   - Use Standalone for full testing

6. **Commit**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin your-branch
   ```

### Hot Reload (Live Coding)

For small C++ changes without restarting editor:

1. Make changes to `.cpp` files
2. Click **Live Coding** button (hammer icon) in editor
3. Wait for compilation
4. Changes applied immediately

**Limitations:**
- Cannot add new UPROPERTY
- Cannot change class hierarchy
- Cannot change function signatures significantly

For major changes, restart the editor.

### Blueprint Development

1. **Create New Blueprint:**
   - Content Browser → Right-click
   - Blueprint Class → Choose Parent
   - Name and save

2. **Edit Blueprint:**
   - Double-click to open
   - Add components
   - Create event graph logic

3. **Compile and Save:**
   - Click Compile button
   - Save with Ctrl+S

### Asset Import

**3D Models:**
1. File → Import to `/Content/Meshes/`
2. Configure import settings
3. Materials created automatically

**Textures:**
1. Drag and drop to Content Browser
2. Or File → Import

**Audio:**
1. Import to `/Content/Sounds/`
2. Supported: WAV, MP3

---

## Testing

### Unit Testing

**C++ Tests:**

Create test file:
```cpp
// SplitstreamTests.cpp
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyTest, 
    "Splitstream.MyTest",
    EAutomationTestFlags::ApplicationContextMask | 
    EAutomationTestFlags::ProductFilter
)

bool FMyTest::RunTest(const FString& Parameters)
{
    TestTrue("Test condition", 1 == 1);
    return true;
}
```

**Run Tests:**
- Editor → Window → Test Automation
- Select tests → Run

### Gameplay Testing

**Test Scenarios:**

1. **Single Player:**
   - Play from main menu
   - Complete tutorial
   - Test all abilities

2. **Multiplayer:**
   - Host game
   - Join game
   - Test replication

3. **AI Testing:**
   - Patrol behavior
   - Detection system
   - Combat AI

**Performance Testing:**
```
Console commands:
stat fps        - Show framerate
stat unit       - Show frame time breakdown
stat game       - Game thread timing
stat memory     - Memory usage
```

---

## Debugging

### Visual Studio Debugging

**Attach Debugger:**

1. **Method 1: Start with Debugger**
   - Press `F5` in Visual Studio
   - Editor launches with debugger attached

2. **Method 2: Attach to Running Process**
   - Debug → Attach to Process
   - Find `UnrealEditor.exe`
   - Click Attach

**Breakpoints:**
- Click left margin in code editor
- Or press `F9` on code line
- Execution pauses at breakpoint

**Debugging Tools:**
- `F10` - Step Over
- `F11` - Step Into
- `Shift + F11` - Step Out
- Watch windows for variable inspection
- Call stack window

### Unreal Engine Debugging

**Visual Logger:**
```cpp
#include "VisualLogger/VisualLogger.h"

UE_VLOG(this, LogTemp, Log, TEXT("Debug message"));
```

View: Window → Developer Tools → Visual Logger

**Output Log:**
```cpp
UE_LOG(LogTemp, Warning, TEXT("Value: %d"), MyValue);
```

View: Window → Developer Tools → Output Log

**Blueprint Debugging:**
- Set breakpoints in Blueprint graphs
- Watch variable values
- Step through execution

**Gameplay Debugger:**
```
Console: showdebug abilitysystem
Console: showdebug ai
```

### Network Debugging

**Network Profiler:**
```
Console: netprofile
```

**Replication Debugging:**
```cpp
// Enable replication logging
LogNet: VeryVerbose
```

**Useful Commands:**
```
stat net        - Network stats
dumpnet         - Dump network info
```

---

## Packaging

### Package Settings

**Edit → Project Settings → Packaging:**

- Build Configuration: Shipping
- Full Rebuild: ✅ (first time)
- Use Pak File: ✅
- Include prerequisites: ✅

### Packaging Steps

1. **Prepare for Package:**
   - Test game thoroughly
   - Fix all errors/warnings
   - Optimize assets

2. **Package Project:**
   ```
   File → Package Project → Windows → Windows (64-bit)
   ```

3. **Select Output Directory:**
   - Choose location outside project
   - e.g., `C:\Builds\Splitstream\`

4. **Wait for Packaging:**
   - Progress shown in Output Log
   - Can take 30-60 minutes

5. **Test Package:**
   ```
   Navigate to output directory
   Run: WindowsNoEditor\Splitstream.exe
   ```

### Package Structure

```
WindowsNoEditor/
├── Splitstream/          # Content directory
│   ├── Content/           # Cooked assets
│   └── Binaries/          # Game binaries
├── Engine/                # Engine binaries
├── Splitstream.exe       # Game executable
└── Prerequisites/         # Required redistributables
```

### Distribution

**Create Installer:**
- Use Inno Setup or NSIS
- Include Prerequisites
- Create shortcuts
- Add uninstaller

**Steam Distribution:**
- Use SteamPipe for upload
- Configure Steam app settings
- Test with Steam beta branch

---

## Troubleshooting

### Build Errors

#### "Missing Modules" Error

**Error:**
```
The following modules are missing or built with a different engine version:
- Splitstream
```

**Solution:**
1. Right-click `Splitstream.uproject`
2. Generate Visual Studio project files
3. Rebuild in Visual Studio

#### "Outdated Precompiled Header" Error

**Solution:**
```bash
# Delete intermediate files
rmdir /s /q Intermediate
rmdir /s /q Binaries

# Rebuild
```

#### "Cannot Open Include File"

**Error:**
```
Cannot open include file: 'SomeHeader.h'
```

**Solution:**
1. Check include path in `.Build.cs`
2. Verify module dependencies
3. Regenerate project files

### Runtime Errors

#### Crash on Startup

**Check:**
1. Output Log for errors
2. Verify plugin compatibility
3. Delete `Saved` and `Intermediate` folders
4. Verify engine version matches project

#### Asset Not Found

**Error:**
```
Warning: Failed to load asset
```

**Solution:**
1. Verify asset path
2. Check Git LFS files downloaded
3. Reimport missing assets

### Performance Issues

#### Low FPS

**Solutions:**
1. Check GPU usage (should be 90%+)
2. Reduce graphics settings
3. Profile with stat commands
4. Check for infinite loops

#### Long Load Times

**Solutions:**
1. Move project to SSD
2. Optimize asset sizes
3. Use streaming levels
4. Reduce texture sizes

### Editor Issues

#### Shader Compilation Stuck

**Solution:**
1. Close editor
2. Delete `Saved/ShaderDebugInfo`
3. Reopen editor

#### Editor Crashes

**Solution:**
1. Check crash logs in `Saved/Crashes/`
2. Verify plugin compatibility
3. Update graphics drivers
4. Disable experimental features

### Git Issues

#### Large File Issues

**Error:**
```
This exceeds GitHub's file size limit
```

**Solution:**
```bash
# Ensure Git LFS is tracking large files
git lfs track "*.uasset"
git lfs track "*.umap"
git add .gitattributes
git commit
```

---

## Best Practices

### Development

1. **Commit Often:** Small, focused commits
2. **Test Before Push:** Build and test locally
3. **Use Branches:** Feature branches for new work
4. **Code Review:** Review changes before merge
5. **Documentation:** Comment complex code

### Performance

1. **Profile Early:** Use profiling tools regularly
2. **Optimize Critical Path:** Focus on hot spots
3. **Asset Budget:** Keep asset sizes reasonable
4. **LODs:** Use Level of Detail for meshes
5. **Batch Operations:** Reduce draw calls

### Collaboration

1. **Communication:** Discuss major changes
2. **Merge Conflicts:** Resolve promptly
3. **Asset Locking:** Coordinate on binary assets
4. **Testing:** Test multiplayer frequently
5. **Documentation:** Keep docs updated

---

## Additional Resources

### Documentation
- [Unreal Engine C++ API](https://docs.unrealengine.com/5.7/en-US/API/)
- [Gameplay Ability System](https://docs.unrealengine.com/5.7/en-US/gameplay-ability-system-for-unreal-engine/)
- [Networking](https://docs.unrealengine.com/5.7/en-US/networking-and-multiplayer-in-unreal-engine/)

### Community
- [Unreal Slackers Discord](https://unrealslackers.org/)
- [Unreal Engine Forums](https://forums.unrealengine.com/)
- [Reddit r/unrealengine](https://reddit.com/r/unrealengine)

### Tools
- [RenderDoc](https://renderdoc.org/) - Graphics debugging
- [Visual Studio Profiler](https://docs.microsoft.com/en-us/visualstudio/profiling/)
- [Unreal Insights](https://docs.unrealengine.com/5.7/en-US/unreal-insights-in-unreal-engine/) - Performance profiling

---

**Last Updated**: January 2025  
**Version**: 1.0
