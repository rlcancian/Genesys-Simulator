---
document_type: technical-reference
authority: informative
owner: project-maintainer
last_reviewed: 2026-10-30
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
`WaitingForSynchronization`, `WaitingForContinuation`, `Completed`, `Failed`, or `Cancelled`). Completion
handlers receive an `ExecutionResult`; callbacks are removed when a component's
request has finished or when the component is destroyed. Replication initialization
cancels pending internal events and notifies outstanding consumers before resetting
the definition.

If a request is rejected before any entity is attached, both the completion and
continuation subscriptions created for that request are removed immediately. This
prevents a later calendar interruption from calling a continuation lambda whose
component has already been destroyed.

## Confirmed state and per-step observation

The maintainer's 2026-10-29 decision for `HUM-SCI-002` in this execution path is
that ordinary discrete events read the latest state confirmed at a global
synchronization. Continuous observers receive each successfully calculated
working state, including steps between synchronization boundaries. The kernel
clock remains owned by `ModelSimulation`; creating an observer notification does
not advance it.

When `StepsToSynchronize` is positive, the continuous definition calculates the
next nominal synchronization boundary, then bounds it by the first later event
already in the calendar. It skips its own queued synchronization event when
finding that bound. A step is calculated only if its endpoint does not pass the
event. For `T0=100`, `StepSize=1`, and a nominal boundary at 110, an event at
105.5 limits work to local time 5 / absolute time 105. The state is confirmed
when the internal synchronization event is dispatched at 105. The same run is
then suspended and resumed by a conventional event carrying the waiting entity
back to the same continuous component at 106. The component recognizes that
event through the reserved runtime input port
`ContinuousModelComponent::CONTINUATION_INPUT_PORT`; it does not issue a new
simulation request or reinitialize the mathematical state. An event at 100.5
allows no complete step before it, so the model wakes at the next grid endpoint.

The calendar is rechecked after each successful step, including when no
observers are registered. Thus events inserted by observers are also detected.
The post-step check treats only events strictly later than the kernel's current
global time as future bounds. Events already at or behind that clock, including
same-timestamp events that the calendar has not ordered yet, do not trigger a
false "event in the past" failure. The event comparator and tie behavior are
unchanged.
If a handler inserts an event before the already calculated working time, the
run fails with an explicit diagnostic; this implementation does not roll back
mathematical state or arbitrary handler side effects. No tie-break rule was
added for equal timestamps. A discrete event exactly at a synchronization time
remains simultaneous under the existing calendar comparator, so tests and
clients must not rely on either event being dispatched first.

For an event between grid points, integration remains on the configured grid.
The next step after the discrete event may span the interval containing that
discontinuity; no fractional step or automatic solver restart at the
discontinuity is introduced. `StepsToSynchronize=0` remains synchronous and
unbounded by the calendar: it does not yield to intermediate discrete events.

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

The working state can be ahead of the kernel only up to the last allowed grid
point before the next calendar event. An ordinary event that mutates inputs at
that time can change the trajectory after the event; the following continuous
step uses the updated values. The step that crosses a non-grid event uses the
existing solver's ordinary step method as described above. The selected
`HUM-SCI-002` policy keeps ordinary getters on the last confirmed state while
observers receive the working state.

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

When a calendar event interrupts a component-owned run, the data definition
enters `WaitingForContinuation` and asks the component to return one retained
entity to itself at the next grid endpoint. Other entities joined to that run
remain retained and are forwarded once, with the returning entity, only after
the full requested `StopTime` is reached. Completion and cancellation remove a
queued continuation event belonging to that component. Autonomous definitions
without a waiting component use an internal continuation event instead.
Automatic evolution with a positive synchronization count is started through a
zero-time internal event so replication-start handlers can populate the
calendar before the first lookahead is calculated.

`ContinuousSystemComponent` submits the ODESolver's definition time window to
the same asynchronous execution lifecycle used by `DiffusionSimulate` and
`BioSimulate`. The component retains arriving entities while the definition is
running or suspended, and forwards them only after the requested `StopTime` is
completed. A calendar interruption returns one retained entity to the component
at the next permitted grid point; that event resumes the existing ODE execution
without resetting the mathematical state. The standalone
`ODESolver::integrate(targetTime)` API remains available for explicit reactive
integration callers; it is not used by the entity-driven component lifecycle.
During model consistency checking, `ODESolver` records each model-data reference
found in its equation expressions as an attached dependency. This prevents a
shared parameter variable from being pruned as an orphan before checking and
keeps it available when the equation is evaluated after a calendar interruption.
For ODEs, `MaxSteps` bounds an integration batch. The shared asynchronous driver
starts a fresh ODE step budget for each synchronization batch, matching the
budget previously provided by a direct `integrate(targetTime)` call per batch.
With `StepsToSynchronize=0`, the synchronous run remains one batch.

`BioNetwork` refreshes its response payload after the base execution state is
changed to `Completed`. Thus `getLastStatus()` and the terminal payload agree
for both synchronous and calendar-driven runs; intermediate working payloads
remain associated with their step state.

## Component time-window compatibility

`DiffusionSimulate` and `BioSimulate` still persist their legacy `StartTime`,
`StopTime`, and `StepSize` controls. When an override window is requested, the
supported local origin is zero. A nonzero component start is rejected during model
or request validation rather than being interpreted as an absolute global time. Field/network window
selection remains available for existing serialized models.

## Evidence and limits

The real-kernel regressions in
`source/tests/unit/test_plugins_continuous_diffusion.cpp` confirm that ODE and
BioNetwork component runs stop at the grid boundary before an event at 5.5,
expose the state confirmed at 5 to the ordinary discrete event, resume at 6,
and forward waiting entities only after local time 10. The BioNetwork case also
checks that two entities share one run and that A-to-B mass is conserved. The
ODE and concrete DiffusionSimulate component cases also check two entities
sharing one run across calendar interruption; each original ID is forwarded
exactly once at completion. ODE component cases cover a constant derivative
and a shared parameter variable changed by a discrete event: rate 1 yields
state 5 at the event at 5.5; after the event changes the rate to 2, the run
resumes and finishes at state 15. Other tests
verify observer-generated events and typed observers for ODE, diffusion, and biochemical
states, subscription removal during dispatch, newly registered observers,
destroyed observers, and publication at a synchronization boundary. Existing tests
in `source/tests/unit/test_simulator_runtime.cpp` cover asynchronous entity
completion and multiple entities waiting on a `BioSimulate` run.

The calendar regressions also exercise an ODE with `MaxSteps=2` over ten
one-step synchronization batches, and BioNetwork verifies that the final JSON
payload says `Completed`. `DiffusionSimulate` rejects a nonzero local
`StartTime` when `UseFieldTimeWindow=false`; the rejection is explicit during
component validation and has a regression test. This retains the zero-origin
contract rather than accepting a configuration that the data definition cannot
execute.

The implementation currently scans the ordered event list after every step;
the optimization to avoid those checks when no observer or event-producing hook
exists has not been applied. These tests cover specific execution paths. They do
not prove all numerical solvers scientifically valid or establish that all
historical model files with nonzero component `StartTime` remain semantically
compatible. `StepsToSynchronize=0` intentionally cannot interleave discrete
events, and a step spanning a non-grid discontinuity remains an approximation.
