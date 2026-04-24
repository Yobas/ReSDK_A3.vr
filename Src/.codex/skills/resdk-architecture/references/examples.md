# Examples

## Example: New GameObject

Question:

- where should a new gameplay item live

Default route:

- `host/GameObjects/Items/...`

Then check:

- craft
- loot
- verbs and interactions
- editor rebuild
- simulation verification

## Example: New Client UI

Question:

- where should a new interface panel or runtime widget live

Default route:

- `client/...`

Do not move it into `host` just because simulation can see both sides.

## Example: New Runtime Module

Question:

- does it belong in an existing system or deserve a new module

Default route:

- extend the obvious existing owner first

If no owner is obvious:

- ask the user instead of inventing a module on guesswork

## Example: New Craft Template

Question:

- is this executable runtime architecture or content data

Default route:

- data config in the craft system

Do not convert a normal craft authoring task into a code-side subsystem unless the task truly requires new runtime behavior.

## Example: New Spawn Loot Template

Question:

- where should spawn loot behavior be authored

Default route:

- loot template data config

Again, prefer data-driven authoring when the project already supports it.
