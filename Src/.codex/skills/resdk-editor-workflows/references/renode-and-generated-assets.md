# ReNode And Generated Assets

## ReNode Launch Modes

There are two supported working modes:

1. Launch from ReEditor:
   - use when debugger integration matters
   - menu path is exposed by the editor and wired to `vs_openEditor`
2. Launch `ReNode\ReNode.exe` manually:
   - use when Arma 3 or ReEditor is unavailable
   - debugger integration is not available

## ReNode In Project Code

- Editor-side entry helpers live in `../../../Editor/VisualScripting/VisualScripting_init.sqf`
- Host-side binding and graph infrastructure lives in `../../../host/ReNode/`
- Local ReNode module authoring notes live in `../../../host/ReNode/README.md`

## Node Library Workflow

- `vs_generateLib` triggers generation of the object or node library used by the editor
- For custom node modules, register the module in `ReNode_init.sqf` and use the binding helpers described in `../../../host/ReNode/README.md`

## Generated Asset Boundaries

Treat these as generated outputs unless explicitly asked to edit them directly:

- `../../../host/ReNode/compiled/*`
- `../../../M2C.sqf`
- `../../../host/MapManager/Maps/*`

Operational rule:

- modify the source workflow first
- regenerate through the owning tool second
- only patch generated files directly when the task is specifically about that generated artifact

## Related Documentation

- ReNode editor guide: `../../../../Documentation/ReNode/Basics.md`
- ReNode overview: `../../../../Documentation/ReNode/README.md`
- Local compiled-graphs warning: `../../../host/ReNode/compiled/README.md`
