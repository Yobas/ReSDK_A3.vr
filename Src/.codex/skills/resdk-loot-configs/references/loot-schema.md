# Loot Schema

Primary source: `../../../host/LootSystem/README.md`

## Where Loot Data Lives

- Loot templates live in `../../../host/LootSystem/Collections/`
- Editor assignment happens through the container inspector field for loot template selection

## Root Shape

Each top-level YAML object is a loot template config.

Important root fields include:

- `name`
- `tag`
- `maps`
- `gamemodes`
- `health`
- `quality`
- `pass_count`
- `items`

## Restrictions

Supported restriction styles include:

- exact name
- `regex`
- `typeof` where supported

Use them to limit availability by map or mode.

## Item Pool

`items` maps class names to per-item options.

Common item-level options:

- `name`
- `prob`
- `count`
- `health`
- `quality`
- `all_types_of`

## Naming Rules

- Keep filenames in English with `.yml`
- Use config names without spaces
- Tags must start with `.`
- Prefer hierarchical tags such as `.clothes.civilian`

## Important Distinction

- Config name: exact template ID
- Tag: random selection bucket across multiple templates

That distinction matters when assigning the loot setup in ReEditor.
