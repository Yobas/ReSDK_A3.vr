# UI Lifecycle And State

## Main UI Modes

### Persistent GUI widgets

Use `getGUI` when the widget should exist as part of the always-on client UI:

- chat root in `client/Chat/chat_init.sqf`
- HUD widgets in `client/Hud/Hud_init.sqf`
- hand slots in `client/Inventory/functions.sqf`

Pattern:

- create once during module init
- store references in module arrays or maps
- update text, position, fade, and colors over time
- avoid reopening a display just to refresh the widget

### Standalone modal windows

Use `displayOpen` for modal windows that own their own ReSDK display and do not need the game world to remain interactable.

Typical examples:

- send-command window
- escape menu

### Dynamic overlays

Use `dynamicDisplayOpen` for overlays that should suppress movement and escape-driven closure.

Typical examples:

- inventory
- chat history
- most `NetDisplays`

`dynamicDisplayOpen` installs key handlers that swallow escape, forbidden buttons, and movement-related input, so do not reimplement that behavior unless the screen truly needs custom exceptions.

### Lobby-context UI

Some client UI must reuse the already-open lobby display instead of opening a new one.

Examples:

- `client/Lobby/functions.sqf`
- lobby-context escape menu
- lobby-context send-command window
- lobby-context `NetDisplays`

If the screen is opened from lobby, prefer `getDisplay` and coordinate with `lobby_sysSetEnable` rather than blindly calling `displayOpen`.

## Where To Store Widget References

Common project patterns:

- module arrays like `chat_widgets`, `craft_widgets`, `esc_widgets`
- hash maps for named HUD elements
- `setVariable` links between parent and child widgets
- control-group variables for callbacks and subordinate controls

Use the storage style that the module already uses. Do not add a second competing widget registry inside the same feature.

## Shell vs Dynamic Region

A recurring project pattern is:

1. create a stable shell once
2. keep references to that shell
3. recreate only the refreshable content area

Good examples:

- chat root is stable, text content is recalculated
- `NetDisplays` keep saved shell widgets and rebuild transient data widgets
- inventory keeps stable hand slots but opens and closes container overlays

## Cleanup Rules

- `displayClose` closes the whole current ReSDK display.
- `deleteWidget` removes a single widget or control group.
- Use `nextFrame(...)` or `nextFrameParams(...)` when closing a display or deleting a parent from within one of its own event handlers.
- Temporary subdialogs can fade or disable the previous group instead of destroying it. `EscapeMenu` uses this pattern for its confirmation step.

## State Shape Guidance

Normal local UI state includes:

- widget references
- currently selected row, category, or tab
- cached text in an input field
- fade timers and animation positions
- lobby/ND mode flags

That is different from gameplay authority. Keep gameplay data on the server and only mirror what the client must present or submit back as input.
