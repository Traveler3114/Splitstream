# Splitstream

**Splitstream** is a cooperative multiplayer heist game built with **Unreal Engine 5.7**. Players work together across two parallel timelines — **Past** and **Future** — to infiltrate high-security locations, steal valuables, and escape before the alarm triggers a lockdown.

The game blends stealth mechanics, puzzle-solving, and time-split teamwork into a tense co-op experience powered by dedicated multiplayer networking and Epic's Gameplay Ability System (GAS).

---

## Key Features

- **Dual-Timeline Gameplay** — The world exists in two eras (Past and Future). Actions in one timeline can affect the other, requiring coordination between teams.
- **Cooperative Multiplayer** — Listen-server based multiplayer with lobby management, team assignment, session handling via Steam/EOS, and seamless travel.
- **Stealth & Detection** — AI guards and security drones patrol the environment. Players must avoid detection or face escalating alarm states.
- **Gameplay Ability System (GAS)** — Character abilities, status effects, and attribute-driven stats (health, walk/run/crouch speed) are managed through Unreal's GAS framework.
- **Puzzle Systems** — Wire puzzles, lever puzzles, keypad codes, lock picking, hacking mini-games, and laser grids gate progress throughout levels.
- **Inventory & Items** — A full inventory system with equippable items, keycards, gadgets, weapons, and data-driven item definitions.
- **Procedural Elements** — Procedural level generation support and randomized NPC schedules via calendar/scheduling systems.
- **Ghost Mirroring** — Guards visible in one timeline produce "ghost" representations in the other, giving players situational awareness across eras.

---

## Project Structure

```
Splitstream/
├── Config/                  # Engine, input, gameplay tag configuration
├── Content/                 # Unreal assets (maps, blueprints, materials, etc.)
├── Plugins/                 # Third-party plugins (AdvancedSessions, AdvancedSteamSessions)
├── Source/
│   ├── Splitstream/         # Primary game module (all C++ gameplay code)
│   ├── Splitstream.Target.cs
│   └── SplitstreamEditor.Target.cs
├── docs-*.md                # Detailed project and codebase documentation
└── Splitstream.uproject     # Project descriptor (UE 5.7)
```

---

## Requirements

- **Unreal Engine 5.7**
- **Visual Studio 2022** (or Rider) with UE C++ workflow
- **Steamworks SDK** (for Steam multiplayer, optional)
- **Epic Online Services** (EOS plugin enabled in .uproject)

---

## Getting Started

1. Clone the repository.
2. Open `Splitstream.uproject` with Unreal Engine 5.7.
3. Generate project files (right-click `.uproject` → "Generate Visual Studio project files").
4. Build and run from the editor or IDE.

---

## Documentation

Detailed documentation is available in the project root:

| Document | Description |
|---|---|
| [Architecture Overview](docs-architecture.md) | High-level system architecture and module layout |
| [Gameplay Systems](docs-gameplay-systems.md) | Detection, alarms, timelines, puzzles, and stealth |
| [Ability System (GAS)](docs-ability-system.md) | GAS integration, attributes, abilities, and tags |
| [Networking & Multiplayer](docs-networking.md) | Replication, session management, and travel flow |
| [Actors & Components](docs-actors-and-components.md) | Reference for all actors and actor components |
| [UI & Widgets](docs-ui-widgets.md) | HUD, menus, mini-game widgets, and settings |
| [Code Review](docs-code-review.md) | Full codebase review, strengths, improvements, and grade |

---

## License

This project is proprietary. All rights reserved.
