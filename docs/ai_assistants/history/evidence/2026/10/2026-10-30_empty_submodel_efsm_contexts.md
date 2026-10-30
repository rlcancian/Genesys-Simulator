---
document_type: executed-evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: finalized
scope: empty Submodel and EFSM Network graphical contexts
---

# Empty Submodel and EFSM Network contexts

## Repository and integration

- Repository: `rlcancian/Genesys-Simulator`
- Integration branch: `WorkInProgress`
- Corrective branch: `WiP20261030/submodel-efsm-empty-open`
- Base: `35aff1ec0c4520a5f017a6374eb3235d0c241a9f`
- PR: [#556](https://github.com/rlcancian/Genesys-Simulator/pull/556)
- Merge commit: `5cd3e08548839490c8880e74a6a1ff1493dd7b5e`

## Red checkpoint and correction

The new empty-Submodel and context-level round-trip regressions first failed:
empty Submodel opening was disabled and a child with a non-root level was not
loaded because persistence omitted non-root components. The correction permits
valid Submodel membership without requiring children, wires the existing Create
and Select actions into the Modal context menu, opens newly created Networks,
assigns the active scene level to inserted components, persists `modelLevel`,
and serializes registered components at every level.

## Executed validation

- Focused GUI/EFSM selection: 8/8 passed.
- `cmake --build --preset tests-unit --parallel "$(nproc)"`: passed.
- `ctest --preset tests-unit --output-on-failure`: 1,966 passed; 4 historical
  tests disabled.
- `cmake --build --preset tests-kernel-unit --parallel "$(nproc)"`: passed.
- `ctest --preset tests-kernel-unit --output-on-failure`: 1,966 passed; 4
  historical tests disabled.
- `cmake --build --preset tests-smoke --parallel "$(nproc)"`: passed.
- `ctest --preset tests-smoke --output-on-failure`: 3/3 passed.
- `cmake --build --preset gui-app --parallel "$(nproc)"`: passed.
- `python3 scripts/validate-ai-docs.py`: passed.
- `git diff --check`: passed.
- Hosted PR checks: 11/11 passed, including Debian package build and lifecycle.

## Bounded conclusion and limitation

The shared graphical editing context now supports empty Submodel and EFSM tabs,
first-component insertion, persistence of the editing level, cancellation
without mutation, and shared Network tab reuse. The change does not implement
hierarchical execution semantics for the `Submodel` plugin. GUI action wiring
and state transitions are covered by Qt offscreen integration tests; this record
does not claim a separate manual mouse/keyboard session.
