# Legacy Unit Tests

## Status

Project docs label `../../../host/UnitTests` as a legacy or old unit-test module, not the default validation path for modern changes.

Use it when:

- the user explicitly asks about that harness
- the affected code already has tests there
- you are maintaining the old test framework itself

## Key Files

- `../../../host/UnitTests/init.sqf`
- `../../../host/UnitTests/loader.sqf`
- `../../../host/UnitTests/TestFramework.h`
- `../../../host/UnitTests/TestsCollection/*.sqf`

## Core Macros

Defined in `TestFramework.h`:

- `TEST(name)`
- `TEST_F(fixture,testname)`
- `FIXTURE_SETUP(name)`
- `FIXTURE_TEARDOWN(name)`
- assertions such as `ASSERT`, `EXPECT`, `ASSERT_EQ`, `EXPECT_EQ`

## Observed Runtime Shape

- test modules are loaded from `src\host\UnitTests\TestsCollection\%1.sqf`
- the runner logs per-test start and success/error state
- the runner exits through `RBuilder_exit`

## Practical Guidance

- Prefer touching this harness only when already working inside it
- If a modern change has no coverage there, do not force unrelated tests into the legacy framework just to claim automation
- When mentioning validation status, say “legacy UnitTests” rather than just “tests” so the scope is clear

## Related Build Hint

Project build docs mention `rb run -def TEST_ALL -def DEBUG` as the legacy debug test-launch pattern.
