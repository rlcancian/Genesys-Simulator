# Continuous and Hybrid Plugin Guidance

## Scope

This guide covers continuous simulation plugins and hybrid/discrete-continuous integration concerns.

Observed source area:

- `source/plugins/components/Continuous/`

Known sampled files:

- `LSODE.h`
- `LSODE.cpp`
- `DiffEquations.h`
- `DiffEquations.cpp`

Related historical material:

- `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md`
- `old_modal_model_efsm_petrinet_plan.md`

## Guidance

- Treat continuous time, numerical integration, and discrete-event scheduling interactions as correctness-sensitive.
- Do not change time units, event scheduling, or integration step behavior without explicit tests.
- Keep hybrid execution semantics documented separately from standard discrete-event semantics.
- Validate numerical behavior with deterministic examples where possible.

## Open follow-up

- Revalidate current `Continuous` plugins and their dependencies.
- Consolidate temporal synchronization analysis into a stable hybrid-simulation guide.
- Define numerical regression tests for representative continuous/hybrid cases.
