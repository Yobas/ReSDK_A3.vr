# Editor Basics

## Core Entry Points

- Editor startup lives in `../../../Editor/Editor_init.sqf`
- Component load order is declared through `componentInit(...)` plus adjacent `#include`
- Common editor menu actions are wired in `../../../Editor/Widgets/Widget_init.sqf`

## Map Lifecycle

The editor map workflow has three different states. Do not conflate them.

1. Workspace:
   - Temporary working state
   - Saved to `mission.sqm` in the project root
   - Used to preserve your current local editing session
2. Map storage:
   - Editor-owned source maps
   - Stored in `../../../Editor/Bin/Maps/*.cpp`
   - Shared source representation other SDK users can open in ReEditor
3. Compiled maps:
   - Runtime output for simulation and server loading
   - Stored in `../../../host/MapManager/Maps/*.sqf`
   - Produced by map build

## Practical Consequences

- `Ctrl+S` saves workspace, not the shared source map and not the runtime map.
- Saving the map to storage updates `Editor/Bin/Maps`.
- Building the map updates `host/MapManager/Maps`.
- If the user wants other developers or runtime code to see map changes, workspace save alone is not enough.

## High-Signal Editor Areas

- Left panel: inspector or layer/object list
- Right panel: GameObjectsLibrary or history
- Scene window: actual placement and transform editing

## Common Menu Actions

These are the editor-facing workflows most likely to matter while helping:

- Recompile class library: `goasm_builder_rebuildClasses`
- Build current map: `mm_build`
- Generate model map: `systools_GenerateModelData`
- Create ReNode object library: `vs_generateLib`
- Open ReNode editor: `vs_openEditor`
- Open ReEditor settings: `core_settings_openWinow`

## Useful Paths

- ReEditor user settings: `../../../Editor/EditorSettings.txt`
- Map storage: `../../../Editor/Bin/Maps`
- Built maps: `../../../host/MapManager/Maps`
- Editor docs: `../../../../Documentation/EditorGuides/README.md`
- ReEditor basics: `../../../../Documentation/EditorGuides/Editor_basics.md`
