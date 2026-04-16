# WidgetSystem Basics

Core files:

- `client/WidgetSystem/widget_init.sqf`
- `client/WidgetSystem/functions.sqf`
- `client/WidgetSystem/widgets.hpp`
- `client/WidgetSystem/defines.sqf`

## Core API

- `getGUI`: the persistent GUI layer display from `uiNamespace`
- `getDisplay`: the currently opened ReSDK display
- `isDisplayOpen`: whether the ReSDK modal display already exists
- `displayOpen`: opens a standalone display dialog
- `dynamicDisplayOpen`: opens a display on top of display `46` and traps more inputs
- `displayClose`: closes the current ReSDK display

## Widget Types

Important widget type macros from `widgets.hpp`:

- `TEXT`: `RscStructuredText`
- `PICTURE`, `ACTIVEPICTURE`
- `BUTTON`, `BUTTONMENU`
- `INPUT`: single-line edit
- `INPUTMULTI`: multi-line edit
- `INPUTMULTIV2`: multi-line edit with built-in Enter handling
- `INPUTCHAT`: chat-style edit box
- `WIDGETGROUP`: no scrollbars
- `WIDGETGROUPSCROLLS`: scrollable group
- `WIDGETGROUP_H`: no horizontal scrollbar, useful for vertical text/content growth
- `BACKGROUND`: structured text widget created in non-interactive background mode

## Position Model

- Widget positions are percentages in `[x, y, w, h]`.
- At top level, percentages are converted against `safezone`.
- Inside a parent control group, percentages are converted against the parent group's width and height.
- `widgetSetPosition` updates position and size.
- `widgetSetPositionOnly` is for moving a widget or updating a subset without redefining everything.
- `widgetGetPosition` returns percent-space coordinates.

## Common Helpers

- `widgetSetText`: applies structured text via `parseText`
- `widgetSetPicture`: sets picture path
- `widgetGetTextHeight`: returns text height in percent space, already normalized for top-level or parent-group context
- `widgetWGScrolldown`: auto-scrolls a `WIDGETGROUP_H` to the bottom
- `widgetSetMouseMoveColors`: attaches simple hover-in and hover-out background colors
- `mouseSetPosition`, `mouseGetPosition`, `isMouseInsideWidget`, `getMousePositionInWidget`: pointer helpers

## Templates

From `defines.sqf`:

- `createWidget_closeButton`: ready-made close button that calls `displayClose` on click
- `createWidget_window`: creates a window control group plus background and a title strip group
- `createWidget_square`: keeps width aspect-correct from height, used heavily by inventory slots and icon-like controls

## Practical Layout Rules

- Use a root control group for each screen or widget cluster.
- Put stable background widgets at the bottom of the group, then content widgets above them.
- For structured text in scroll groups:
  - create a text control inside the group
  - set text with `widgetSetText`
  - read height with `widgetGetTextHeight`
  - resize the text widget
  - scroll the parent group when needed
