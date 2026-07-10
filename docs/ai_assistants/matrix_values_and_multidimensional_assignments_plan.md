# Matrix Values and Multidimensional Assignments Plan

- Date: 2026-07-04
- Branch: `WiP20261`
- Status: in progress

## Scope

Fix the currently failing unit tests around scalar reads and writes using
multidimensional textual indexes for variables and attributes, with emphasis on
expressions like `Var[1,2]=3.14` and `Attr[1,2,3]=36`.

## Historical context

- `Variable` values have historically behaved as a sparse matrix keyed by a
  textual index string.
- The scalar position is represented by the empty key `""`.
- One-dimensional, two-dimensional, and N-dimensional positions are represented
  by comma-separated textual keys such as `"1"`, `"1,2"`, and `"1,2,3"`.
- Missing positions must read back as `0.0`.
- The same sparse semantics are being extended to entity attributes, with the
  difference that variables are global and attributes are stored per entity.

## Current state

- Reading scalar, vector, matrix, and N-dimensional variable entries is already
  partially covered by the parser tests.
- Attribute reads and writes during an event depend on the active entity.
- The open work is to make the parser/store path consistent for scalar and
  indexed reads and writes without introducing matrix-to-matrix assignment yet.
- The new focused tests in `source/tests/unit/test_parser_expressions.cpp`
  now cover:
  - direct kernel access for `Variable` and `Attribute`;
  - parser-based indexed assignment for variables;
  - parser-based indexed assignment for attributes during an event using the
    established `ParserAttrND` path.
- The `ParserAssignAttrND` probe was exercised as a separate sanity check and
  now passes with the same sparse semantics; it remains a diagnostic case, not
  a new semantic branch.

## Phase 1: active now

- Reproduce the currently failing unit tests related to indexed scalar
  assignment and lookup.
- Confirm whether the failure originates in the parser grammar, sparse value
  store, variable runtime wrapper, attribute/entity storage, or test setup.
- Apply the smallest behavior-preserving fix that keeps the sparse semantics and
  the legacy textual key format intact.
- Current validation status: the targeted `ParserExpressionsTest.*` suite is
  green, and `ctest --preset tests-unit --output-on-failure` completed with all
  tests passing.

## Future phases

- Matrix-to-matrix assignment.
- Explicit dimension declaration and validation.
- Shape compatibility checks.
- Future integration with Octave and R.
- Stronger typed index/dimension representations.
- Persistence and serialization round trips for sparse multidimensional values.
- Additional parser and runtime tests for edge cases.

## Technical decisions recorded

- Keep the current canonical sparse key format as a comma-separated string.
- Preserve `0.0` for missing positions.
- Avoid broad parser refactors unless the current grammar cannot express the
  expected indexed assignment semantics.
- Prefer a local fix in the specific runtime/store path if the grammar is already
  producing the right lookup key.

## Explicit limitations

- Matrix-to-matrix assignment is out of scope for this phase.
- Shape validation is out of scope for this phase.
- No new matrix typing system is introduced here.
- No persistence or serialization redesign is attempted here.

## How to record future updates

Each later update should include:

- Date.
- Branch.
- Commit, if one exists.
- PR, if one exists.
- Assistant responsible.
- Files affected.
- Validation commands executed.
- What changed since the previous update.
- Next steps.
