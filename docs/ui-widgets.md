# UI & Widgets

This document covers the UI/Widget architecture in **Splitstream**.

---

## 1. HUD System

### `ACharacterHUD` (`Widgets/HUD/CharacterHUD.h`)
- Custom `AHUD` subclass used as the main in-game HUD.
- Creates and manages the `UCharacterOverlay` widget.
- `AddCharacterOverlay()` — Adds the overlay widget to the viewport.

### `UCharacterOverlay` (`Widgets/HUD/CharacterOverlay.h`)
The primary in-game overlay widget displaying:
- **Health text** — Updated via `SetHealthText(float)`.
- **Ping text** — Updated every second via `SetPingText(float)`.
- **Objective text** — Money collected vs. target via `SetObjectiveText(int32 Current, int32 Target)`.
- **Status text** — Alarm/pre-alarm/illegal status with color via `SetStatusTextWithColor(FString, FLinearColor)`.
- **Detection widgets** — Per-detector indicators via `UpdateDetectionWidget(AActor*, float Progress, bool bIsLocked, FVector2D ScreenPos, bool bIsOnScreen)`.
- **Repair countdowns** — `UpdateRepairCountdowns(TMap<AActor*, float>)`.

### Detection Widget Positioning

`ADefaultPlayerController::ClientUpdateDetectionWidget_Implementation()` handles screen-space positioning of detection indicators:
- Projects the detector's world location to screen coordinates.
- For off-screen/behind-camera targets, calculates edge-clamped screen positions using camera-space direction mapping.
- Accounts for character capsule height for accurate overhead positioning.

---

## 2. Pause Menu

### `UPauseMenuWidget` (`Widgets/PauseMenuWidget.h`)
- Toggled by Escape key via `ADefaultPlayerController::TogglePauseMenu()`.
- **Delegate:** `OnPauseMenuResumed` — fired when the player resumes.
- Manages input mode switching between `GameAndUI` (paused) and `GameOnly` (resumed).
- Includes options to leave to main menu via `RequestLeaveToMainMenu()`.

---

## 3. Lobby UI

### `ULobbyUI` (`Widgets/Lobby/LobbyUI.h`)
Main lobby widget created by `ALobbyPlayerController::BeginPlay()`.
- Displays map name from session settings.
- Start button visibility based on host/client role.
- Start button enabled/disabled based on all-players-ready state.

### `UPlayerLobbyInfo` (`Widgets/Lobby/PlayerLobbyInfo.h`)
Per-player lobby info widget displayed on `ALobbyPlatformActor`.
- Shows player name, avatar, ready state, and team.
- Host can kick players via this widget.

### `UFriendList` (`Widgets/Lobby/FriendList.h`)
Steam friends list widget for inviting players.

### `UFriendWidget` (`Widgets/Lobby/FriendWidget.h`)
Individual friend entry in the friends list.

### `UOpenFriendsListButton` (`Widgets/Lobby/OpenFriendsListButton.h`)
Button to toggle the friends list visibility on unoccupied platforms.

---

## 4. Main Menu

### `UMainMenuWidget` (`Widgets/MainMenu/MainMenuWidget.h`)
Root main menu widget created by `AMainMenuPlayerController::BeginPlay()`.

### `UMapSelectionWidget` (`Widgets/MainMenu/MapSelectionWidget.h`)
Map selection interface for hosting games.

### `UMapWidget` (`Widgets/MainMenu/MapWidget.h`)
Individual map entry in the selection list.

---

## 5. Settings

### `USettingsWidget` (`Widgets/Settings/SettingsWidget.h`)
Root settings widget containing graphics and input tabs.

### `UGraphicsWidget` (`Widgets/Settings/GraphicsWidget.h`)
Graphics quality and resolution settings.

### `UInputWidget` (`Widgets/Settings/InputWidget.h`)
Input/keybinding settings. Works with `UDefaultGameInstance` to save/load keybinds.

### `UKeybindWidget` (`Widgets/Settings/KeybindWidget.h`)
Individual keybind entry for remapping.

### `USliderWidget` (`Widgets/Settings/SliderWidget.h`)
Reusable slider control (e.g., mouse sensitivity).

### `UEnumOptionWidget` (`Widgets/Settings/EnumOptionWidget.h`)
Dropdown/cycle widget for enum settings (e.g., quality presets).

---

## 6. Calendar System

### `UCalendarWidget` (`Widgets/Calendar/CalendarWidget.h`)
Calendar popup showing NPC schedules and dates.
- Populated via `ADefaultPlayerController::ClientShowCalendarWidget()` RPC.
- Contains `CalendarDateRecords` for civilian schedule display.

### `UCalendarButtonWidget` (`Widgets/Calendar/CalendarButtonWidget.h`)
Clickable date button in the calendar.

### `UCalendarResultWidget` (`Widgets/Calendar/CalendarResultWidget.h`)
Result display after selecting a calendar date.

---

## 7. Mini-Game Widgets

### `UFirewallWidget` (`Widgets/Minigames/FirewallWidget.h`)
Rendering surface for the Firewall mini-game (space invader style).
- Receives game state from `UFirewallMiniGame` and renders player, enemies, projectiles, and boss.
- Custom paint via canvas-based rendering.

### `UNeonRunnerWidget` (`Widgets/Minigames/NeonRunnerWidget.h`)
Rendering surface for the Neon Runner mini-game (endless runner style).
- Displays player, obstacles, tiles, and hover meter.

---

## 8. Interaction Widgets

### `UHackWidget` (`Widgets/HUD/HackWidget.h`)
Hacking progress UI shown during `UDefaultGAHack` ability.

### `ULockPickWidget` (`Widgets/HUD/LockPickWidget.h`)
Lock picking UI shown during `UDefaultGALockPick` ability. Displays pins, sweet spots, and player input angle.

### `USearchWidget` (`Widgets/HUD/SearchWidget.h`)
Search progress UI shown during `UDefaultGASearch` ability.

### `UDetectionWidget` (`Widgets/DetectionWidget.h`)
Individual detection indicator displayed per-detector on the HUD overlay.

### `UProximityHackWidget` (`Widgets/ProximityHackWidget.h`)
Proximity hack progress display attached to `UProximityHackComponent` via `UWidgetComponent`.

### `USecurityDocumentWidget` (`Widgets/SecurityDocumentWidget.h`)
Security document display for viewing guard information.

---

## 9. Widget Creation Patterns

### Controller-Created Widgets
Most widgets are created by their corresponding player controller:
```cpp
// Example: LobbyPlayerController
if (LobbyUIClass)
{
    LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
    LobbyUIInstance->AddToViewport();
}
```

### Ability-Created Widgets
GAS abilities create their widgets during `ActivateAbility()` and destroy them in `EndAbility()`:
- `UDefaultGAHack` → `UHackWidget`
- `UDefaultGALockPick` → `ULockPickWidget`
- `UDefaultGASearch` → `USearchWidget`

### Mini-Game Widgets
Mini-games manage their own input contexts and widget lifecycle:
1. `StartGame()` → creates widget, swaps input mapping context, starts tick timer.
2. `EndGame()` → removes widget, restores input context, clears timer.
