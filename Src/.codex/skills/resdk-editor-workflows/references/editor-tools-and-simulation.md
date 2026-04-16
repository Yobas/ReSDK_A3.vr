# Editor Tools And Simulation

## Editor Logging

Use editor logger helpers instead of host/client logging macros:

- `["message"] call printLog`
- `["warning"] call printWarning`
- `["error"] call printError`
- `["trace"] call printTrace`

This is the correct logging ecosystem for code under `../../../Editor/`.

## SystemTools Surface

System tools are registered from `../../../Editor/SystemTools/SystemTools_init.sqf`.

High-value tools and files:

- `GenerateModelData.sqf`
- `ClassValidator.sqf`
- `LightConfigValidator.sqf`
- `LootChecker.sqf`
- `LigthSimulation.sqf`
- shared helpers in `Common.sqf`, including `revoicer_rebuild`

## Rebuild And Regeneration

- Class-library rebuild:
  - usually via `goasm_builder_rebuildClasses`
  - relevant after changing game object definitions the editor consumes
- Automatic editor-side rebuild hooks exist in:
  - `../../../Editor/Core/Core_fileWatcher.sqf`
  - `../../../Editor/Core/Core_postInit.sqf`
- Model map generation:
  - owned by `systools_GenerateModelData`
  - updates `../../../M2C.sqf`

## Simulation

Simulation is not a lightweight preview. It is a local full-cycle run where server and client logic execute on the same machine.

Use simulation when validating:

- gameplay interactions
- client/server integration
- editor-created map content in runtime
- object placement and role or mode startup

Key points from docs:

- startup options live under the `Запуск` menu in ReEditor
- there is a mode-aware simulation launch path
- there is a "last mode and role" shortcut for repeated iteration
- particle simulation has its own launch path

Related source file:

- `../../../Editor/Simulation/Simulation_init.sqf`

## Validation Mindset

- Validator tools are often the fastest correctness check for content mistakes.
- Simulation is the main smoke test when the change affects runtime behavior.
- If a fix only changes an editor tool UI or component behavior, opening ReEditor and exercising that specific flow is usually enough before broader simulation.
