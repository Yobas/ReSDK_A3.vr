---
name: resdk-loot-configs
description: Create and edit LootSystem YAML templates and editor-facing loot setup. Use when adding loot collections, defining tags, map or gamemode restrictions, probability, count, health or quality rules, or when checking how a loot template should be assigned and tested through ReEditor.
---

# ReSDK Loot Configs

This skill covers LootSystem data authoring and review. Pair it with `resdk-gameobject-authoring` when loot references new classes, and with `resdk-editor-workflows` when the question is about editor assignment or validator usage.

## Entry Workflow

1. Open the target file under `../../../host/LootSystem/Collections/`.
2. Read `../../../host/LootSystem/README.md`.
3. Open:
   - `references/loot-schema.md`
   - `references/loot-testing-and-checklist.md`

## Mandatory Rules

- Edit loot templates in YAML collections, not in ad hoc runtime code.
- Keep one file focused on a coherent loot theme or usage area.
- Root config names are unique template identifiers.
- Tags must start with `.` to avoid collisions with config names.
- Distinguish exact template names from shared tags used for random selection in the editor.
- Preserve local naming rules for files, configs, and tags.

## Working Style

- State whether the user wants an exact template or a tag-based random pool.
- Call out map or gamemode restrictions explicitly if they affect runtime behavior.
- If validation is needed, prefer the editor-facing loot testing flow before inventing custom diagnostics.
