# Integration Checklists

## New Or Changed Gameplay Object

Check whether the change also needs:

- craft integration
- loot integration
- verbs or interactions
- UI hooks
- editor class-library rebuild
- model regeneration if model mapping changed
- game verification in simulation or runtime
- tests where reasonably possible

This is the most common cross-system architecture case in the project.

## New Or Changed Module

Confirm:

- it is in the correct subsystem
- it follows the module naming and init shape
- it is registered in the correct loader
- public constants and shared macros are in `.hpp`
- private internals are in `.h`

## New Shared Helper

Before placing code in `CommonComponents`, verify:

- both client and server truly need it
- it does not pull in side-specific behavior
- it will not bloat the shared layer unnecessarily

If those are not clearly true, place it in the owning subsystem instead.

## Architectural Completion

A task is architecturally complete when:

- the module or change is registered correctly
- the code lives in the correct subsystem
- nearby required integrations were checked
- the feature was verified in game
- tests were added where practical
- merge-request tests did not fail

This is about completion of the design and integration, not just “the file compiles”.
