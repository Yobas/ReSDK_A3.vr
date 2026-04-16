---
name: resdk-craft-configs
description: Create and edit CraftSystem recipe YAML files and related craft integration. Use when adding or changing craft recipes, recipe types (`default`, `building`, `system`, `interact`), ingredient or result schemas, skill gates, failure handlers, craft durations, or when checking whether a craft change also needs server or client menu updates.
---

# ReSDK Craft Configs

This skill is for CraftSystem content authoring and craft-specific review. Pair it with `resdk-dsl` for SQF semantics and with `resdk-gameobject-authoring` when the recipe depends on new classes.

## Entry Workflow

1. Inspect the target YAML and `../../../host/CraftSystem/README.md`.
2. Decide whether the task is:
   - data-only recipe work
   - recipe plus new object classes
   - recipe plus runtime code in `../../../host/CraftSystem` or `../../../client/CraftMenu`
3. Read:
   - `references/craft-schema.md`
   - `references/craft-authoring-checklist.md`

## Mandatory Rules

- Prefer authoring recipes in YAML configs rather than hardcoding recipe content in SQF.
- Keep recipe class names aligned with actual game object classes.
- Match recipe type to behavior:
  - `default`
  - `building`
  - `system`
  - `interact`
- Use `system_specific` only with `type: system`.
- For interactive recipes, use the `hand_item` and `target` layout instead of a flat generic component list.
- Preserve local category names, system identifiers, modifier names, and existing config style.

## Working Style

- Say whether the change is pure config, config plus UI, or config plus gameplay code.
- If a recipe references new items, call out that the object classes must exist first.
- If validation is requested, route through `resdk-tests-validation` for the narrowest smoke test.
