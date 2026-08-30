---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-08-30
review_cadence: on-process-allocation-contract-change
status: active
tracks: 511
---

# Process AllocationType vs internal Delay

## Purpose

This note isolates one unresolved semantic question discovered during the
Arena compatibility audit so the maintainer can decide it later without
blocking the rest of the Arena close-out work.

## Date

- Recorded on: `2026-08-30`

## Short version

`Process` exposes an `AllocationType` control, but the current code routes it
to the internal `Seize` component instead of the internal `Delay` component.

That matters because Arena's `Process` module uses its allocation/category
field to classify the time spent in the module's processing delay as
`VA`/`NVA`/`Wait`/`Transfer`/`Other`.

Today, GenESyS appears to classify the internal `Delay` time of `Process` as
`Wait` permanently, because `Delay` keeps its own default allocation and no
reviewed code path changes it through `Process`.

## Code path observed

- `source/plugins/components/DiscreteProcessing/Process.cpp`
  - `setAllocationType()` forwards to the internal `_seize`
  - no reviewed path was found that calls `_delay->setAllocation(...)`
- `source/plugins/components/DiscreteProcessing/Delay.h`
  - default allocation is `Util::AllocationType::Wait`
- `source/plugins/components/DiscreteProcessing/Delay.cpp`
  - delay time is credited to category-specific entity totals/statistics based
    on the `Delay` allocation
- `source/plugins/components/DiscreteProcessing/Seize.cpp`
  - `Seize` stamps `Entity.Allocation.<ResourceName>`
- `source/plugins/components/DiscreteProcessing/Release.cpp`
  - `Release` reads that allocation back to credit resource-held time

## Why this is a decision

Two distinct time spans exist inside `Process`:

1. resource-holding time, mediated by `Seize`/`Release`
2. processing delay time, mediated by `Delay`

Both can legitimately be categorized, but they are not the same quantity.

The unresolved question is not whether categories exist. They do.

The unresolved question is:

Should `Process::AllocationType` govern:

1. the internal `Delay` category, because it represents the module's
   processing time in Arena terms; or
2. the internal `Seize`/resource-held category, because the current wiring is
   intentional; or
3. both, via a broader redesign not yet authorized?

## Why no autonomous fix was applied

The audit found a strong bug signal, but not enough proof of intended contract
to change behavior safely without maintainer confirmation.

If the intended contract is Arena-like, then routing `Process::AllocationType`
to `_seize` is probably wrong.

If the current GenESyS contract intentionally classifies only the
resource-holding phase through `Process::AllocationType`, then changing the
wiring would create a semantic regression.

## Practical impact

If the Arena-like interpretation is correct, then a modeler setting
`Process::AllocationType = ValueAdded` may currently still get the module's own
delay time credited as `Wait`, not `ValueAdded`.

That can affect:

- entity category-time totals such as `Entity.TotalWaitTime`
- entity-type statistics collectors such as `<EntityType>.WaitTime`
- downstream reporting and Arena equivalence claims

## What to decide later

Please decide one of these options before the Arena front is finally closed:

1. `Process::AllocationType` must govern the internal `Delay`
2. `Process::AllocationType` must continue to govern the internal `Seize`
3. the component needs two separate controls in the future
4. keep current behavior and document it explicitly as a GenESyS divergence

## Recommended next step when decision time comes

After the decision:

1. write a focused regression test that proves the intended accounting target
2. apply the smallest wiring change consistent with the chosen contract
3. rerun focused tests plus broader regression levels
4. update `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md`
