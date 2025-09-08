# Archive Puzzle System

## Overview

The Archive Puzzle System provides a procedural way to set up archive computer puzzles with keypad scanners. The system consists of several key components that work together to create a fully functional puzzle setup.

## Components

### 1. ArchiveComputer (AArchiveComputer)
- **Purpose**: Specialized computer terminals for archive puzzles
- **Features**: 
  - Procedural unique naming (e.g., "Data Node Alpha", "Storage Terminal Beta")
  - Archive-specific properties and identification
  - Inherits all functionality from the base Computer class
- **Usage**: Place in level like regular computers, they will be automatically managed by the ProceduralLevelManager

### 2. ProceduralLevelManager (UProceduralLevelManager)
- **Purpose**: Core management system for procedural puzzle generation
- **Features**:
  - Finds existing ArchiveComputers and KeypadScanners in the level
  - Generates unique codes for each scanner
  - Links scanners to computers with proper code assignment
  - Validates puzzle setup integrity
- **Configuration**: Uses FPuzzleConfiguration struct for flexible setup

### 3. ArchivePuzzleComponent (UArchivePuzzleComponent)
- **Purpose**: Component that can be added to GameMode or other actors
- **Features**:
  - Easy integration into existing game systems
  - Blueprint-friendly interface
  - Auto-setup functionality with configurable delay
  - Real-time puzzle validation and statistics

### 4. ArchivePuzzleTestActor (AArchivePuzzleTestActor)
- **Purpose**: Standalone test actor for validating puzzle functionality
- **Features**:
  - Comprehensive puzzle testing
  - Detailed logging and validation
  - On-screen debug messages
  - Auto-generation for quick testing

## Configuration

### FPuzzleConfiguration
```cpp
struct FPuzzleConfiguration
{
    int32 NumArchiveComputers = 3;      // Number of archive computers to use
    int32 NumKeypadScanners = 1;        // Number of keypad scanners
    int32 DifficultyLevel = 1;          // Difficulty setting (future expansion)
    bool bUseUniqueCodesPerScanner = true; // Ensure each scanner has unique code
};
```

## Usage Examples

### Method 1: Using ArchivePuzzleComponent (Recommended)

1. Add the ArchivePuzzleComponent to your GameMode or other actor
2. Configure the puzzle settings in the editor
3. Enable auto-setup or call SetupArchivePuzzle() manually

```cpp
// In your GameMode or other actor
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puzzle")
UArchivePuzzleComponent* ArchivePuzzleComp;

// In constructor
ArchivePuzzleComp = CreateDefaultSubobject<UArchivePuzzleComponent>(TEXT("ArchivePuzzleComponent"));

// Manual setup (if auto-setup is disabled)
ArchivePuzzleComp->SetupArchivePuzzle();
```

### Method 2: Using ProceduralLevelManager Directly

```cpp
UProceduralLevelManager* LevelManager = NewObject<UProceduralLevelManager>();
LevelManager->Initialize(GetWorld());

FPuzzleConfiguration Config;
Config.NumArchiveComputers = 5;
Config.NumKeypadScanners = 2;

bool bSuccess = LevelManager->GenerateArchivePuzzle(Config);
```

### Method 3: Using ArchivePuzzleTestActor (Testing)

1. Place ArchivePuzzleTestActor in your level
2. Configure test settings in the editor
3. Enable auto-generation or call GenerateTestPuzzle() manually

## Level Setup

1. **Place ArchiveComputers**: Add ArchiveComputer actors to your level where you want terminals
2. **Place KeypadScanners**: Add KeypadScanner actors where players will enter codes
3. **Add Management Component**: Add ArchivePuzzleComponent to your GameMode or place ArchivePuzzleTestActor
4. **Configure**: Set up puzzle configuration (number of computers, scanners, etc.)
5. **Test**: The system will automatically link scanners to computers with unique codes

## Integration with Existing Systems

The Archive Puzzle System is designed to work seamlessly with existing Echoes of Time systems:

- **Backward Compatible**: Works with existing Computer and KeypadScanner classes
- **Networking**: All components support Unreal's replication system
- **Blueprint Friendly**: All major functions are Blueprint callable
- **Modular**: Components can be used independently or together

## Validation and Debugging

The system includes comprehensive validation:

- **Puzzle Validation**: Ensures all components are properly linked
- **Code Uniqueness**: Prevents duplicate codes when configured
- **Detailed Logging**: Extensive UE_LOG messages for debugging
- **On-Screen Messages**: Visual feedback during setup and testing
- **Statistics**: Real-time puzzle statistics and status reporting

## Future Enhancements

The system is designed to be easily extensible:

- **Difficulty Scaling**: Framework for implementing difficulty-based puzzle generation
- **Custom Naming**: Support for custom archive terminal naming schemes
- **Advanced Linking**: More sophisticated computer-scanner relationship logic
- **Puzzle Templates**: Predefined puzzle configurations for different scenarios