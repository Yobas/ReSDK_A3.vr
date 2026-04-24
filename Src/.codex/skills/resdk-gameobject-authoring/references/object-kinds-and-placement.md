# Object Kinds And Placement

Primary sources:

- `../../../../Documentation/PROJECT_ARCHITECTURE.md`
- `../../../../Documentation/Project/02_PROJECT_STRUCTURE.md`

## Main Families

- `Item`
  - pickable and usable items
  - smallest practical render-distance assumptions
  - usually live under `../../../host/GameObjects/Items/`
- `IStruct`
  - interactive structures such as doors, lamps, consoles, furniture with behavior
  - medium-distance structural gameplay content
  - usually live under `../../../host/GameObjects/Structures/`
- `Decor`
  - mostly decorative large scene objects
  - widest render-distance assumptions
  - usually live under decor or construction-oriented folders
- `BasicMob`
  - living entities, player or AI controlled
  - usually live in mob-oriented object areas and adjacent systems

## Why The Family Choice Matters

- gameplay semantics differ
- replication assumptions differ
- editor/library expectations differ
- downstream systems such as interactions, inventory, or verbs differ

## Placement Heuristic

Prefer this order when choosing a file location:

1. existing sub-family file already holds close peers
2. existing feature folder for that family
3. only then create a new file or subfolder, and keep it consistent with the module loader or family organization

## Common Adjacent Systems

Depending on the object, follow-up changes may be needed in:

- craft recipes
- loot templates
- verbs or interactions
- client visual or proxy modules
- lighting helpers
- map content and editor library flows
