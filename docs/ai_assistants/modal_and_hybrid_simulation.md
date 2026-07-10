# Modal and Hybrid Simulation Guidance

## Purpose

This document is the stable AI-assistant reference for modal models, EFSM/Petri-net planning, and hybrid discrete-continuous simulation concerns in GenESyS.

Historical notes from `oldies/` must be checked against current source files before being treated as current implementation facts.

## Scope

Primary source areas:

- `source/plugins/components/ModalModel/`
- `source/plugins/components/Continuous/`
- kernel event scheduling and simulation-time flow
- continuous, biochemical, and whole-cell plugins when they interact with the event calendar

Historical source documents:

- `old_modal_model_efsm_petrinet_plan.md`
- `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md`

## Modal-model policy

Modal models are not generic queueing components. Treat them as a separate semantic domain involving states, transitions, places, nodes, and submodels.

Guidance:

- Preserve state-transition semantics when changing runtime behavior.
- Validate load/save behavior with minimal modal models.
- Keep EFSM and Petri-net concerns separate from generic component catalog concerns.
- Do not mix modal execution semantics into standard component behavior without an explicit abstraction.

## Hybrid simulation policy

Discrete-event time and continuous integration time must be kept conceptually separate unless there is an explicit synchronization contract.

Guidance:

- Do not assume a fixed delta is equivalent to the elapsed simulated time between events.
- Do not change time units or scheduling behavior without explicit regression tests.
- Keep numerical solvers free from kernel event-calendar concerns when possible.
- Components that advance internal continuous state should define how their step size relates to simulated time.

## Known architectural concern

Historical analysis identified a central concern: components that advance in fixed time steps can implicitly assume that their internal step size matches elapsed simulated time, but the event calendar does not automatically guarantee this.

Treat this as a design issue to validate, not as a local bug to patch casually.

## Validation checklist

For modal or hybrid changes, prefer this order:

1. Run unit-test validation.
2. Add focused tests for state-transition or time-step behavior.
3. Validate model load/save round trip.
4. Validate deterministic examples where numerical behavior is involved.
5. Inspect interaction with `InternalEvent` and regular event scheduling if calendar behavior changes.

## Open follow-up tasks

- Inventory all ModalModel components and data structures.
- Revalidate temporal synchronization analysis against current `ModelSimulation` and plugin code.
- Define minimal regression models for EFSM, Petri-net, and continuous/hybrid examples.
- Decide whether a formal time-step synchronization contract is needed for continuous and biochemical plugins.
