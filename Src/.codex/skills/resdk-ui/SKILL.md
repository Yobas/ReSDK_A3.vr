---
name: resdk-ui
description: Build and maintain ReSDK client UI with `WidgetSystem` and `NetDisplays`. Use when creating or editing client-side windows, overlays, HUD elements, chat, lobby screens, inventory widgets, themed controls, or ND displays; when choosing between `displayOpen`, `dynamicDisplayOpen`, or reusing an existing display; or when wiring input, scrolling, text sizing, and widget lifecycle.
---

# ReSDK UI

This skill is the project guide for client UI built on top of `client/WidgetSystem`. Pair it with `resdk-dsl` for DSL semantics and with `resdk-architecture` when placement or ownership is unclear.

## Entry Workflow

1. Classify the UI task:
   - persistent HUD or always-on widget on `getGUI`
   - standalone modal window
   - overlay display that should suppress game inputs
   - lobby-context UI that must reuse the current display
   - server-driven `NetDisplay`
2. Open the narrowest matching reference:
   - `references/widget-system-basics.md`
   - `references/ui-lifecycle-and-state.md`
   - `references/theming-input-and-interaction.md`
   - `references/netdisplays.md`
   - `references/examples.md`
3. If the task is really about subsystem placement or gameplay ownership, route into `resdk-architecture` before changing code.

## Mandatory Rules

- Treat `WidgetSystem` positions as percentages. Inside a control group they are relative to that parent group, not to the whole screen.
- Use `widgetSetText` for structured text widgets. Use `ctrlSetText` only when the control expects plain text such as button labels or edit-box contents.
- When text height is dynamic, recompute with `widgetGetTextHeight`, resize the widget, and scroll the parent group if needed.
- Pick the right display context:
  - `getGUI` for persistent always-on UI
  - `displayOpen` for standalone dialogs
  - `dynamicDisplayOpen` for overlays that should trap escape, forbidden keys, and movement inputs
  - `getDisplay` when UI lives inside an already-open context such as lobby
- Keep stable shell widgets and refreshable data widgets separate. Rebuild only the dynamic part when the screen updates.
- Theme-aware persistent UI should use `ct_getValue` or module-level theme helpers instead of scattering unrelated hardcoded colors.
- UI stays on the client as presentation and input glue. Gameplay state remains server-authoritative.

## Routing

- Widget types, coordinate system, templates, text, and scroll helpers:
  see `references/widget-system-basics.md`
- Choosing display mode, storing widget references, cleanup, and modal layering:
  see `references/ui-lifecycle-and-state.md`
- Input widgets, hover behavior, display handlers, themes, and text sanitization:
  see `references/theming-input-and-interaction.md`
- `NDBase`, saved-vs-transient widgets, update flow, and server round-trips:
  see `references/netdisplays.md`
- Concrete module patterns from chat, lobby, inventory, escape menu, send-command, interactions, and ND screens:
  see `references/examples.md`

## Working Style

- State which display context the new UI belongs to before editing.
- Reuse the module's existing widget arrays, maps, or control-group variables instead of inventing a new storage style beside them.
- Prefer small module-local helpers or macros when one screen repeats the same widget setup many times.
- Close displays and delete widgets deliberately. When an event handler is mid-flight, prefer `nextFrame(...)` or `nextFrameParams(...)` around teardown.
