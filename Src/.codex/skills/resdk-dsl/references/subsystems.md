# Subsystems

## Host

Path:

- `../../../host/`

Default assumptions:

- this is the fullest DSL environment
- OOP and struct systems are normal here
- server architecture, object model, and load order matter

Typical patterns:

- `class`, `extends`, `var`, `func`
- `objParams_*`
- `getSelf`, `setSelf`, `callSelf`, `callFunc`
- `struct(...)`, `def(...)`, `getv`, `setv`, `callv`, `callp`

When editing host code:

- respect class and field declarations
- do not create dynamic fields casually
- prefer public accessors over reaching into internal fields unless local design clearly expects direct access

## Client

Path:

- `../../../client/`

Default assumptions:

- this is client runtime code, not editor code
- client module load order matters
- some helpers are shared with host, but that does not mean host OOP conventions automatically apply everywhere

When editing client code:

- preserve loader order assumptions from `client/loader.hpp`
- do not copy editor-only patterns into client modules
- still prefer project wrappers over raw SQF when the client codebase uses them

## Editor

Path:

- `../../../Editor/`

Default assumptions:

- editor has its own execution model and component system
- editor functions and init flow differ from normal runtime modules
- legacy editor code may close over variables from parent scopes

When editing editor code:

- treat editor conventions as editor-only unless proven otherwise
- do not export editor habits back into new `host` or `client` code
- prefer the editor logging ecosystem and real editor-local patterns over generic assumptions

## CommonComponents

Path:

- `../../../host/CommonComponents/`

Default assumptions:

- this is shared code intended to work across runtime boundaries
- it is not the same thing as ordinary `host` OOP code

When editing CommonComponents:

- do not normalize it into server-only OOP style
- keep it compatible with its shared role
- verify whether a helper is truly available here before using it

## Routing Checklist

Before explaining or editing a file:

1. identify which subsystem owns the file
2. decide whether the file is hand-written or generated
3. read the local loader or init path if module order might matter
4. only then apply style and macro expectations
