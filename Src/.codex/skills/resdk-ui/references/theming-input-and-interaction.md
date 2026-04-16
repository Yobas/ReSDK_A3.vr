# Theming, Input, And Interaction

## Themes

Theme system entry point:

- `client/ColorThemes/ColorThemes_init.sqf`

Important points:

- theme values are read through `ct_getValue`
- persistent modules often expose `*_applyColorTheme`
- `ct_applyTheme` fans out into module-specific refresh functions such as chat, inventory, stamina, cursor, and progress UI

If a persistent UI already participates in theme refresh, keep new colors behind the same mechanism instead of introducing unrelated one-off constants.

## Text Rules

- `widgetSetText` is for structured text and HTML-like formatting
- `ctrlSetText` is for plain text on buttons and edit controls
- when sending user-authored text to the server, sanitize it first if the module already does so

Examples:

- `chatprint` validates and sanitizes styled text before showing it
- `Paper` sanitizes multi-line player text before sending ND input

## Input Widget Choices

- `INPUT`: simple single-line input
- `INPUTCHAT`: chat-focused single-line input used by lobby chat
- `INPUTMULTI`: multi-line input when the module handles key behavior itself
- `INPUTMULTIV2`: multi-line input with built-in Enter newline behavior from `createWidget`

If you add multi-line input to an older screen, either:

- use `INPUTMULTIV2`, or
- call `widget_registerInput` / add equivalent handler logic intentionally

## Event Handler Patterns

Use display handlers for window-level behavior:

- escape and key swallowing
- history navigation
- screen-level shortcuts

Use control handlers for local behavior:

- button clicks
- hover colors
- list selection
- per-input key handling

Common project patterns:

- `displayAddEventHandler ["KeyDown", ...]` for global screen shortcuts
- `ctrlAddEventHandler ["MouseButtonUp", ...]` for click actions
- `ctrlAddEventHandler ["KeyUp", ...]` for edit-box behavior

## Hover And Focus

- `widgetSetMouseMoveColors` is the simplest reusable hover helper
- some modules wire `MouseEnter` and `MouseExit` manually for custom visuals
- use `ctrlSetFocus` when a screen should immediately capture an input widget

Examples:

- `client/Interactions/interactEmoteMenu.sqf` uses `widgetSetMouseMoveColors`
- `client/ClientData/SendCommand.sqf` focuses the command input on open
- several ND screens attach per-button hover background handlers manually

## Spam Protection

For buttons that can trigger server calls or expensive actions repeatedly, follow local patterns such as `input_spamProtect` instead of trusting raw click rate.

This is especially common in:

- `NetDisplays`
- lobby system actions
- chat-adjacent command-style actions

## Scroll And Dynamic Text

When text grows inside a scrollable group:

1. set the structured text
2. measure with `widgetGetTextHeight`
3. resize the text control
4. call `widgetWGScrolldown` if the UX wants the newest content visible

This exact pattern appears in chat, lobby chat, message histories, and multiple ND screens.
