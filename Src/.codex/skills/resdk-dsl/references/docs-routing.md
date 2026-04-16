# Documentation Routing

Open these project docs when you are uncertain. Prefer reading the narrowest file that answers the question.

## Core Standards

- `../../../../CODE-STANDARDS.md`
  Use for base style, local variables, naming, module-global prefixes, and basic anti-patterns.

- `../../../../Documentation/Project/09_CODING_STANDARDS.md`
  Use for expanded project conventions, anti-patterns, null-check discipline, OOP design expectations, and project-specific preferred idioms.

## Syntax And DSL Semantics

- `../../../../Documentation/Project/03_SYNTAX_GUIDE.md`
  Use for macro behavior, `arg`, OOP and struct syntax, type/null semantics, `FHEADER` and `RETURN`, editor syntax notes, and common misuse patterns.

- `../../../../Documentation/Project/QUICK_REFERENCE.md`
  Use for quick lookup of macro names, helper names, and short examples.

## Architecture And Module Boundaries

- `../../../../Documentation/Project/02_PROJECT_STRUCTURE.md`
  Use for host/client/Editor/CommonComponents boundaries, project layout, and high-level subsystem roles.

- `../../../../Documentation/Project/04_MODULE_SYSTEM.md`
  Use for module shape, entrypoints, registration, dependency order, and loader expectations.

## Debugging And Logging

- `../../../../Documentation/Project/05_DEBUGGING.md`
  Use for debugging workflows, logging expectations, editor-specific debugging behavior, and troubleshooting hints.

## How To Use This Fallback

When unsure:

1. identify whether the uncertainty is about syntax, architecture, or subsystem behavior
2. open the matching doc above
3. prefer doc-backed conclusions over generic SQF assumptions
4. if local module style conflicts with docs, preserve local style only when core DSL/preprocessor/type safety rules remain intact
