# Implementation Choice

## OOP Class

Choose an OOP class when the feature is:

- a long-lived runtime system
- a gameplay object
- behavior that naturally fits the existing object model

Tradeoff:

- instance creation is a bit slower than a struct
- use it where lifecycle, identity, or object semantics matter more than raw creation speed

## Struct

Choose a struct when the feature is:

- a fast lightweight system object
- a refcount-managed data holder
- a PDO-like construct
- something that benefits from the platform refcounted model and lighter instances

Use it when you want a typed data-and-methods shape without pulling the full OOP object model into the design.

## Function Module

Choose a module of functions when the feature is:

- a system module
- procedural logic
- a utility layer
- behavior that does not need per-instance object identity

This is often the best shape for systemic modules.

## Data Config

Choose data config when the feature is primarily content data rather than executable architecture.

Examples:

- craft recipes
- spawn loot templates

Do not force such content into code-first architecture if the project already expects data-driven authoring.

## Decision Rule

Form is an architectural choice, not just a style preference.

Ask:

1. Is this a long-lived object or system?
2. Does it need identity and object semantics?
3. Is it mostly content data?
4. Is it shared procedural logic?

Then pick:

- class
- struct
- function module
- data config
