---
name: resdk-tests-validation
description: Choose the right validation path for ReSDK changes: simulation, editor validators, map and generated-asset rebuilds, and the legacy `host/UnitTests` harness. Use when asked what to test after a change, how to validate a subsystem or content edit, whether a rebuild or regeneration is required, or how much confidence a change currently has.
---

# ReSDK Tests Validation

This skill routes validation by change type instead of pretending there is one default test command for the whole project.

## Entry Workflow

1. Classify the change:
   - editor code
   - map or content data
   - host gameplay code
   - client UI or rendering
   - generated asset workflow
   - legacy `host/UnitTests`
2. Read `references/validation-matrix.md`.
3. If the task explicitly mentions `UnitTests` or old test macros, also read `references/legacy-unit-tests.md`.

## Mandatory Rules

- Treat simulation and editor validators as the primary validation path for most gameplay, content, and editor changes.
- Be explicit when a change likely requires regeneration before testing, for example class-library rebuild, map build, model-map generation, or ReNode library generation.
- Do not oversell `host/UnitTests` as the default safety net. `../../../../Documentation/PROJECT_ARCHITECTURE.md` marks it as a legacy module.
- If no automated validation exists, say so plainly and recommend the narrowest reproducible smoke test.

## Working Style

- Return the smallest useful checklist for the actual change.
- Separate “must regenerate first” from “must verify afterward”.
- Prefer workflow-specific validation over generic advice like “run everything”.
