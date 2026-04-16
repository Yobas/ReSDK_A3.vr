---
name: resdk-editor-workflows
description: Work with ReEditor-only workflows: editor components in `Editor/`, map workspace and build flow, GameObjectsLibrary rebuilds, ReNode and VisualScripting integration, SystemTools validators and generators, and simulation or editor-side debugging. Use when editing editor modules, guiding ReEditor actions, rebuilding editor-owned artifacts, or troubleshooting editor behavior.
---

# ReSDK Editor Workflows

This skill covers operational editor workflows. Pair it with `resdk-dsl` for SQF DSL semantics and subsystem boundaries.

## Entry Workflow

1. Classify the task:
   - editor code in `../../../Editor/`
   - map/content workflow
   - class-library rebuild
   - validator or generator tool
   - ReNode / VisualScripting
   - simulation or editor debugging
2. Open the narrowest matching reference:
   - `references/editor-basics.md`
   - `references/editor-tools-and-simulation.md`
   - `references/renode-and-generated-assets.md`
3. Read `../../../Editor/Editor_init.sqf` if the task touches component load order or editor startup.
4. Identify the source of truth before editing anything generated.

## Mandatory Rules

- Use editor logging functions such as `printLog`, `printWarning`, `printError`, and `printTrace` in editor code. Do not use the host/client `engine.hpp` logging macros inside editor modules.
- Treat `../../../Editor/Bin/Maps`, `../../../host/MapManager/Maps`, `../../../host/ReNode/compiled`, and `../../../M2C.sqf` as generated or derived outputs unless the task explicitly says otherwise.
- Respect component order in `../../../Editor/Editor_init.sqf`. Do not reorder `componentInit()` and `#include` pairs casually.
- Distinguish clearly between workspace save, map storage save, and compiled map build.
- Prefer existing editor actions and helper functions such as `mm_build`, `goasm_builder_rebuildClasses`, `vs_openEditor`, `vs_generateLib`, `systools_GenerateModelData`, and `revoicer_rebuild` over ad hoc workarounds.

## Routing

- Map opening, saving, building, panels, and menus: read `references/editor-basics.md`
- Validators, generators, class rebuilds, simulation, and editor logging: read `references/editor-tools-and-simulation.md`
- ReNode, node-library generation, and generated asset boundaries: read `references/renode-and-generated-assets.md`

## Working Style

- Say whether the task is code-side, content-side, or generated-side before proposing edits.
- Name the owning tool or menu path when the correct workflow is editor-driven.
- If a fix touches generated outputs, prefer editing the source workflow first and regenerating second.
