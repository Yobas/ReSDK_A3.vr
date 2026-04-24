# Craft Schema

Primary source: `../../../host/CraftSystem/README.md`

## Where Craft Data Lives

- Craft configs are YAML files loaded by the server
- Runtime code lives in `../../../host/CraftSystem/`
- Client craft UI lives in `../../../client/CraftMenu/`

## Recipe Types

- `default`: standard menu craft
- `building`: preview or placement craft
- `system`: tied to a craft subsystem such as a special object system
- `interact`: direct item-to-item interaction, usually hand item plus target

## Main Sections

Most recipes are structured around:

- top-level identity fields
- `required`
- `failed_handler`
- `result`
- `options`

## Common Top-Level Fields

- `name`
- `desc`
- `type`
- `category`
- `system_specific`
- `ignored`

## `required`

Used for:

- skill visibility and success gating
- ingredient definitions

Common fields:

- `force_visible`
- `skills`
- `components`

Component fields commonly used:

- `class`
- `name`
- `count`
- `hp`
- `check_type_of`
- `optional`
- `destroy`
- `condition`
- `meta_tag`

## `failed_handler`

Optional failure behavior. Typical structure:

- `handler_type`
- extra parameters such as `item`, `count`, or ranges

## `result`

Common fields:

- `class`
- `count`
- `radius`
- `modifiers`

Interactive recipes may also use extra fields such as:

- `sound`
- `emote`

## `options`

Common examples:

- `collect_distance`
- `craft_duration`

Craft duration expressions may use project-specific helpers described in the README, such as:

- `rta`
- `from_skill(min,max)`
- `irnd(min,max)`
- `rnd(min,max)`

## Interact Recipe Shape

Interactive recipes split ingredient requirements into:

- `hand_item`
- `target`

Do not model these as two anonymous generic components when the recipe is interaction-based.
