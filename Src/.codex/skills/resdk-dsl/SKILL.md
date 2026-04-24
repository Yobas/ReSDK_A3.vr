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

- Use project-first semantics. When interacting with ReSDK code, do not default to native SQF/Arma behavior if the project already defines a DSL wrapper, helper, macro, or documented convention for that concern.
- Native SQF/Arma semantics are an exception path. Only use them when no project-level contract exists for the case, and verify that exception before coding or reviewing.
- In review mode, compare code against canonical ReSDK DSL idioms, not just against the lower bar of "valid SQF". Code can be runtime-valid and still be a standards smell if it bypasses a project-preferred idiom.
- Respect RV preprocessor limits. It is text-based, brittle, and not context-aware.
- Any helper that looks like a function call may still be a macro wrapper from `engine.hpp`, `oop.hpp`, or `text.hpp`. Verify `#define` bodies before passing complex expressions or inline code blocks.
- Do not pass non-trivial anonymous code blocks directly into macro wrappers such as delayed-call, next-frame, networking, or helper macros that accept `code`-like arguments. Prefer `private _code = { ... };` and pass the symbol instead.
- In stringify-dependent OOP macros such as `getSelf`, `getVar`, `setVar`, `callFunc`, `callSelf`, and similar forms, any spaces that survive preprocessing inside the member-name argument become part of the final string and break lookup.
- For null/type/control-flow helpers such as `isNullVar`, `isNullReference`, `valid`, `equals`, `not_equals`, `array_copy`, `FHEADER`, `RETURN`, and `exitWith`, first determine the project-level contract for the value category you are handling, then choose the helper that matches that contract.
- When a project helper explicitly occupies a common idiom slot, prefer it even if raw SQF syntax would also work. Example: use `ifcheck(...)` for expression-level ternary selection instead of inline `if ... then ... else ...` expressions.
- When using native RV/Arma commands or operators, verify the official Bohemia docs before making claims about return shape, key names, optional fields, or version behavior.
- For new code, standards and docs win.
- For edits to existing code, preserve local module style unless it breaks core DSL, preprocessor, type, or architecture rules.
- Do not treat `lang.hpp` as a model for new code. It is legacy compatibility context only.
- Avoid carrying editor-specific patterns into new `host` or `client` code.
- Avoid closing over parent-scope variables by default, even though legacy editor code sometimes does this.
- Treat load/include/import order as part of the architecture, not formatting noise.

## Which Reference To Read

- DSL semantics, macro pitfalls, types, null handling, control flow:
  See [references/dsl-foundation.md](references/dsl-foundation.md)
- Native engine command docs and official lookup workflow:
  See [references/docs-routing.md](references/docs-routing.md)
- Subsystem routing and style boundaries:
  See [references/subsystems.md](references/subsystems.md)
- Generated/system files and legacy compatibility:
  See [references/generated-and-legacy.md](references/generated-and-legacy.md)
- Project documentation fallback map:
  See [references/docs-routing.md](references/docs-routing.md)

## Working Style

- When explaining code, say which subsystem you are in before interpreting conventions.
- When proposing edits, check whether the file is hand-written or generated/system-owned.
- Before introducing any raw SQF/Arma idiom into ReSDK code, ask whether the project already has a canonical DSL way to express the same thing.
- When reviewing, explicitly ask "is this the canonical ReSDK way to express this idiom?" before concluding that code is standards-compliant.
- If a change uses a helper with a `code` argument, inspect the helper definition before writing the call site.
- When uncertain, read more project docs instead of guessing from general SQF knowledge.
