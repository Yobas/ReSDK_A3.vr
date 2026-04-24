# Generated And Legacy

## Generated / System-Owned Artifacts

Treat these as system-owned unless the user explicitly asks to modify them directly:

- `../../../M2C.sqf`
- `../../../host/MapManager/Maps/*.sqf`
- `../../../host/ReNode/compiled/*`

Notes:

- these files may still be called like ordinary code
- they matter for reading, tracing, and debugging behavior
- they are not the right default style reference for hand-written module code
- do not "clean up" or normalize them casually
- prefer finding the source generator or authoritative hand-written inputs when possible

`host/ReNode/compiled/README.md` explicitly warns not to modify that folder without necessity.

## Legacy Compatibility

`../../../host/lang.hpp` is a legacy compatibility layer.

Use it as context for reading older files, but:

- do not introduce it in new files
- do not treat it as the canonical DSL entrypoint
- do not infer that legacy annotations there define current best practice

## Practical Rule

If a task touches one of these files:

1. state that the file looks generated/system-owned or legacy-scoped
2. avoid using it as the style template for new hand-written code
3. inspect surrounding docs and real hand-written modules before proposing broad edits
