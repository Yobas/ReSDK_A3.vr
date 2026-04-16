---
name: resdk-dsl
description: Use when working in ReSDK_A3 project code that looks like SQF but actually relies on the project's macro DSL, RV preprocessor constraints, OOP/struct wrappers, subsystem-specific rules, or generated system files. Read this skill before explaining, editing, reviewing, or generating host/client/Editor/CommonComponents code.
---

# ReSDK DSL

This skill is the project-specific onboarding guide for ReSDK code. Treat the codebase as a custom DSL over SQF, not as plain SQF.

## Entry Workflow

1. Read the DSL foundation headers first:
   - `../../../host/engine.hpp`
   - `../../../host/oop.hpp`
   - `../../../host/text.hpp`
   - `../../../host/struct.hpp`
2. Determine the subsystem from the task or file path:
   - `../../../host/` -> server DSL, OOP, structs
   - `../../../client/` -> client runtime code
   - `../../../Editor/` -> editor-specific component system
   - `../../../host/CommonComponents/` -> shared low-level/common layer
   - `../../../M2C.sqf`, `../../../host/MapManager/Maps/*`, `../../../host/ReNode/compiled/*` -> generated/system artifacts
3. Open the matching reference file from `references/` before making non-trivial claims.
4. If any macro behavior, type rule, control flow rule, subsystem convention, or architectural boundary is unclear, open the project docs listed in [references/docs-routing.md](references/docs-routing.md).

## Mandatory Rules

- Prefer project helpers and macros over raw native operators when the project already defines a canonical wrapper.
- Respect RV preprocessor limits. It is text-based, brittle, and not context-aware.
- In stringify-dependent OOP macros such as `getSelf`, `getVar`, `setVar`, `callFunc`, `callSelf`, and similar forms, any spaces that survive preprocessing inside the member-name argument become part of the final string and break lookup.
- Choose `isNullVar`, `isNullReference`, `valid`, `equals`, `not_equals`, `array_copy`, `FHEADER`, `RETURN`, and `exitWith` based on type and scope semantics, not by habit.
- For new code, standards and docs win.
- For edits to existing code, preserve local module style unless it breaks core DSL, preprocessor, type, or architecture rules.
- Do not treat `lang.hpp` as a model for new code. It is legacy compatibility context only.
- Avoid carrying editor-specific patterns into new `host` or `client` code.
- Avoid closing over parent-scope variables by default, even though legacy editor code sometimes does this.
- Treat load/include/import order as part of the architecture, not formatting noise.

## Which Reference To Read

- DSL semantics, macro pitfalls, types, null handling, control flow:
  See [references/dsl-foundation.md](references/dsl-foundation.md)
- Subsystem routing and style boundaries:
  See [references/subsystems.md](references/subsystems.md)
- Generated/system files and legacy compatibility:
  See [references/generated-and-legacy.md](references/generated-and-legacy.md)
- Project documentation fallback map:
  See [references/docs-routing.md](references/docs-routing.md)

## Working Style

- When explaining code, say which subsystem you are in before interpreting conventions.
- When proposing edits, check whether the file is hand-written or generated/system-owned.
- When uncertain, read more project docs instead of guessing from general SQF knowledge.
