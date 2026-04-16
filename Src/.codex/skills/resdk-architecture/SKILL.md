---
name: resdk-architecture
description: Route new code and content to the correct ReSDK subsystem and enforce architectural invariants. Use when deciding whether work belongs in `host`, `client`, `Editor`, or `CommonComponents`; when creating or extending modules; when choosing between OOP classes, structs, function modules, and data configs; or when checking whether a change is architecturally complete.
---

# ReSDK Architecture

This skill is the project-level routing and invariants guide. Use it before creating new systems, modules, UI, gameplay objects, craft configs, or shared helpers. Pair it with `resdk-dsl` for SQF DSL semantics and with subsystem skills for detailed workflows.

## Entry Workflow

1. Classify the task:
   - runtime server logic
   - runtime client logic
   - editor-only logic
   - truly shared helper or algorithm
   - gameplay object
   - craft or loot data
2. Open the narrowest matching reference:
   - `references/layer-boundaries.md`
   - `references/module-shape.md`
   - `references/implementation-choice.md`
   - `references/integration-checklists.md`
   - `references/anti-patterns.md`
   - `references/examples.md`
3. If you cannot confidently identify the existing owning system or extension point, stop and ask the user instead of creating a new module on guesswork.

## Mandatory Rules

- Never place client code in a server directory. It may appear to work in simulation and still fail in production because the real client build will not load it correctly.
- Treat the server as the source of truth for gameplay data. Treat the client primarily as the presentation layer.
- Remember the exception: some visual state is still server-authoritative, for example server-driven animation setup for AI mobs.
- Keep `Editor` isolated from runtime gameplay code except in already established bridge points.
- Keep `CommonComponents` small and genuinely shared. Put code there only when both server and client truly need it.
- Prefer project macros and helpers over raw SQF operators when the project already has canonical wrappers.
- Do not capture parent or external scope variables as an architecture habit. Pass data through parameters instead.

## Sensitive Systems

Be extra conservative around:

- base OOP systems
- server and client loaders
- core low-level systems such as `NOEngine`

If a task touches these areas, preserve existing structure and only make the minimum change needed.

## Routing

- Host, client, editor, and shared boundaries: read `references/layer-boundaries.md`
- Naming, init files, headers, and module composition: read `references/module-shape.md`
- Choosing class vs struct vs function module vs data config: read `references/implementation-choice.md`
- Object-side follow-up checks and architectural completion: read `references/integration-checklists.md`
- Bad patterns to reject during design and review: read `references/anti-patterns.md`
- Canonical placement examples: read `references/examples.md`

## Working Style

- State the target subsystem before proposing or making changes.
- Prefer extending an obvious existing system over inventing a parallel one.
- If the owning system is unclear, ask instead of improvising a new module.
- Treat "works in simulation" as insufficient when a layer boundary is violated.

