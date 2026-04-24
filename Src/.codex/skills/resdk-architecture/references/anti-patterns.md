# Anti-Patterns

## Cross-Layer Placement Errors

- placing client code under `host`
- treating simulation success as proof that the placement is correct
- coupling editor code directly into runtime code outside approved bridge points

## Bad Module Shapes

- god-files that accumulate too many unrelated responsibilities
- over-fragmented modules with too many tiny submodule includes

Both make maintenance and ownership harder.

## State Placement Problems

- keeping module state inside random function files
- relying on implicit undeclared file-level variables instead of init-time defaults

Prefer explicit module state declared in the init file.

## Scope Abuse

- reading variables from outer scope instead of passing them as parameters

Do not normalize closure-style external scope usage as a design pattern.

## Ignoring Project Macros

- using raw SQF operators where the project has clearer or canonical macro wrappers

Favor project conventions so behavior, readability, and safety remain consistent.

## Guess-Driven Architecture

- creating a new module because the owning system is unclear
- inventing a parallel subsystem instead of asking when the extension point is uncertain

When ownership is unclear, stop and ask.
