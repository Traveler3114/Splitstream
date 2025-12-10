# Game Design Document
# Echoes of Time

**Version:** 1.0  
**Last Updated:** December 10, 2025  
**Project Type:** Multiplayer Stealth Heist Game  
**Engine:** Unreal Engine 5.7  
**Platform:** PC (Windows)  
**Target Audience:** Players aged 16+ who enjoy stealth games, puzzle-solving, and cooperative gameplay  

---

## Table of Contents

1. [High-Level Concept](#1-high-level-concept)
2. [Core Gameplay Pillars](#2-core-gameplay-pillars)
3. [Game Narrative & Setting](#3-game-narrative--setting)
4. [Core Mechanics](#4-core-mechanics)
5. [Player Character & Abilities](#5-player-character--abilities)
6. [Game Systems](#6-game-systems)
7. [AI & NPCs](#7-ai--npcs)
8. [Level Design](#8-level-design)
9. [Puzzle Design](#9-puzzle-design)
10. [Multiplayer Design](#10-multiplayer-design)
11. [Progression & Replayability](#11-progression--replayability)
12. [User Interface](#12-user-interface)
13. [Art Direction](#13-art-direction)
14. [Audio Design](#14-audio-design)
15. [Technical Architecture](#15-technical-architecture)
16. [Development Roadmap](#16-development-roadmap)

---

## 1. High-Level Concept

### 1.1 Vision Statement

**Echoes of Time** is a cooperative stealth heist game that combines the tension of infiltration with the mind-bending possibilities of time manipulation. Players work together to pull off elaborate heists by switching between Past and Future timelines, using information from one era to overcome obstacles in another. The game emphasizes careful planning, teamwork, and creative problem-solving over combat.

### 1.2 Core Experience

Players experience the thrill of:
- **Strategic Planning**: Scouting environments across two timelines to find the optimal approach
- **Temporal Puzzle-Solving**: Using clues from the Past to solve puzzles in the Future (and vice versa)
- **Tense Stealth**: Avoiding detection by guards, civilians, and security systems
- **Cooperative Teamwork**: Coordinating with teammates to overcome challenges
- **Procedural Variety**: Each heist offers unique challenges with randomized elements

### 1.3 Unique Selling Points

1. **Dual Timeline Mechanic**: Seamlessly switch between Past and Future eras, each with distinct environments and challenges
2. **Ghost Echo System**: See recordings of past player movements to coordinate complex maneuvers
3. **Cooperative Heist Gameplay**: Designed for 1-4 players with scalable difficulty
4. **Procedural Generation**: Randomized security codes, NPC placements, and puzzle solutions ensure high replayability
5. **Non-Lethal Focus**: Emphasis on stealth and evasion rather than combat

---

## 2. Core Gameplay Pillars

### Pillar 1: Time Manipulation
The ability to switch between Past and Future timelines is the foundation of all gameplay. This mechanic enables:
- Discovery of hidden information and paths
- Solving of complex environmental puzzles
- Strategic planning through temporal reconnaissance
- Unique cooperative strategies

### Pillar 2: Stealth & Detection
Tension is maintained through a sophisticated detection system:
- Multiple threat types (guards, civilians, cameras, sensors)
- Progressive detection with clear feedback
- Movement options that balance speed and stealth
- Consequences for being detected

### Pillar 3: Puzzle-Solving
Environmental puzzles gate progress and require:
- Information gathering from documents and computers
- Pattern recognition and logical thinking
- Coordination between timelines
- Team collaboration in multiplayer

### Pillar 4: Cooperative Play
The game is designed for seamless cooperation:
- Shared objectives and synchronized puzzle states
- Individual player abilities and inventories
- Communication and coordination requirements
- Scalable difficulty based on player count

---

## 3. Game Narrative & Setting

### 3.1 Story Overview

The player characters are members of an elite group of temporal operatives who have discovered a way to shift between two connected timelines: the Past (historical period) and the Future (modern day). They use this ability to conduct heists, stealing valuable artifacts and information by exploiting the differences between the two eras.

### 3.2 Timeline Settings

#### The Past Timeline
- **Era**: 1950s-1960s aesthetic
- **Characteristics**: 
  - Analog security systems
  - Paper records and filing systems
  - Mechanical locks and safes
  - Lower-tech surveillance
  - Different architectural layouts (older construction, more ornate)

#### The Future Timeline
- **Era**: Present day (2020s)
- **Characteristics**:
  - Digital security systems
  - Computer databases
  - Electronic locks and biometric scanners
  - Modern surveillance technology
  - Updated architecture (renovations, new sections)

### 3.3 Mission Context

Each heist takes place in a specific location (bank, museum, corporate office) that exists in both timelines. Players must:
1. Research the target location in both eras
2. Identify security vulnerabilities
3. Gather necessary tools and information
4. Execute the heist
5. Escape without detection

### 3.4 Locations

**Primary Heist Location: The Bank**
- A grand bank building that has existed in both eras
- Past: Classic bank with vaults, filing rooms, and manager offices
- Future: Modern financial institution with upgraded security
- Multiple entry points, ventilation shafts, and hidden passages
- Procedurally populated with NPCs and security systems

---

## 4. Core Mechanics

### 4.1 Timeline Switching

**Mechanic**: Press a dedicated button to instantly switch between Past and Future timelines

**Design Principles**:
- Instant transition with visual effects (shimmer, color shift)
- No cooldown - can switch freely
- Timeline-specific objects become visible/invisible
- Player position maintained across timelines
- Ghost characters only visible when using Past Echo ability in Future

**Gameplay Applications**:
- Scout Future timeline for security patterns
- Switch to Past to find alternative routes
- Gather information in one timeline to use in another
- Coordinate with teammates in different timelines

### 4.2 Movement System

#### Walking (Default)
- Normal movement speed
- Standard detection radius
- Balanced approach

#### Crouching
- **Activation**: Hold Ctrl
- **Effects**:
  - Reduced movement speed (50% of walk speed)
  - Reduced detection radius
  - Can fit through vents and low spaces
  - Quieter footsteps
- **Use Cases**: Sneaking past guards, navigating secure areas

#### Sprinting
- **Activation**: Hold Shift
- **Effects**:
  - Increased movement speed (200% of walk speed)
  - Increased detection radius
  - Louder footsteps
  - Cannot perform actions while sprinting
- **Use Cases**: Escaping detection, covering large distances quickly

### 4.3 Interaction System

**Primary Interaction**: Press E to interact with objects

**Interactable Object Types**:
- **Doors**: Open/close, require keycards or codes
- **Computers**: Access databases, find codes
- **Items**: Pick up keycards, tools, documents
- **Hackable Objects**: Cameras, electronic locks
- **Searchable Objects**: Desks, lockers, filing cabinets
- **Puzzle Elements**: Keypads, wire panels, levers

**Interaction Feedback**:
- Highlight system shows interactable objects
- Prompt displays available action
- Progress bar for timed interactions
- Audio/visual feedback on completion

### 4.4 Detection & Stealth

#### Detection System

**Progressive Detection**:
1. **Unaware** (0%): NPC has not noticed player
2. **Suspicious** (1-50%): NPC is becoming aware, detection builds slowly
3. **Alert** (51-99%): NPC is actively tracking player, detection builds quickly
4. **Fully Detected** (100%): Alarm triggered, guards called, mission compromised

**Detection Factors**:
- **Distance**: Closer = faster detection
- **Line of Sight**: Must have clear view
- **Movement State**: Sprinting > Walking > Crouching
- **Illegal Status**: Carrying illegal items increases detection speed
- **Light Level**: Brighter areas increase detection
- **Field of View**: Guards and cameras have limited FOV

**Threat Types**:

1. **Guards** (Active AI)
   - Patrol designated routes
   - Investigate suspicious activity
   - Can pursue and detain players
   - NetUpdateFrequency: 30Hz for responsiveness

2. **Civilians** (Passive AI)
   - Wander work areas randomly
   - Can witness illegal activity
   - Report to guards if suspicious
   - NetUpdateFrequency: 10Hz (less critical)

3. **Security Cameras** (Static)
   - Rotate on fixed axis
   - Limited field of view (cone-shaped detection)
   - Can be hacked to disable
   - Wall-mounted at strategic locations

4. **Metal Detectors** (Trigger)
   - Detect weapons and illegal items
   - Located at choke points
   - Instant alarm on detection
   - Can sometimes be avoided or disabled

**Detection UI**:
- Detection meter fills progressively
- Color-coded: White → Yellow → Orange → Red
- Warning indicators when being detected
- Directional markers show threat locations

### 4.5 Inventory System

**Capacity**: 8 slots, accessed via number keys 1-8

**Inventory Management**:
- **Pick Up**: E key on item pickups
- **Switch Item**: Press number keys
- **Drop Item**: Press G to drop active item
- **Active Item**: Displayed in hand and on character model

**Item Categories**:

1. **Tools**
   - Lockpicks: Open mechanical locks
   - Hacking Device: Disable electronics
   - Screwdriver: Open panels, vents

2. **Access Items**
   - Keycards: Various security levels
   - Code Documents: Contain keypad codes
   - ID Badges: Bypass certain checkpoints

3. **Weapons** (Illegal)
   - Pistol: Basic firearm
   - Triggers metal detectors
   - Increases detection speed

4. **Documents**
   - Newspapers: Contain date clues
   - Notes: Puzzle hints
   - Files: Story information

**Illegal Item System**:
- Certain items marked as illegal
- Carrying illegal items adds gameplay tag "Character.Status.Illegal"
- Increases detection speed by guards and civilians
- Triggers metal detectors
- Must be dropped or hidden to remove tag

---

## 5. Player Character & Abilities

### 5.1 Character Concept

Players control temporal operatives—professional thieves with access to time-manipulation technology. They are highly skilled infiltrators trained in:
- Stealth and evasion
- Lock manipulation
- Computer hacking
- Puzzle-solving
- Temporal navigation

### 5.2 Ability System (Gameplay Ability System)

The game uses Unreal Engine's Gameplay Ability System (GAS) for all player abilities, providing:
- Network replication
- Ability activation/cancellation
- Cost and cooldown management
- Gameplay tags for state tracking
- Extensibility for future abilities

#### Core Abilities

**1. Hacking**
- **Activation**: E key on hackable object
- **Gameplay Tag**: `Character.Status.Hacking`
- **Requirements**: Hacking device in inventory
- **Execution**:
  - Mini-game: Match symbols or complete circuit
  - Timer-based (10-20 seconds)
  - Vulnerable during hacking
- **Targets**: 
  - Security cameras
  - Electronic locks
  - Computers
  - Access panels
- **Network**: Server-authoritative with client prediction

**2. Lock Picking**
- **Activation**: E key on locked door
- **Gameplay Tag**: `Character.Status.LockPicking`
- **Requirements**: Lockpicks in inventory
- **Execution**:
  - Precision mini-game
  - Multiple pins to set
  - Failed attempts may break lockpick
- **Targets**: Mechanical locks on doors and containers
- **Network**: Server-authoritative

**3. Searching**
- **Activation**: E key on searchable object
- **Gameplay Tag**: `Character.Status.Searching`
- **Requirements**: None
- **Execution**:
  - Timer-based interaction (5-10 seconds)
  - Progress bar shown
  - Reveals items and documents
- **Targets**:
  - Desks
  - Filing cabinets
  - Lockers
  - Trash cans
- **Network**: Server validates, client shows feedback

**4. Past Echo (Future Timeline Only)**
- **Activation**: Q key (toggle)
- **Gameplay Tag**: `Character.Status.PastEcho`
- **Requirements**: In Future timeline
- **Execution**:
  - Instant activation
  - No duration limit
  - Visual overlay effect
  - Shows ghost characters from Past timeline
- **Purpose**: Coordinate movements, see past player paths
- **Network**: Client-side visual effects, server tracks state

**5. Weapon System**
- **Primary Fire**: Left Mouse Button
  - Fires projectile from pistol
  - Gameplay Tag: `Character.Status.Firing`
- **Aim Down Sights**: Right Mouse Button
  - Zooms camera
  - Increases accuracy
  - Gameplay Tag: `Character.Status.Aiming`
  - Blocks movement with tag: `Character.Status.Block.Movement`

### 5.3 Attribute System

**Player Attribute Set**:
- **Health**: Player vitality (not heavily used in stealth-focused gameplay)
- **Stamina**: Used for sprinting and abilities
- **Detection Level**: Current detection percentage (0-100%)

---

## 6. Game Systems

### 6.1 Timeline System

**Implementation**: `ETimelineEra` enum with Past/Future values

**Timeline-Specific Objects**:

1. **Past Doors**
   - Only exist in Past timeline
   - Invisible/non-collidable in Future
   - May reveal alternate routes

2. **Future Doors**
   - Only exist in Future timeline
   - Represent modern additions/renovations
   - May block old routes

3. **Double Doors**
   - Exist in both timelines
   - May have different states (locked/unlocked) in each era
   - Key architectural elements

4. **Past Items**
   - Only collectible in Past timeline
   - May contain historical information

5. **Future Items**
   - Only collectible in Future timeline
   - Modern tools and technology

**Ghost Character System**:
- Records player movement in Past timeline
- Plays back as semi-transparent ghost in Future timeline
- Only visible when Past Echo ability is active
- Updates mesh and materials at 20Hz for performance
- Smooth interpolation maintains visual quality
- Used for puzzle-solving and coordination

### 6.2 Procedural Generation System

**Purpose**: Ensure high replayability by randomizing key elements each playthrough

**Procedurally Generated Elements**:

1. **Security Codes** (`CodeGenerator`)
   - 4-digit keypad codes
   - Generated at level start
   - Distributed in documents and computers
   - Timed expiration (500ms check interval)

2. **Calendar Dates**
   - Random historical dates
   - Used in calendar puzzles
   - Found in research archives

3. **NPC Placement**
   - Guards spawn at designated guard spawn points
   - Civilians spawn at civilian spawn points
   - Random desk assignments for civilians
   - Procedural name assignment from name pools

4. **NPC Names** (ProceduralGenConstants)
   - **Civilian Names**: Pool of ~30 common names
   - **Guard Names**: Pool of ~30 different names
   - Assigned randomly to prevent repetition

5. **Wire Puzzle Solutions**
   - Random wire color sequences
   - Different for each playthrough
   - Must be discovered through gameplay

6. **Item Distribution**
   - Random placement of key items
   - Keycards in different locations
   - Documents scattered procedurally

**ProceduralLevelGenerator**:
- Fully refactored with extracted helper methods
- Clear section organization
- Runs at level start
- Populates spawn points
- Generates puzzle solutions
- Assigns NPC schedules

### 6.3 Interaction System

**Highlight System**:
- Raytrace from camera to detect interactable objects
- Implements `IInteractable` interface
- Updates at 10Hz (100ms intervals) for performance
- Shows object name and available action
- Different colors for different interaction types

**Interaction Interface** (`IInteractable`):
```cpp
virtual void Interact(ADefaultCharacter* Character);
virtual FString GetInteractionPrompt();
virtual bool CanInteract(ADefaultCharacter* Character);
```

**Item Requirement Interface** (`IRequiresItem`):
```cpp
virtual bool HasRequiredItem(ADefaultCharacter* Character);
virtual FString GetRequiredItemName();
```

### 6.4 Objective System

**Mission Objectives**:
1. Infiltrate the location
2. Reach the target area
3. Obtain the target item/information
4. Escape without triggering alarms
5. Optional: Remain undetected throughout

**Objective Tracking**:
- UI displays current objective
- Updates as objectives are completed
- Shared across all players in multiplayer
- Synchronized via game state

---

## 7. AI & NPCs

### 7.1 Guard AI

**Character Class**: `GuardCharacter`

**Behavior System**: State Tree AI (modern Unreal system)

**AI States**:
1. **Patrol**:
   - Follow designated nav nodes
   - Regular walking speed
   - Alert for suspicious activity
   - Uses AI perception

2. **Investigate**:
   - Move to last known player position
   - Search area systematically
   - Elevated alert status

3. **Chase**:
   - Active pursuit of detected player
   - Increased movement speed
   - Call for backup

4. **Combat** (if implemented):
   - Engage player with weapon
   - Take cover
   - Coordinate with other guards

**Detection Parameters**:
- Sight range: 2000 units
- Sight angle: 90 degrees (cone)
- Hearing range: 1000 units
- Detection speed: Variable based on distance and movement

**Performance Optimizations**:
- Tick disabled (`bCanEverTick = false`)
- NetUpdateFrequency: 30Hz for responsive AI
- State Tree handles behavior efficiently

### 7.2 Civilian AI

**Character Class**: `CivilianCharacter`

**Behavior**: Simple wander AI

**Characteristics**:
- Randomly assigned desk location
- Procedurally assigned name
- Wanders within work area radius
- Can witness illegal activity
- Reports to guards if suspicious

**AI Behavior**:
- Move to random points near assigned desk
- Pause at locations (simulating work)
- Limited detection capability
- Does not pursue players

**Performance Optimizations**:
- Tick disabled
- NetUpdateFrequency: 10Hz (lower priority)
- Simple movement patterns

### 7.3 Security Camera

**Actor Class**: `SecurityCamera`

**Behavior**: Static rotation with detection cone

**Characteristics**:
- Wall-mounted position
- Rotates on yaw axis (-60° to +60°)
- Red light indicates active
- Blue light indicates hacked/disabled
- Detection cone visualization

**Detection**:
- Fixed rotation speed
- Cone-shaped detection area
- Line of sight check
- Distance-based detection speed

**Vulnerability**: Can be hacked to disable

### 7.4 Drone Pawn

**Pawn Class**: `DronePawn`

**Behavior**: Patrol flight paths

**Characteristics**:
- Aerial surveillance
- 360-degree detection
- Can access multiple vertical levels
- Difficult to avoid

**Future Enhancement**: Currently basic implementation

---

## 8. Level Design

### 8.1 Level Design Philosophy

**Principles**:
1. **Multiple Paths**: Every objective has multiple approach routes
2. **Vertical Gameplay**: Use of multiple floors, vents, and elevation
3. **Timeline Asymmetry**: Different routes available in Past vs Future
4. **Risk vs Reward**: Faster routes have higher detection risk
5. **Environmental Storytelling**: Level design hints at location history

### 8.2 Bank Level Structure

**Exterior**:
- Front entrance (heavily guarded)
- Side entrance (service door)
- Roof access (via adjacent building)
- Sewer entrance (Past timeline only)

**Ground Floor**:
- **Past**: Classic bank lobby, teller windows, customer area
- **Future**: Modern banking floor, ATMs, security checkpoint
- Metal detectors at main entrance
- Security cameras at key points
- Civilian NPCs (bank employees, customers)

**Second Floor**:
- **Past**: Manager offices, file room, staff areas
- **Future**: Modernized offices, server room, break rooms
- Fewer NPCs but more guards
- Key puzzle locations
- Access to vault level

**Vault Level (Basement)**:
- **Past**: Classic vault with mechanical locks
- **Future**: Modern vault with electronic security
- Highest security concentration
- Main objective location
- Multiple access points from above

**Maintenance Areas**:
- Ventilation shafts connecting floors
- Electrical rooms with fuse boxes
- Maintenance corridors (Past timeline access)
- Boiler room (Past timeline)

**Rooftop**:
- Helicopter pad (Future)
- Maintenance access
- Alternative escape route
- Drone patrol area

### 8.3 Environmental Hazards

**Laser Grids**:
- Visible red laser beams
- Trigger alarms on contact
- Can be disabled via power boxes
- More common in Future timeline

**Metal Detectors**:
- Placed at choke points
- Detect weapons and illegal items
- Force players to choose routes carefully
- Can sometimes be bypassed

**Alarm Systems**:
- Triggered by full detection
- Summons all guards to player location
- Locks down certain areas
- Mission fails if alarm persists

### 8.4 Level Spawn System

**Spawn Point Types**:
1. **Guard Spawn Points**: Where guards begin patrol
2. **Civilian Spawn Points**: Where civilians are placed
3. **Item Spawn Points**: Where items can appear
4. **Nav Nodes**: AI patrol waypoints

**Procedural Population**:
- Level designer places spawn points
- ProceduralLevelGenerator populates at runtime
- Randomization ensures variety
- Maintains gameplay balance

---

## 9. Puzzle Design

### 9.1 Puzzle Philosophy

**Design Goals**:
- Require information gathering across timelines
- Encourage exploration and observation
- Provide "aha!" moments when solution is discovered
- Support both solo and cooperative solving
- Maintain gameplay pacing

### 9.2 Keypad Puzzles

**Mechanic**: Enter 4-digit code to unlock door or safe

**Implementation**:
- `KeypadScanner`: Main keypad actor
- `KeypadButton`: Individual number buttons (0-9, Clear, Enter)
- `CodeGenerator`: Generates and manages codes

**Puzzle Flow**:
1. Player finds locked keypad
2. Searches for code in:
   - Documents (newspapers, notes)
   - Computers (archive research)
   - NPC dialogue (future enhancement)
3. Returns to keypad and enters code
4. Success: Door unlocks, access granted
5. Failure: Error message, must re-enter

**Design Variations**:
- Different codes for different locations
- Time-limited codes (future enhancement)
- Partial codes requiring assembly

### 9.3 Wire Puzzles

**Mechanic**: Connect wires in correct sequence to complete circuit

**Implementation**:
- `WirePuzzleManager`: Controls puzzle state
- `WireActor`: Individual wire pieces
- `WireDeviceActor`: Connection endpoints

**Puzzle Flow**:
1. Player discovers wire puzzle panel
2. Observes disconnected wires
3. Must determine correct wire sequence:
   - Color matching
   - Pattern recognition
   - Information from documents
4. Connects wires in sequence
5. Success: Circuit complete, door/device activates
6. Failure: Can retry, no penalty

**Visual Feedback**:
- Wires highlight on hover
- Connected wires show active power
- Completed puzzle shows visual confirmation

### 9.4 Lever Combination Puzzles

**Mechanic**: Activate levers in specific order

**Implementation**:
- `LeverManager`: Tracks lever sequence
- `LeverActor`: Individual lever switches

**Puzzle Flow**:
1. Player finds multiple levers
2. Must discover correct activation order:
   - Symbols on levers
   - Clues in environment
   - Trial and error
3. Activates levers in sequence
4. Success: Mechanism activates (door opens, elevator works)
5. Failure: Reset, must try again

**Design Variations**:
- 2-5 levers per puzzle
- Visual symbols for hints
- Sound cues for correct activation

### 9.5 Calendar/Date Puzzles

**Mechanic**: Enter specific date to unlock system

**Implementation**:
- `CalendarWidget`: UI for date entry
- `CalendarResultWidget`: Success/failure display
- `ArchiveComputer`: Research terminal

**Puzzle Flow**:
1. Player finds locked archive or safe requiring date
2. Researches in Past timeline:
   - Old newspapers with historical events
   - Archive computers with records
   - Wall calendars with marked dates
3. Switches to Future timeline
4. Enters date into calendar interface
5. Success: Access granted to secure area
6. Failure: Can retry with different date

**Timeline Integration**:
- Past timeline: Find historical information
- Future timeline: Use information to unlock modern systems
- Encourages timeline switching

### 9.6 Laser Security Navigation

**Mechanic**: Navigate around or disable laser detection grids

**Implementation**:
- `LaserManager`: Controls laser grid
- `LaserSensor`: Individual laser beams

**Puzzle Flow**:
1. Player encounters laser grid blocking path
2. Options:
   - **Navigate Around**: Crouch, jump, time movements
   - **Disable**: Find and hack power source
   - **Alternative Route**: Use vents or other paths
3. Success: Pass through safely
4. Failure: Triggers alarm, detection increases

**Design Variations**:
- Static laser patterns
- Moving lasers
- Timed laser sequences

---

## 10. Multiplayer Design

### 10.1 Multiplayer Philosophy

**Core Principles**:
1. **Cooperative Focus**: Players work together, not against each other
2. **Scalable Difficulty**: Adjust challenge based on player count
3. **Individual Agency**: Each player has their own inventory and abilities
4. **Shared Progress**: Objectives and puzzle states synchronized
5. **Communication Important**: Success requires coordination

### 10.2 Network Architecture

**Server-Client Model**:
- **Server-Authoritative**: All gameplay logic validated on server
- **Client Prediction**: Smooth movement and interactions
- **RPCs**: Remote Procedure Calls for player actions
- **Replication**: State synchronized to all clients

**Network Optimization**:
- **Variable Update Frequencies**:
  - Players: 60Hz (high responsiveness)
  - Guards: 30Hz (active AI)
  - Civilians: 10Hz (lower priority)
  - Static objects: 1Hz (minimal updates)
- **Tick Optimizations**: Many actors use timers instead of Tick
- **Interest Management**: Future enhancement for large levels

**Replication Examples**:
```cpp
// Inventory replication
UPROPERTY(Replicated)
TArray<FInventorySlot> InventorySlots;

// Server RPC for interaction
UFUNCTION(Server, Reliable)
void ServerInteract(AActor* InteractableActor);

// Multicast for effects
UFUNCTION(NetMulticast, Reliable)
void MulticastPlayEffect(FVector Location);
```

### 10.3 Session Management

**Lobby System**:
- **LobbyGameMode**: Pre-game lobby session
- **LobbyUI**: Player list, ready system
- **Friend System**: Steam and EOS integration

**Session Flow**:
1. **Main Menu**: Player chooses Host or Join
2. **Lobby Creation**: Host creates session, invites friends
3. **Player Joining**: Friends join via invite or server browser
4. **Ready System**: All players mark ready
5. **Game Start**: Host starts game, loads level
6. **Travel**: All clients travel to game level
7. **Gameplay**: Cooperative heist execution
8. **Return to Lobby**: Option to replay or return to menu

**Plugins Used**:
- **Advanced Sessions**: Session creation and management
- **Advanced Steam Sessions**: Steam integration
- **OnlineSubsystemEOS**: Epic Online Services support

### 10.4 Cooperative Gameplay

**Shared Elements**:
- **Objectives**: All players work toward same goals
- **Puzzle States**: Keypad codes, lever positions synchronized
- **Detection**: One player's detection affects team alert level
- **Timeline**: Players can be in different timelines simultaneously

**Individual Elements**:
- **Inventory**: Each player has own 8 slots
- **Abilities**: Each player uses their own abilities
- **Position**: Players move independently
- **Detection Level**: Individual detection meters

**Cooperative Strategies**:
1. **Timeline Coordination**:
   - One player in Past, one in Future
   - Communicate about guard positions
   - Use Past Echo to see each other's movements

2. **Distraction**:
   - One player draws guard attention
   - Other player accesses restricted area
   - Coordinated timing

3. **Resource Sharing**:
   - Drop items for teammates
   - Distribute keycards and tools
   - Share discovered information

4. **Multi-Stage Puzzles**:
   - Simultaneous lever activation
   - Coordinated door opening
   - Team-based puzzle solving

### 10.5 Multiplayer UI

**HUD Elements**:
- **Team Status**: Show teammate names, health, detection
- **Objective Tracker**: Shared objective display
- **Chat System**: Text communication (future enhancement)
- **Waypoint System**: Mark locations for teammates (future enhancement)

**Lobby Interface**:
- **Player List**: Show all connected players
- **Ready Status**: Checkmarks for ready players
- **Friend List**: Invite friends from Steam/EOS
- **Settings**: Game mode options, difficulty

---

## 11. Progression & Replayability

### 11.1 Progression Systems

**Current Implementation**:
- Mission-based progression (complete heist objectives)
- No persistent player progression between missions
- Focus on mastery and optimization

**Future Enhancements**:
1. **Skill Progression**:
   - Faster hacking
   - Quieter movement
   - Improved detection resistance

2. **Equipment Unlocks**:
   - Advanced tools
   - Better weapons
   - Specialized gadgets

3. **Mission Campaign**:
   - Sequential heist locations
   - Escalating difficulty
   - Story progression

### 11.2 Replayability Features

**Procedural Generation**:
- Different security codes each playthrough
- Randomized NPC placement and names
- Variable item locations
- Unique wire puzzle solutions

**Multiple Approaches**:
- Different entry and exit routes
- Various solution strategies
- Timeline-based variations
- Player-chosen methods

**Challenge Modes** (Future):
- **Speed Run**: Complete heist as fast as possible
- **Ghost Mode**: Never be detected
- **No Tools**: Complete without hacking/lockpicking
- **Multiplayer Challenges**: Team-based competitive times

**Scoring System** (Future):
- Time completion bonus
- Detection penalties
- Objectives completed
- Optional challenges
- Leaderboards for competition

---

## 12. User Interface

### 12.1 HUD Design

**In-Game HUD Elements**:

1. **Interaction Prompt** (Center-Bottom)
   - Shows available action (e.g., "Press E to Hack")
   - Object name display
   - Updates at 10Hz for performance

2. **Detection Meter** (Top-Center)
   - Horizontal bar filling left to right
   - Color-coded progression: White → Yellow → Orange → Red
   - Threat direction indicators
   - Fades when not detected

3. **Inventory Display** (Bottom-Right)
   - 8 slot grid with numbers 1-8
   - Active slot highlighted
   - Item icons and names
   - Illegal item indicator (red border)

4. **Ability Indicators** (Bottom-Center)
   - Shows active abilities
   - Cooldown timers (future enhancement)
   - Resource costs (future enhancement)

5. **Objective Display** (Top-Left)
   - Current objective text
   - Optional objective checkbox
   - Updates as mission progresses

6. **Teammate Status** (Left Side) - Multiplayer
   - Player names
   - Health/status bars
   - Distance indicators
   - Detection status

**Widget Classes**:
- `DefaultHUD`: Main HUD widget
- `DetectionWidget`: Detection meter
- `InventoryWidget`: Inventory display
- `InteractionWidget`: Interaction prompts
- `ObjectiveWidget`: Mission objectives

### 12.2 Menu Systems

**Main Menu**:
- **Single Player**: Start solo game
- **Multiplayer**: Host or join session
- **Options**: Graphics, audio, controls
- **Credits**: Team and acknowledgments
- **Exit**: Quit game

**Pause Menu** (In-Game):
- **Resume**: Continue playing
- **Options**: Adjust settings
- **Leave Mission**: Return to lobby
- **Quit to Desktop**: Exit game

**Lobby Interface**:
- **Player List**: All connected players
- **Ready System**: Checkboxes for readiness
- **Friend List**: Invite system
- **Start Game**: Host initiates level load
- **Leave Lobby**: Return to main menu

### 12.3 Puzzle Interfaces

**Keypad Interface**:
- 4-digit display
- Number pad (0-9)
- Clear and Enter buttons
- Feedback for correct/incorrect codes

**Wire Puzzle Interface**:
- Visual wire representation
- Click-and-drag connections
- Color-coded wires
- Completion indicator

**Calendar Interface**:
- Month, Day, Year selectors
- Historical date ranges
- Submit button
- Success/failure feedback

**Archive Computer Interface**:
- Search functionality
- Historical records display
- Date information
- Document viewing

### 12.4 UI Visual Style

**Design Language**:
- Clean, minimal interface
- High contrast for visibility
- Sci-fi aesthetic with temporal themes
- Color coding for quick recognition

**Colors**:
- **Primary**: Cyan/blue (interactive elements)
- **Warning**: Yellow/orange (caution)
- **Danger**: Red (alerts, illegal items)
- **Success**: Green (completion, safe status)
- **Neutral**: White/gray (information)

**Typography**:
- Sans-serif fonts for readability
- Different sizes for hierarchy
- High contrast text
- Drop shadows for visibility

---

## 13. Art Direction

### 13.1 Visual Style

**Overall Aesthetic**: Stylized realism with contrasting timelines

**Past Timeline**:
- Warm color palette (browns, golds, sepia tones)
- Classic architecture (art deco, traditional)
- Analog technology (rotary phones, mechanical devices)
- Worn textures, aged materials
- Ornate details, decorative elements

**Future Timeline**:
- Cool color palette (blues, grays, whites)
- Modern architecture (clean lines, glass, metal)
- Digital technology (screens, LEDs, touchpads)
- Clean textures, polished materials
- Minimalist design, functional aesthetics

**Transition Effects**:
- Temporal shimmer when switching
- Particle effects around switching characters
- Material blending for timeline-specific objects
- Post-process color grading shift

### 13.2 Character Art

**Player Character**:
- Professional thief aesthetic
- Tactical clothing (not military)
- Dark colors for stealth (blacks, dark blues/grays)
- Tool belt and equipment visible
- Mask/hood option for anonymity

**Guards**:
- Past: Traditional security uniform, peaked cap
- Future: Modern security attire, body armor
- Distinguishable from civilians
- Authoritative appearance
- Weapon holsters visible

**Civilians**:
- Varied professional attire
- Past: Suits, dresses, period-appropriate
- Future: Business casual, modern fashion
- Diversity in appearance
- Procedural variation in clothing

**Ghost Characters**:
- Semi-transparent shader (30-50% opacity)
- Slightly glowing outline
- Desaturated colors
- Temporal artifact effects (shimmer, distortion)

### 13.3 Environment Art

**Asset Style**:
- **Polygon Heist Pack**: Primary art assets
- **Polygon Spy Pack**: Additional spy-themed props
- Low-poly, stylized models
- Consistent visual language
- Performance-optimized

**Bank Environment**:
- Grand architecture conveying importance
- Vertical spaces with multiple floors
- Contrast between public and secure areas
- Environmental storytelling elements
- Timeline-specific variations

**Lighting**:
- Motivated lighting sources (windows, lamps, monitors)
- Dramatic shadows for stealth gameplay
- Different lighting per timeline (warm vs cool)
- Dynamic lights on security systems (red = active, blue = disabled)

**Materials**:
- PBR materials (Physically Based Rendering)
- Master materials with timeline variants
- Optimized material instances
- Detail textures for close viewing

### 13.4 Effects & Polish

**Visual Effects**:
- Timeline switch effect (full-screen shimmer)
- Ghost character dissolve in/out
- Hacking progress effects (circuit patterns)
- Detection alert pulse (screen edge vignette)
- Laser security beams (volumetric)

**Particle Effects**:
- Temporal distortion particles
- Sparks from hacking
- Dust motes in Past timeline
- Camera lens flare in Future timeline

---

## 14. Audio Design

### 14.1 Sound Design Philosophy

**Goals**:
- Reinforce stealth tension
- Provide clear feedback
- Differentiate timelines through audio
- Support spatial awareness
- Enhance immersion

### 14.2 Ambient Audio

**Past Timeline**:
- Analog sounds (ticking clocks, mechanical)
- Distant traffic (vintage cars)
- Period-appropriate music (classical, jazz)
- Muffled, warmer sound profile
- Paper rustling, footsteps on wood

**Future Timeline**:
- Digital sounds (computer hums, electronic beeps)
- Modern city ambience
- Contemporary background music
- Crisp, clear sound profile
- Footsteps on tile, carpet

**Environmental Audio**:
- Room tone variations by location
- Ventilation hum in corridors
- Crowd murmur in public areas
- Security system alerts
- Mechanical door sounds

### 14.3 Character Audio

**Player Character**:
- **Footsteps**: Vary by surface and movement state
  - Loud when sprinting
  - Quiet when crouching
  - Surface materials affect sound (tile, carpet, metal)
- **Breathing**: Increases when sprinting or detected
- **Interaction Sounds**: Button presses, item pickups
- **Ability Sounds**: Hacking beeps, lockpick scraping

**Guards**:
- Heavy footsteps (boots)
- Radio chatter
- Alert callouts ("Hey, stop!")
- Investigation sounds (flashlight click, weapon handling)

**Civilians**:
- Lighter footsteps
- Conversation murmur
- Keyboard typing
- Phone conversations

### 14.4 UI & Feedback Audio

**Interface Sounds**:
- Menu navigation (hover, click)
- Inventory item selection
- Objective completion (success chime)
- Alert notifications

**Gameplay Feedback**:
- **Detection**: Progressive audio cue (tone pitch increases with detection level)
- **Hacking**: Success/failure tones
- **Lock Picking**: Pin clicks, lock open sound
- **Keypad**: Button beeps, correct/incorrect entry
- **Wire Puzzle**: Connection clicks, circuit completion

**System Sounds**:
- Alarm siren (when fully detected)
- Camera rotation servo
- Laser grid hum
- Metal detector beep
- Door locks/unlocks

### 14.5 Music

**Dynamic Music System**:
- **Exploration**: Ambient, minimal music during safe exploration
- **Tension**: Building music when detection increases
- **Alert**: Intense music during chase/combat
- **Success**: Triumphant music on objective completion

**Timeline Theming**:
- Past: Orchestral, period-appropriate instruments
- Future: Electronic, synthesizer-based
- Transition: Audio crossfade when switching timelines

---

## 15. Technical Architecture

### 15.1 Engine & Tools

**Engine**: Unreal Engine 5.7
- Latest features and optimizations
- Enhanced networking capabilities
- Modern rendering features
- State Tree AI system

**Programming Language**: C++ 17
- Performance-critical gameplay code
- Network replication
- Core systems
- Blueprint interoperability

**IDE**: Visual Studio 2022
- C++ compilation
- Debugging tools
- Source control integration

**Version Control**: Git (GitHub)
- Source code management
- Collaboration
- Issue tracking

### 15.2 Code Architecture

**Design Patterns**:

1. **Component-Based Architecture**:
   - `InventoryComponent`: Inventory management
   - `HackComponent`: Hacking functionality
   - `SearchComponent`: Search functionality
   - `LockPickComponent`: Lock picking
   - Reusable, modular components

2. **Interface-Driven Design**:
   - `IInteractable`: Objects player can interact with
   - `IDetectable`: Objects that can be detected
   - `IRequiresItem`: Objects requiring specific items
   - Flexible, extensible system

3. **Data-Driven Configuration**:
   - DataAssets for abilities, items, settings
   - Configuration files (DefaultEngine.ini, etc.)
   - Gameplay tags (DefaultGameplayTags.ini)
   - Easy balancing and iteration

4. **Event-Driven Communication**:
   - Delegates for UI updates
   - Multicast delegates for state changes
   - Gameplay cues for effects
   - Decoupled systems

**Key Architectural Systems**:

1. **Gameplay Ability System (GAS)**:
   - `DefaultAbilitySystemComponent`: Custom ASC
   - `DefaultGASet`: Ability grant configuration
   - `AbilityInputSet`: Input-to-ability mapping
   - Gameplay abilities for all player actions
   - Network-replicated, extensible

2. **State Tree AI**:
   - Modern AI behavior system
   - Visual scripting in editor
   - Performance optimizations
   - Replaces old Behavior Trees

3. **Network Replication**:
   - Server-authoritative gameplay
   - Client prediction for responsiveness
   - RPCs for player actions
   - Optimized update frequencies

### 15.3 Performance Optimizations

**Tick Optimizations**:
- **Disabled Tick** on actors that don't need per-frame updates:
  - DefaultPlayerController
  - GuardCharacter
  - CivilianCharacter
  - KeypadScanner, CodeGenerator
  - DoorBase, and many others
- **Timer-Based Updates** instead of Tick:
  - Detection: 100ms intervals (10Hz)
  - Interaction highlight: 100ms intervals (10Hz)
  - Code expiration: 500ms intervals (2Hz)
  - Ghost mesh updates: 50ms intervals (20Hz)
- **Tick Intervals** on active components:
  - HackComponent: 50ms (20Hz)
  - SearchComponent: 50ms (20Hz)

**Network Optimizations**:
- **Variable Update Frequencies**:
  - Static actors: 1Hz (NetUpdateFrequency = 1.0f)
  - Civilians: 10Hz (infrequent updates)
  - Items/Moderate: 20Hz
  - Guards: 30Hz (active AI)
  - Players: 60Hz (high responsiveness)
- **Relevancy**: Only replicate actors in player's interest area
- **Dormancy**: Actors become dormant when inactive

**Code Optimizations**:
- **Static TMap** for key-to-slot mapping (O(1) lookup)
- **Cached Gameplay Tags** (static const tags)
- **Throttled Updates** using time accumulators
- **Object Pooling** for frequently spawned actors (future enhancement)

**Memory Management**:
- Unreal's garbage collection
- Smart pointers where appropriate
- Texture streaming for large assets
- LOD (Level of Detail) systems

### 15.4 Project Structure

```
Source/EchoesOfTime/
├── AbilitySystem/           # GAS implementation
│   ├── Abilities/          # Individual gameplay abilities
│   ├── AttributeSets/      # Player attributes
│   ├── GameplayCues/       # Visual/audio effects
│   └── AbilityTasks/       # Async tasks for abilities
├── ActorComponents/        # Reusable components
├── Actors/                 # All actor classes
│   ├── TimeObjects/        # Timeline-specific actors
│   ├── Computers/          # Interactive computers
│   ├── KeypadScanner/      # Keypad puzzle
│   ├── Laser/              # Laser security
│   ├── Lever/              # Lever puzzle
│   └── Wire/               # Wire puzzle
├── Characters/             # Character classes
├── Controllers/            # Player/AI controllers
├── GameModes/              # Game modes
├── GameStates/             # Game states
├── Interfaces/             # C++ interfaces
├── Widgets/                # UI widgets
├── DataAssets/             # Configuration data
└── ProceduralLevelGenerator # Procedural generation
```

### 15.5 Third-Party Plugins

**Advanced Sessions** (Multiplayer):
- Session creation and management
- Steam and LAN support
- Friend invite system
- Server browser

**Advanced Steam Sessions** (Steam Integration):
- Steam P2P networking
- Steam friend list integration
- Steam achievements (future)

**OnlineSubsystemEOS** (Epic Online Services):
- Cross-platform multiplayer
- Epic Games Store integration
- EOS friend system
- Matchmaking (future)

**Gameplay Abilities** (Built-in):
- Gameplay Ability System
- Core Unreal plugin
- Ability and attribute management

**State Tree** (Built-in):
- Modern AI behavior system
- Visual scripting
- Performance optimized

### 15.6 Configuration Files

**DefaultEngine.ini**:
- Network settings
- Rendering configuration
- Physics settings
- Plugin configuration

**DefaultGame.ini**:
- Game mode settings
- Player controller defaults
- Game state configuration

**DefaultInput.ini**:
- Input mappings (legacy)
- Enhanced Input integration

**DefaultGameplayTags.ini**:
- Gameplay tag definitions
- Tag organization
- Tag metadata

---

## 16. Development Roadmap

### 16.1 Current State (Version 1.0)

**Completed Features**:
- ✅ Core timeline switching mechanic
- ✅ Player character with full ability system
- ✅ Stealth and detection system
- ✅ Inventory system (8 slots)
- ✅ Hacking, lockpicking, searching abilities
- ✅ Past Echo ability (ghost character visualization)
- ✅ Keypad, wire, lever, calendar puzzles
- ✅ Laser security system
- ✅ Guard and civilian AI
- ✅ Security camera actors
- ✅ Bank level environment
- ✅ Procedural generation system
- ✅ Multiplayer foundation (Steam, EOS)
- ✅ Lobby system
- ✅ Performance optimizations
- ✅ Basic UI/HUD
- ✅ Comprehensive documentation

### 16.2 Short-Term Goals (3-6 months)

**Polish & Enhancement**:
1. **Audio Implementation**:
   - Complete sound design
   - Dynamic music system
   - Spatial audio for detection awareness

2. **Visual Effects**:
   - Enhanced timeline transition effects
   - Improved detection feedback
   - Particle systems for abilities
   - Post-processing enhancements

3. **UI Improvements**:
   - Minimap system
   - Waypoint markers for multiplayer
   - Improved objective tracking
   - Tutorial system

4. **Gameplay Refinement**:
   - Balance detection parameters
   - Polish ability mini-games
   - Improve AI behavior
   - Additional puzzle variations

5. **Content Expansion**:
   - More items and tools
   - Additional guard patrol routes
   - More documents and lore
   - Environmental storytelling

### 16.3 Mid-Term Goals (6-12 months)

**Major Features**:
1. **Additional Heist Locations**:
   - Museum level
   - Corporate office
   - Art gallery
   - Research facility

2. **Campaign Mode**:
   - Story-driven mission sequence
   - Character development
   - Escalating difficulty
   - Cutscenes and narrative

3. **Progression System**:
   - Player skill tree
   - Equipment unlocks
   - Cosmetic customization
   - Player profiles

4. **Challenge Modes**:
   - Speed run mode
   - Ghost mode (no detection)
   - Tool-restricted mode
   - Leaderboards

5. **Enhanced AI**:
   - More sophisticated guard behavior
   - Civilian interactions
   - Drone AI implementation
   - Alert states and backup systems

6. **Advanced Multiplayer**:
   - Matchmaking system
   - Voice chat integration
   - Player roles (classes)
   - Competitive modes

### 16.4 Long-Term Goals (12+ months)

**Platform Expansion**:
- Console ports (PlayStation 5, Xbox Series X)
- Controller support enhancements
- Platform-specific features
- Cross-platform play

**Content**:
- 10+ heist locations
- Multiple campaigns
- Community-created content support
- Level editor (future consideration)

**Systems**:
- Dynamic difficulty adjustment
- Procedural level generation (full levels)
- Advanced customization
- In-game economy (cosmetics)

**Live Service** (if applicable):
- Seasonal content
- Regular updates
- Community events
- DLC missions

### 16.5 Technical Debt & Maintenance

**Ongoing Tasks**:
- Code refactoring for maintainability
- Performance profiling and optimization
- Bug fixing and QA
- Documentation updates
- Compatibility with new Unreal Engine versions

**Known Issues to Address**:
- Network latency compensation improvements
- Rare detection edge cases
- AI pathfinding in complex geometry
- Memory optimization for large levels

---

## Conclusion

**Echoes of Time** represents an ambitious stealth heist game that combines innovative time-manipulation mechanics with cooperative multiplayer gameplay. The dual timeline system provides a unique twist on traditional stealth gameplay, while the emphasis on puzzle-solving and teamwork creates engaging and replayable experiences.

The game's technical foundation, built on Unreal Engine 5.7 with the Gameplay Ability System, provides a robust and extensible platform for future development. The comprehensive codebase, with over 105 C++ classes and extensive network optimization, demonstrates a professional approach to game development.

With procedural generation ensuring variety, a sophisticated AI system providing challenging opposition, and a rich set of puzzles testing player ingenuity, Echoes of Time offers a compelling experience for fans of stealth and heist games. The multiplayer focus encourages cooperation and communication, creating memorable moments when plans come together—or fall apart.

The roadmap outlines a clear path for continued development, with opportunities for content expansion, system refinement, and community engagement. Whether playing solo or with friends, Echoes of Time invites players to master the art of temporal infiltration and pull off the perfect heist.

---

**Document Version History**:
- Version 1.0 (December 10, 2025): Initial comprehensive GDD creation

**Contributors**:
- Primary Author: AI Design Assistant
- Based on: Echoes of Time codebase and documentation
- Repository: github.com/Traveler3114/Echoes-of-Time

---

*"Master time, master the heist."*
