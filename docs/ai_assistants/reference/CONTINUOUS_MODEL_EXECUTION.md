---
document_type: technical-reference
authority: informative
owner: project-maintainer
last_reviewed: 2026-10-29
status: active
tracks: 550
---

# Continuous model execution and discrete-event coupling

This reference describes the current implementation in `ContinuousModelDataDefinition`,
its ODE, diffusion, and biochemical subclasses, and the shared continuous component
bridge. It is implementation documentation, not a scientific validation claim.

## Clock and execution contract

The kernel owns the global clock `ModelSimulation::SimulatedTime`. A continuous
definition owns a local clock that starts at zero for each run. At run start it
captures a fixed global origin `T0`; a local time `t` corresponds to absolute time
`T0 + t`. `StopTime` is a local duration, `StepSize` is the nominal local step, and
`StartTime` is not a configurable base-definition property. Compatibility setters
and component validation reject nonzero local starts.

`StepsToSynchronize = 0` evolves synchronously without inserting internal calendar
events. A positive value inserts an `InternalEvent` after that many continuous steps,
with a mandatory final event at `StopTime`; a final partial step is bounded exactly
at the requested endpoint. Inserting an event does not advance the kernel clock.
The kernel confirms time only when it removes and dispatches that event.

`ODESolver` currently defaults to `StopTime=60`, `StepSize=0.01`, and
`StepsToSynchronize=1`. Because definitions with a positive synchronization count
start automatically between replications, this can schedule about 6,000 events per
replication even when no component requests the solver. The default is preserved for
compatibility; changing its meaning or default remains a separate policy decision.

The execution state is represented by `ExecutionState` (`Idle`, `Running`,
`WaitingForSynchronization`, `Completed`, `Failed`, or `Cancelled`). Completion
handlers receive an `ExecutionResult`; callbacks are removed when a component's
request has finished or when the component is destroyed. Replication initialization
cancels pending internal events and notifies outstanding consumers before resetting
the definition.

## Causality at intermediate discrete events

The current scheduler defers mathematical evolution until the corresponding
`InternalEvent` is dispatched. Thus a discrete event between synchronization
boundaries sees the last confirmed continuous state. For example, with `T0=100`,
`StepSize=1`, and `StepsToSynchronize=5`, state evolves through local time 5 only
when the event at global time 105 is dispatched; events at 102 and 104 see the
state from the preceding confirmed boundary. This avoids exposing state from the
future. It also means continuous state is stale relative to physical elapsed time
between boundaries. The correct hybrid semantics for this interval remain open under
`HUM-SCI-002`; alternatives include stepwise synchronization or tentative isolated
state and require a maintainer decision.

## Shared component lifecycle

`ContinuousModelComponent` is the common `ModelComponent` base for
`ContinuousSystemComponent`, `DiffusionSimulate`, and `BioSimulate`. Existing concrete
plugin names, factories, categories, connections, and serialized fields remain on
the concrete classes. The base owns entity IDs while asynchronous work is pending,
subscribes to the requested data definition, checks entity existence again before
dispatch, and routes each entity at most once after completion. It does not own a
solver or mathematical state.

`DiffusionSimulate` and `BioSimulate` submit an `ExecutionRequest`. Requests with the
same active time window join the in-flight run and share its completion. A request
with a conflicting window is rejected. No concurrent mutation of one definition is
started. If a numerical run fails or is cancelled, waiting entities are removed from
the model with a fatal trace because these plugins have no error output port. That
failure-routing policy is explicit in code, but a richer user-facing recovery path
requires a plugin contract decision.

`ContinuousSystemComponent` retains its reactive ODE integration behavior. It uses
the shared completion/forwarding path after a synchronous integration result. It
returns errors from `ODESolver::integrate(targetTime, errorMessage)` and refuses to
advance beyond the next pending synchronization boundary. A discrete event at or
before that boundary can advance the ODE to the current kernel time; the scheduled
callback then integrates the remaining interval. The relationship between this
reactive path and automatic full-window evolution remains an open
compatibility/semantic issue; the component does not silently join an unrelated ODE
run.

## Component time-window compatibility

`DiffusionSimulate` and `BioSimulate` still persist their legacy `StartTime`,
`StopTime`, and `StepSize` controls. When an override window is requested, the
supported local origin is zero. A nonzero component start is rejected during model
or request validation rather than being interpreted as an absolute global time. Field/network window
selection remains available for existing serialized models.

## Evidence and limits

The real-calendar integration regression in
`source/tests/unit/test_simulator_runtime.cpp` verifies that two entities reaching
the same `BioSimulate` before a run completes are both dispatched at completion, and
that a discrete event before the next synchronization sees the last confirmed local
time. Focused diffusion synchronization and component compatibility tests are in
`source/tests/unit/test_plugins_continuous_diffusion.cpp`.

These tests cover specific execution paths. They do not resolve `HUM-SCI-002`, prove
all numerical solvers scientifically valid, or establish that all historical model
files with nonzero component `StartTime` remain semantically compatible.
