# Validation Matrix

## Editor Code In `Editor/`

Recommended path:

- open ReEditor
- exercise the affected component or menu path
- watch editor logs via `printLog`/`printWarning`/`printError`
- run simulation only if the change affects runtime behavior

## Map Or Content Changes

Recommended path:

- save workspace if needed
- save map to storage if source changed
- build map if runtime output matters
- launch simulation for the relevant map and mode

Especially relevant for:

- object placement
- inspector properties
- map scripts or bindings

## Craft And Loot Configs

Recommended path:

- validate YAML shape against the docs
- use ReEditor validator or checker flows when available
- run a targeted simulation smoke test for the actual recipe or loot source

## Host Gameplay Or OOP Code

Recommended path:

- ensure the owning modules load cleanly
- use a focused simulation scenario that exercises the changed interaction
- inspect RPT or project logs if behavior is wrong

## Client UI Or Rendering

Recommended path:

- launch simulation
- open the affected menu, display, or HUD flow
- use client-facing debug tools such as `debugvars` if helpful

## ReNode Or VisualScripting

Recommended path:

- compile or regenerate the affected graph/library
- if editor integration matters, launch ReNode through ReEditor
- if debugger support is unnecessary, manual ReNode launch is acceptable

## Generated Asset Workflows

Regenerate first, then verify output paths:

- class library: `goasm_builder_rebuildClasses`
- map build: `mm_build`
- model map: `systools_GenerateModelData`
- ReNode object library: `vs_generateLib`

Generated outputs commonly involved:

- `../../../host/MapManager/Maps`
- `../../../M2C.sqf`
- `../../../host/ReNode/compiled`

## When Confidence Is Limited

Say exactly which of these is true:

- not executed
- blocked by missing environment
- only doc-level validation performed
- simulation smoke test performed
- validator tool performed
- legacy unit-test harness performed
