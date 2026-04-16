# Module Shape

## Naming

- module names use `CamelCase`
- module init file uses `ModuleName_init.sqf`

## Init File Responsibilities

`ModuleName_init.sqf` is the module entry file that gets connected to the relevant loader.

Put in it:

- includes for the module parts
- default declarations for module-level state
- the canonical entry shape for the module

Prefer declaring file-level defaults in the init file rather than scattering undeclared state across function files.

## Headers

- use `.hpp` for public headers shared with other modules or systems
- use `.h` for private internal headers

If constants are shared, move them into the appropriate header instead of burying them in implementation files.

## Composition Guidance

Avoid both extremes:

- one giant god-file with unrelated concerns
- one module split into too many tiny include fragments

Use a moderate shape:

- one clear module root
- a meaningful small set of included components
- cohesive files grouped by responsibility

## Ask Instead Of Guessing

If you cannot confidently answer either of these questions, stop and ask the user:

- which existing system owns this feature
- whether this should extend an existing module or become a new one
