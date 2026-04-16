# DSL Foundation

## Core View

ReSDK is not plain SQF. It is a project DSL built from:

- `engine.hpp` for core helpers, null/type utilities, control-flow helpers, logging wrappers, load/import helpers
- `oop.hpp` for class/object macros
- `text.hpp` for text and formatted-string helper macros
- `struct.hpp` for struct-style data and methods

Read these headers first on any serious task.

## Preprocessor Reality

- RV preprocessor behavior is a hard constraint, not an implementation detail.
- Treat preprocessing as fragile textual substitution.
- Macro behavior is not context-aware.
- Keep macro usage syntactically strict.
- Do not improvise with formatting around sensitive macro arguments.

## Code-Accepting Macro Wrappers

Some helpers that read like ordinary function calls are actually `#define` wrappers around engine calls.

Examples to verify before use:

- `invokeAfterDelay`
- `invokeAfterDelayParams`
- `nextFrameParams`
- networking helpers and similar wrappers from `engine.hpp` or subsystem headers

Rules:

- if the helper is a macro, treat each argument as raw text that will be substituted before compilation
- do not pass non-trivial anonymous code blocks directly into these wrappers
- do not pass comma-heavy expressions into macro arguments unless you have checked the final expansion is safe
- if the code block contains `params`, `foreach`, `format`, `vec*`, `arg`, `getOrDefault [...]`, nested macro calls, or other comma-rich DSL constructs, assume inline use is unsafe
- prefer:
  `private _code = { ... };`
  `invokeAfterDelayParams(_code,delay,_params);`
- only inline a code block into a macro wrapper when the block is trivial and you have explicitly checked the `#define`

Review habit:

- before writing or approving a call site that passes `code` into a helper, open the helper definition and reason about the exact textual expansion

## Macro and Helper Priorities

Prefer project wrappers where they exist, especially for:

- comparisons: `equals`, `not_equals`, `equalTypes`, `not_equalTypes`
- null and validity: `isNullVar`, `isNullReference`, `valid`
- arrays: `array_copy`, `array_remove`, related helpers from `engine.hpp`
- control flow: `FHEADER`, `RETURN`, `IF_RET`, `exitWith`
- expression-level ternary selection: `ifcheck`

Avoid falling back to raw operators when the codebase already uses a canonical DSL helper unless local file style clearly requires otherwise and the change remains safe.

## Project-First Semantics

Do not make decisions against ReSDK code by asking "what would plain SQF do here?" first.

Ask in this order instead:

1. does the project already define a DSL helper, macro, wrapper, or documented convention for this concern
2. if yes, use the project contract
3. if no obvious contract exists, verify that by reading the relevant headers or docs
4. only then consider raw SQF/Arma semantics as an explicit exception

This applies to:

- nil/null and existence checks
- comparisons and type handling
- control flow helpers
- expression idioms such as ternary-style selection
- delayed execution helpers
- RPC/network wrappers
- native command return-value interpretation

Bad default:

- choosing between raw SQF primitives by habit and only later checking whether ReSDK wraps them

Good default:

- treating raw SQF/Arma as the fallback path, not the primary mental model

## Canonical Idiom Review

During review, the question is not only whether code is legal SQF or whether it will run.

The question is also whether it matches the canonical ReSDK DSL idiom for the operation.

Implications:

- "valid SQF" is not enough to call code standards-compliant
- if the project has a named helper for a common idiom, bypassing it is at least a smell unless there is a local, documented reason
- review findings should call out canonical-idiom drift even when the code is functionally correct

Example:

- for expression-level ternary selection, prefer `ifcheck(condition,a,b)`
- inline `if (condition) then {a} else {b}` used as an expression may still run, but it should be reviewed against `ifcheck` first, not against plain SQF permissiveness

## Stringify-Sensitive OOP Macros

These macros depend on stringification of the field or method argument after preprocessing:

- `getSelf`
- `setSelf`
- `getVar`
- `setVar`
- `callFunc`
- `callFuncParams`
- `callSelf`
- `callSelfParams`

Rule:

- if spaces get into the member-name argument, they become part of the resulting string key
- this can silently turn `value` into `" value"` or `"value "`
- that breaks field or method lookup

Do not normalize these calls casually. Preserve tight argument formatting.

## Type Model

Always reason about which kind of value you have:

- plain scalar/string/array/hashmap-like value
- nil-like variable state
- platform reference such as object/control/display/location
- ReSDK OOP object
- ReSDK struct

Choose checks accordingly:

- `isNullVar` for nil-like variable state
- `isNullReference` for platform references and deleted references
- `valid` for project validity semantics, especially OOP-centric checks

Default rule:

- choose the ReSDK helper that matches the value category first
- do not casually drop to raw native checks just because they are familiar from plain SQF
- if you think a raw native check is required, verify why the project helper is not appropriate before using it

Do not spray null checks everywhere. Add them where values can really arrive from unsafe/public/external boundaries.

## OOP vs Struct Semantics

- OOP macros and object access use the project object system from `oop.hpp`
- struct access uses the struct system from `struct.hpp`
- do not assume they behave identically
- keep exact casing for struct members and methods
- do not project OOP conventions onto structs without verification

## Control Flow

Understand scope before rewriting logic:

- `exitWith` exits the current scope
- `FHEADER` declares a named outer scope
- `RETURN(value)` exits that named scope with a value

Use `RETURN` when returning from nested scopes in functions that intentionally use `FHEADER`.
Use plain `exitWith` for local early exits when a named return scope is not required.

## Local Variables and Prefixes

- local variables should stay `private`
- local variables should use `_name`
- module globals and module functions should keep module prefixes

## New Code Default

For new code:

- follow standards and docs first
- do not introduce `lang.hpp`
- prefer project helpers over raw SQF idioms
- do not guess the structure of native engine return values such as HashMaps, arrays, or option dictionaries
- when a native command returns keyed data, use documented keys and documented optional-field behavior rather than fallback guessing
- do not assume editor or legacy shortcuts are acceptable in normal runtime code
