# Archive Computer and Widget System

This system implements a complete procedural archive computer and widget system for Echoes of Time, including random clue and code logic management.

## System Components

### 1. AProceduralLevelManager
**Location**: `Source/EchoesOfTime/Actors/ProceduralLevelManager.h/cpp`

Central manager for procedural code generation and distribution:
- Generates random 4-digit codes
- Distributes codes to computers and keypad scanners
- Manages clue templates and procedural content
- Provides global singleton access for coordination

**Key Features**:
- Configurable number of codes and code length
- Automatic distribution to available computers
- Template-based clue generation
- Replicated for multiplayer support

### 2. AArchiveComputer  
**Location**: `Source/EchoesOfTime/Actors/ArchiveComputer.h/cpp`

Specialized computer class for archive functionality:
- Extends base AComputer class
- Manages temporal archive data with date-based entries
- Provides access level system (None/Level1/Level2/Level3/Full)
- Integrates with hacking system for unlock mechanism
- Creates and manages ArchiveCalendarWidget interface

**Key Features**:
- Historical archive entries with dates and content
- Procedural clue integration
- Player interaction interface
- Mouse cursor and input mode management

### 3. UArchiveCalendarWidget
**Location**: `Source/EchoesOfTime/Widgets/HUD/ArchiveCalendarWidget.h/cpp`

Interactive calendar widget for archive browsing:
- Month/year navigation system
- Grid-based calendar display
- Entry content viewing
- Date selection and highlighting
- Integration with archive computer data

**Key Features**:
- Calendar grid with day buttons
- Entry highlighting for dates with content
- Scrollable content display
- Month navigation controls
- Close/cleanup functionality

### 4. Enhanced AKeypadScanner Integration
**Location**: `Source/EchoesOfTime/Actors/KeypadScanner/KeypadScanner.cpp` (updated)

Updated keypad scanner to work with procedural manager:
- Integrates with AProceduralLevelManager for code assignment
- Maintains backward compatibility with fallback generation
- Seamless integration with existing keycard unlock system

## System Flow

1. **Level Initialization**:
   - AProceduralLevelManager spawns and generates codes
   - Codes are distributed to available AComputer actors
   - AKeypadScanner actors receive codes from the manager

2. **Player Interaction**:
   - Player finds keypad scanners requiring codes
   - Player must hack computers to reveal codes
   - AArchiveComputer shows calendar interface when hacked
   - Calendar displays procedural clues and historical entries

3. **Code Discovery**:
   - Archive entries contain clues with embedded codes
   - Players navigate calendar to find relevant information
   - Codes are revealed through archive content exploration

## Integration Guide

### Setup Requirements

1. **Level Setup**:
   ```cpp
   // Place one AProceduralLevelManager in your level
   // It will automatically handle code distribution
   ```

2. **Widget Blueprint**:
   Create a Blueprint widget extending UArchiveCalendarWidget with required components:
   - TitleText (UTextBlock)
   - MonthYearText (UTextBlock)
   - CalendarGrid (UUniformGridPanel)
   - EntryScrollBox (UScrollBox)
   - SelectedDateText (UTextBlock)
   - EntryContentText (UTextBlock)
   - PrevMonthButton, NextMonthButton, CloseButton (UButton)

3. **Archive Computer Setup**:
   ```cpp
   // Set ArchiveWidgetClass in Blueprint to your calendar widget
   // Archive computers will automatically integrate with the procedural manager
   ```

### Testing

Use the AArchiveSystemTester actor for automated testing:
- Place in level to run comprehensive system tests
- Tests procedural manager, archive computer, and keypad integration
- Results displayed on-screen and in logs

### Customization

1. **Clue Templates**: Modify `ClueTemplates` array in AProceduralLevelManager
2. **Archive Content**: Add entries in `AArchiveComputer::InitializeArchiveData()`
3. **Access Levels**: Customize unlock conditions in AArchiveComputer
4. **Visual Design**: Style calendar widget in Blueprint editor

## Technical Notes

- System uses Unreal Engine's replication for multiplayer support
- Widget creation requires valid PlayerController for proper setup
- Calendar supports leap year calculations for February
- All components integrate with existing IInteractable system
- Logging available via LogTemp category for debugging

## File Structure

```
Source/EchoesOfTime/
├── Actors/
│   ├── ProceduralLevelManager.h/cpp
│   ├── ArchiveComputer.h/cpp
│   └── KeypadScanner/KeypadScanner.cpp (updated)
├── Widgets/HUD/
│   └── ArchiveCalendarWidget.h/cpp
└── Testing/
    ├── ArchiveSystemTester.h/cpp
    └── ArchiveSystemIntegrationGuide.txt
```

This system provides a complete, integrated solution for procedural archive management with minimal changes to existing code while adding significant gameplay depth through temporal data exploration.