# Layer Boundaries

## Host

Use `../../../host/` for:

- authoritative gameplay logic
- authoritative world state
- game systems and long-lived runtime systems
- OOP classes and server-owned object behavior
- networking, validation, and replication control

Hard rule:

- client code does not belong here

Reason:

- simulation can hide this mistake
- production client packaging will not

## Client

Use `../../../client/` for:

- UI
- input
- rendering and presentation
- local visual behavior
- client-only helpers

Default rule:

- the client presents state rather than owning game truth

Important exception:

- some presentation can still be server-authoritative, for example animation state for AI mobs

## Editor

Use `../../../Editor/` for:

- editor tools
- editor windows and component systems
- validation and generation flows owned by ReEditor
- simulation launch and editor-side workflows

Isolation rule:

- keep editor code separate from gameplay runtime code
- only use already established bridge points where the project intentionally connects editor and game systems

## CommonComponents

Use `../../../host/CommonComponents/` only for code that is truly shared by client and server.

Good fits:

- common helpers
- algorithms
- shared low-level utilities
- common core pieces already designed for both sides

Bad fits:

- code that only one side uses
- code placed there for convenience
- large subsystems that bloat both builds

## Source Of Truth

- server is authoritative for gameplay data
- client is mostly responsible for display
- if a display decision is explicitly server-driven in current architecture, preserve that ownership model
