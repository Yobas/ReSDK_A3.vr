# NetDisplays

Core files:

- `client/NetDisplays/NetDisplays.sqf`
- `client/NetDisplays/Displays/BaseDisplay.sqf`
- `client/NetDisplays/Displays/*.sqf`

## Mental Model

`NetDisplays` are client UI shells backed by server-owned state. The client renders the current mode and sends input back to the server. Do not treat them as client-authoritative gameplay systems.

## Base Structure

`NDBase` provides:

- `thisDisplay`: assigned display reference
- `lastNDWidget`: last widget created by `addWidget`
- `addSavedWidget`: stores stable shell widgets
- `getSavedWidgets`: retrieves saved shell widgets
- `addWidget`: creates a widget, puts it into the transient list, and tags its `data`

Each ND struct implements:

- `process(_args, _isFirstCall)`

## First Call vs Refresh

The normal pattern is:

1. on first call, create the root shell, background, close button, and main content group
2. save those stable widgets with `addSavedWidget`
3. on every refresh, call `nd_cleanupData`
4. rebuild only the dynamic content region with `addWidget`

This keeps the shell stable while server updates can redraw lists, buttons, and text cheaply.

## Key Helpers

- `nd_cleanupData`: deletes transient widgets from the refresh list
- `nd_regWidget`: convenience wrapper around `addWidget`
- `nd_addClosingButton`: close button helper that routes through ND close logic
- `nd_stdLoad`: common shell constructor for many simple displays
- `nd_onPressButton`: sends client input back to the server

## Display Lifecycle

- `nd_loadDisplay`: opens normal in-game ND screens on a dynamic display
- `nd_loadDisplay_lobby`: opens ND screens inside the lobby display
- `nd_onClose`: server-notified close path
- `nd_unloadDisplay`: local teardown and state reset

`nd_loadDisplay` already coordinates with inventory, chat history, and other screen conflicts. Respect that flow instead of bypassing it.

## Useful Patterns From Real Screens

- `Paper.sqf`: toggle between read and write mode, preserve buffered text across refresh, use separate saved shell and rebuilt content
- `HeadConsole.sqf`: menu mode switching, lists, buttons, per-control stored references, multi-line input, and server button events
- `MerchantConsole.sqf`: list rebuilds and dynamic text height adjustments
- `RoundStartScreen.sqf`: full-screen ND with mixed static and scroll content

## ND Authoring Rules

- Keep the shell minimal and stable.
- Put dynamic rows, list entries, and changing text into the transient rebuild area.
- Store local UI-only state on the saved control group when needed.
- Send compact semantic input back to the server through `nd_onPressButton`.
- Reuse existing ND helper conventions before inventing a second screen framework inside a display struct.
