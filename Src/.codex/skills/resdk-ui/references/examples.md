# Examples

## Chat

Files:

- `client/Chat/chat_init.sqf`
- `client/Chat/functions.sqf`

Patterns:

- persistent root widget on `getGUI`
- module array for widget references
- structured text buffer joined with `<br/>`
- text height recalculated after every update
- history window opened on `dynamicDisplayOpen`
- lobby chat rendering reuses the same content through `chat_onRenderLobby`

## Lobby

Files:

- `client/Lobby/lobbi_init.sqf`
- `client/Lobby/functions.sqf`
- `client/Lobby/SystemSettings.sqf`

Patterns:

- one display hosting many client subsystems
- chat, ready button, settings, and character controls as separate control-group regions
- module arrays and named widget variables for storage
- temporary disabling/fading of other lobby regions while modal sub-UI is active
- lobby system actions that coexist with escape menu, send-command window, and ND screens

## Inventory

Files:

- `client/Inventory/functions.sqf`
- `client/Inventory/container.sqf`

Patterns:

- persistent hand slots on `getGUI`
- square slot widgets via `createWidget_square`
- nested control-group structure with background, icon, overlay, and text
- per-slot references stored on the root widget via `setVariable`
- fade-based visibility and movement animation
- drag/drop state handled through widget references instead of ad hoc globals per control

## Escape Menu And Send Command

Files:

- `client/ClientData/EscapeMenu.sqf`
- `client/ClientData/SendCommand.sqf`

Patterns:

- modal windows centered in a root control group
- plain button labels through `ctrlSetText`
- structured body text through `widgetSetText`
- temporary confirmation group layered above an existing group instead of recreating the whole menu
- display-level key handling for command history and Enter submission
- lobby-aware mode that reuses `getDisplay`

## Interactions

Files:

- `client/Interactions/interactMenu.sqf`
- `client/Interactions/interactEmoteMenu.sqf`

Patterns:

- screen built from repeated local helper macros
- hover colors through `widgetSetMouseMoveColors`
- animated panel show and hide via `widgetSetPosition` and `widgetSetPositionOnly`
- control groups used as movable panels instead of rebuilding the whole UI repeatedly

## NetDisplays

Files:

- `client/NetDisplays/NetDisplays.sqf`
- `client/NetDisplays/Displays/BaseDisplay.sqf`
- `client/NetDisplays/Displays/Paper.sqf`
- `client/NetDisplays/Displays/HeadConsole.sqf`

Patterns:

- `process(_args, _isFirstCall)` split
- stable saved shell plus transient rebuilt content
- widget references stored on root groups for mode toggles and local input caches
- UI sends semantic input to the server instead of mutating gameplay state locally
- lobby and non-lobby open paths share the same display-authoring model
