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
events. It publishes the final state before returning while leaving the kernel clock
at the caller's current time; the whole local interval is therefore an out-of-calendar
operation and cannot interleave ordinary discrete events. Use a positive value when
calendar-time observations are required. A positive value inserts an `InternalEvent` after that many continuous steps,
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

## Confirmed state and per-step observation

The maintainer's 2026-10-29 decision for `HUM-SCI-002` in this execution path is
that ordinary discrete events read the latest state confirmed at a global
synchronization. Continuous observers receive each successfully calculated
working state, including steps between synchronization boundaries. The kernel
clock remains owned by `ModelSimulation`; creating an observer notification does
not advance it.

When `StepsToSynchronize` is positive, the continuous definition first places the
next boundary `InternalEvent` in the calendar, then calculates each continuous
step through that boundary. After each successful step it invokes its active
observers. For `T0=100`, `StepSize=1`, and `StepsToSynchronize=5`, observer
notifications carry local/absolute times `(1,101)` through `(5,105)`. The
ordinary synchronization event remains at 105. If an observer at local time 3
requests a discrete event for absolute time 103, that event is inserted before
the kernel reaches it and the kernel dispatches it at 103. A discrete event at
102 sees the previous confirmed state, while the observer payload at step 3
contains the working state at local time 3.

`getCurrentTime()` and ordinary model getters expose confirmed values during
lookahead. The observer payload uses a class-specific, read-only borrowed view:
`ODESolver::StateView`, `DiffusionField::StateView`, or `BioNetwork::StateView`.
The referenced data remains valid only for the duration of the callback and
must not be retained. The views avoid copying the diffusion grid at every step.
An observer can still update other simulator objects or schedule calendar
events. Such changes made inside one notification are visible to subsequent
continuous steps in that same lookahead block.

Observers register a member method with
`addInternalStateEventHandler(observer, &Observer::handle, priority)` and
remove it by the returned ID with `removeInternalStateEventHandler(id)`.
Lower numeric priorities run first; equal priorities use registration order.
Duplicate registrations are separate subscriptions. Removal during dispatch
takes effect immediately for later callbacks in the same notification; new
registrations begin with the next step. A weak lifetime token skips observers
whose `ModelDataDefinition` has begun destruction. Observer callbacks are
independent of component entity dispatch and completion callbacks. Each
successful full or partial step notifies once, including when
`StepsToSynchronize=0`; that value still creates no synchronization event.
An exception from a handler is recorded, remaining active handlers for that
notification still run, and the continuous execution then fails and removes its
pending synchronization event. Cancellation from a handler removes the pending
event and notifies completion subscribers.

The work state is calculated ahead of ordinary calendar events in the same
synchronization block. An ordinary event that mutates inputs used by that
continuous calculation can therefore invalidate already calculated work. The
current implementation does not provide rollback or retract discrete events
that an observer already scheduled from such work. Models that require discrete
events to alter an active continuous trajectory need an explicit invalidation
policy before relying on that interaction. The selected `HUM-SCI-002` policy
does not make the working state visible through ordinary getters.

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

`ContinuousSystemComponent` retains its synchronous reactive request path and the
shared entity completion/forwarding behavior. During an already calculated block,
a request at an intermediate kernel time returns the confirmed state; it does not
re-integrate the working state or advance beyond the pending synchronization
boundary. The calendar callback commits the state at the boundary before the next
block starts.

## Component time-window compatibility

`DiffusionSimulate` and `BioSimulate` still persist their legacy `StartTime`,
`StopTime`, and `StepSize` controls. When an override window is requested, the
supported local origin is zero. A nonzero component start is rejected during model
or request validation rather than being interpreted as an absolute global time. Field/network window
selection remains available for existing serialized models.

## Evidence and limits

The real-calendar integration regression in
`source/tests/unit/test_plugins_continuous_diffusion.cpp` uses a `ModelComponent`
observer at `T0=100`, confirms an observer-generated entity event is processed at
103, and verifies ordinary events at 102 and 103 read the state last confirmed at
100. The same file covers typed observers for ODE, diffusion, and biochemical
states, subscription removal during dispatch, newly registered observers,
destroyed observers, and publication at a synchronization boundary. Existing tests
in `source/tests/unit/test_simulator_runtime.cpp` cover asynchronous entity
completion and multiple entities waiting on a `BioSimulate` run.

These tests cover specific execution paths. They do not prove all numerical
solvers scientifically valid or establish that all historical model files with
nonzero component `StartTime` remain semantically compatible. Interactions where
an ordinary discrete event changes an input used by an already calculated block
remain unsupported and require an invalidation policy.
