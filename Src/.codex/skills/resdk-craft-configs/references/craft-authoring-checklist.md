# Craft Authoring Checklist

## Before Editing

- Confirm whether the recipe should be `default`, `building`, `system`, or `interact`
- Confirm all referenced classes already exist
- Check whether the result item needs supporting gameplay code beyond data

## While Editing

- Keep the config in YAML, not inline SQF
- Preserve local naming and category conventions
- Ensure `skills` matches the intended gating behavior
- Validate `count`, `hp`, `check_type_of`, `optional`, and `destroy`
- Use ranges only where the system already supports them
- Keep modifier names exactly as expected by runtime code

## Cross-System Checks

After changing a craft recipe, consider whether you also need to touch:

- `../../../client/CraftMenu/craftmeun_init.sqf`
- craft system runtime code in `../../../host/CraftSystem/`
- result item classes in `../../../host/GameObjects/`
- source items used as ingredients

## Validation Suggestions

- Verify the recipe appears in the intended category
- Verify visibility behavior when skills are missing
- Verify ingredient matching and destroy behavior
- Verify the result and modifiers are correct
- For `building`, verify preview or placement behavior
- For `interact`, verify the hand item and target routing

## Escalation Rules

- If a recipe needs new classes, switch to `resdk-gameobject-authoring`
- If the issue looks like UI behavior rather than config shape, inspect `client/CraftMenu`
- If runtime code is involved, use `resdk-dsl` alongside this skill
