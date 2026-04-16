# Loot Testing And Checklist

## Authoring Checklist

- Confirm every referenced class exists
- Confirm the file groups a coherent loot set
- Confirm tags start with `.`
- Confirm `maps` and `gamemodes` restrictions match the intended scope
- Confirm `prob`, `count`, `health`, and `quality` use supported forms
- Confirm whether `all_types_of` is intended or too broad

## Editor Usage

The intended editor workflow from the docs is:

1. Create or update the YAML in `../../../host/LootSystem/Collections/`
2. In ReEditor, select the target container
3. Set the loot template field in the inspector to the config name or tag

## Testing In ReEditor

The docs describe an editor utility for loot testing under validator or loot-check flows.

Use it to:

- generate sample loot from a template or tag
- reload templates after editing without restarting the editor
- visually confirm random outcomes

## Common Failure Modes

- used a config name where a tag was intended
- forgot the leading dot on a tag
- referenced a non-existent class
- overly broad or overly narrow map restrictions
- misleading probabilities or counts

## Escalation Rules

- If loot references brand-new content, finish object authoring first
- If runtime behavior looks wrong but the data is fine, inspect the LootSystem runtime code separately
