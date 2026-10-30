---
document_type: evidence-report
authority: informative
owner: project-maintainer
period: 2026-10
status: active
tracks: 550
---

# GenESyS Continuous Models Observable — Implementation Handoff

Status: `BLOCKED_BY_ARCHITECTURAL_DECISION`.
Evidence labels used below: **Confirmed in code**, **Confirmed by execution**,
**Strong indication**, and **Hypothesis to validate**.

## 1. Escopo analisado

This change extends PR #550 on `WiP20261016/ContinuousModels`. It implements
per-step observation for `ContinuousModelDataDefinition`, working/confirmed
state separation in `ODESolver`, `DiffusionField`, and `BioNetwork`, and preserves
the shared `ContinuousModelComponent` lifecycle. The `Failure` plugin is unchanged.

**Confirmed in code:** the maintainer's decision for the deterministic continuous
path is recorded as `HUM-SCI-002`: ordinary discrete events use confirmed state;
registered observers receive each successful working step; only
`ModelSimulation` advances the global clock; observers may schedule ordinary
calendar events. This does not settle stochastic CTMC holding-time semantics.

## 2. Diagnóstico inicial

**Confirmed in code:** before this change, synchronization blocks were evaluated
at calendar dispatch, so there was no callback for each mathematical step early
enough to schedule an event at an intermediate absolute time. The model classes
also exposed mutable species, variables, and field state directly, which would
have exposed lookahead values to ordinary discrete events if they were advanced
before the boundary.

**Confirmed in code:** `Resource` has a direct callback pattern, but its storage
uses raw component pointers and manual ownership. The new mechanism therefore
uses explicit subscription IDs, observer lifetime tokens, and dispatch-time
membership checks.

**Confirmed by execution:** the first full unit run exposed a replication-order
regression: `BioNetwork` could capture working amounts before `BioSpecies` reset
them. A replication-start hook now resets the network's referenced species before
continuous state initialization. The formerly failing two-replication regression
passes after this correction.

## 3. Evidências e problemas encontrados

**Confirmed in code:** callbacks need access to working state before calendar
dispatch while public getters must remain at the last confirmed boundary. The
continuous base now schedules the next boundary first and calculates/notifies its
steps synchronously through that boundary; the global clock remains unchanged.

**Confirmed by execution:** a real `ModelSimulation::start()` regression at
`T0=100` verifies observer notification at local step 3, scheduling and dispatch
of an ordinary event at 103, an ordinary event at 102 reading the state confirmed
at 100, and boundary confirmation at 105. The kernel processes the inserted event
at its calendar time; notification itself does not advance the clock.

**Confirmed by execution:** the focused continuous plugin executable reports
28/28 passing tests, including observer lifetime, removal/addition during dispatch,
exceptions, cancellation, ODE/diffusion/biochemical state publication, priorities,
duplicate registrations, partial final steps, and the real-calendar scenario.

**Confirmed by execution:** the latest complete `tests-unit` CTest run reports
1,928 passed, 0 failed, and 4 disabled (1,932 discovered). The earlier run before
the replication-order correction had one failure in the BioNetwork two-replication
test; that cause was fixed and the complete suite was rerun successfully.

**Confirmed by execution:** `tests-smoke` passes 3/3 after aligning the harmonic
oscillator smoke assertion with the selected confirmed-state contract. The old
demo compared the last state confirmed at local time 6.28 with the analytic
solution at `2*pi`; the event at `2*pi` is allowed to observe the prior boundary.
The test now compares against the analytic solution at the sampled confirmed
local time and asserts that the global-to-confirmed lag is bounded by one nominal
step. Numerical error tolerances were not changed.

**Confirmed by execution:** the GUI application built successfully. No
interactive GUI scenario was run. The developer manual PDF rebuilt successfully;
the figure-specification validator and AI documentation governance validator
passed. Doxygen generated its HTML/LaTeX sources, but its PDF build failed on a
pre-existing Unicode `Δ` in `CellGrowthComponent.h` (`pdflatex` reports that the
character is not configured). That file is outside this change and was not edited.

**Confirmed in code:** `StepsToSynchronize=0` remains a synchronous, out-of-calendar
run: it emits per-step observer callbacks, inserts no synchronization events, and
publishes its completed state before returning without advancing the global clock.
It cannot interleave ordinary calendar events during that local interval.

## 4. Impacto arquitetural

The public observer API lives in `ContinuousModelDataDefinition`; it is independent
of entity dispatch and completion callbacks. Both `ModelComponent` and other
`ModelDataDefinition` objects are accepted as observers because they derive from
`ModelDataDefinition`.

Each notification contains the source definition, monotonically increasing step
index, local time, absolute time (`T0 + localTime`), and a class-specific typed
borrowed view. The view is valid only for the duration of the callback; diffusion
grids are not copied at each step. Lower numeric priorities run first, with
registration order as the tie breaker. Duplicate subscriptions have distinct IDs.

## 5. Solução implementada

`ContinuousModelDataDefinition` stores subscriptions with IDs and weak lifetime
tokens. Dispatch snapshots IDs, rechecks the active set before every call, skips
expired observers, defers registrations made during dispatch until the next step,
and catches callback exceptions while still invoking remaining active handlers.
Removal during notification takes effect immediately for later handlers in that
notification. A callback exception fails the continuous run and removes its
pending synchronization event. Cancellation removes pending events and notifies
completion subscribers.

Each successful full or partial step updates local working time and notifies once.
At a positive synchronization boundary, the `InternalEvent` callback publishes
the working state and confirmed time before starting the next block. With zero
synchronization steps, the run publishes synchronously at completion.

`ODESolver`, `DiffusionField`, and `BioNetwork` expose typed working-state views
to observers and confirmed-state getters to ordinary readers. BioNetwork restores
species amounts at replication start before capturing its working vector. Existing
component completion and entity-forwarding behavior is retained.

## 6. Patches e commits

The implementation and focused tests were committed as
`f98319ff` (`feat(continuous): observe internal state at every step`). Documentation,
the handoff, ledger, and regenerated Developer Manual are being committed separately.
The pre-existing local formatting-only change in
`source/plugins/data/DiscreteProcessing/Resource.h` and untracked `teste1.gui` are
excluded from staging and commits.

## 7. Validação executada

| Check | Result |
|---|---|
| Focused continuous plugin tests | 28/28 passed |
| Full `tests-unit` | 1,928 passed, 4 disabled, 0 failed |
| `tests-kernel-unit` | 1,929 passed, 4 disabled, 0 failed (1,933 discovered) |
| `tests-smoke` | 3/3 passed |
| `gui-app` build | Passed, 303 build actions; no interactive exercise |
| Developer Manual | PDF regenerated; figure-spec and AI docs validators passed |
| Doxygen PDF | Failed on pre-existing Unicode `Δ` in `CellGrowthComponent.h` |
| ASan / UBSan / LSan | Focused test executable: 28/28 passed; no sanitizer diagnostics or leak report |
| PR checks after push | All reported checks passed for published SHA `d3462b987c2a492cd45031ce6d0b9e354a2f2666` |

The report records observed outcomes only. The final documentation commit, push,
and hosted checks are recorded after publication.

## 8. Riscos remanescentes

**Confirmed in code:** the complete working block is calculated before ordinary
calendar events inside that block are dispatched. If an ordinary event mutates an
input used by already calculated work, that work may be stale. There is no rollback,
replay, or retraction of observer-generated calendar events. Models requiring such
event-driven trajectory changes need a separate invalidation policy decision.

**Confirmed in code:** in zero-synchronization mode, the local interval runs
synchronously and cannot be interleaved with discrete events. Its final state is
published at the caller's current global time. Use a positive synchronization
count where calendar-time interaction is required.

**Hypothesis to validate:** all historical persisted models with nonzero component
`StartTime` remain semantically compatible; current validation rejects unsupported
nonzero local starts, while serialized fields remain readable.

## 9. Próximos passos

Resolve the scientific invalidation policy for discrete changes to an already
precomputed continuous block and for retracting observer-derived events. Then
revalidate the affected cases. Doxygen PDF generation also needs an established
Unicode-safe toolchain/configuration or correction of the pre-existing comments.
The sanitizer comparison against a `WorkInProgress` baseline was not run. After
publication, recheck hosted CI while keeping PR #550 in draft. Do not merge into
`WorkInProgress`.

## Apêndice técnico

### Identificação e estado Git

- Initial SHA: `ad2c261ec7878638a8812090599397cee78eed69`.
- Final validated implementation/documentation SHA: `d3462b987c2a492cd45031ce6d0b9e354a2f2666`.
- This report is updated in a separate evidence-only commit after that validation;
  the resulting branch HEAD is given in the final response.
- Branch: `WiP20261016/ContinuousModels`.
- PR: #550, existing draft PR; refresh exact state/head after push.
- `WorkInProgress` is not modified or merged.
- Implementation commit: `f98319ff`.
- Files changed: `source/kernel/simulator/model/ModelDataDefinition.cpp/.h`;
  `source/plugins/data/Continuous/ContinuousModelDataDefinition.cpp/.h`,
  `ODESolver.cpp/.h`, `DiffusionField.cpp/.h`;
  `source/plugins/data/BiochemicalSimulation/BioNetwork.cpp/.h`;
  `source/tests/unit/test_plugins_continuous_diffusion.cpp`;
  `source/tests/test_continuous_system.cpp`;
  `docs/ai_assistants/BACKLOG_HUMAN.md`, `STATUS.md`,
  `reference/README.md`, `reference/CONTINUOUS_MODEL_EXECUTION.md`,
  this handoff and `VALIDATION_LEDGER.md`;
  `docs/developers/ManualGenESyS_subrepo/chapter_continuous_modal_and_hybrid_simulation.tex`,
  its generated `.ptc`, and `docs/ManualGenESyS.pdf`.
- `source/plugins/data/DiscreteProcessing/Resource.h` and `teste1.gui` are
  pre-existing unrelated local changes and are not part of either commit.
- Hierarchy changed: none. `ContinuousModelDataDefinition` remains the temporal
  abstract base; `ContinuousModelComponent` remains the shared component base with
  its three concrete components.
- APIs added: typed `InternalStateEvent`, `InternalStateEventHandlerId`,
  `addInternalStateEventHandler` (callable and typed member overloads),
  `removeInternalStateEventHandler`, `stateAs<T>()`, typed state-view hooks, and
  `ModelDataDefinition::getLifetimeToken()` for safe observer liveness checks.
  Existing APIs removed: none.
- Persistence contracts changed: none. No new persisted fields or factories.
- New regression coverage: 10 Observable tests (ModelDataDefinition observer,
  ModelComponent observer scheduling a real intermediate kernel event, state
  visibility, ODE/diffusion/bio views, priority/duplicate handling, removal and
  registration during dispatch, destroyed observer, callback exception, and
  cancellation). Existing continuous-system smoke assertion now compares the
  analytic oscillator value at the last confirmed local time.

### API pública e exemplos

```cpp
auto id = continuous.addInternalStateEventHandler(
    &observer, &Observer::handle, priority);
continuous.removeInternalStateEventHandler(id);
```

`Observer` may derive from `ModelDataDefinition` or `ModelComponent`. A callback
receives `InternalStateEvent`; it can inspect `event.localTime`, `event.absoluteTime`,
`event.step`, and `event.stateAs<ODESolver::StateView>()` (or the view type for the
actual producer). A component observer can use its model's ordinary calendar APIs
to schedule an event at `event.absoluteTime` if that instant is still future to the
kernel. The real-calendar regression demonstrates this at 103 from an origin of 100.

### Temporal example

For `T0=100`, `StepSize=1`, `StopTime=10`, and
`StepsToSynchronize=5`, callbacks report `(local, absolute)` values `(1,101)` through
`(5,105)`. The boundary event is at 105. An ordinary event at 102 reads the state
confirmed at 100. An observer may insert an ordinary event at 103; the kernel later
dispatches it at 103. Boundary processing publishes step 5 and the next block starts.

### Validation commands and evidence

- Tool versions: GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1, CTest 3.28.3,
  Git 2.43.0, and GitHub CLI 2.45.0.
- Effective C++ standard: CMake configures C++23 (`CMAKE_CXX_STANDARD=23`,
  required, extensions off); compile commands use `-std=c++23`.
- Unit build/CTest logs: `/tmp/genesys-observable-unit-rerun-build.log` and
  `/tmp/genesys-observable-unit-rerun-ctest.log`.
- Focused plugin logs: `/tmp/genesys-observable-focused-final-build.log` and
  `/tmp/genesys-observable-focused-final-test.log`.
- Smoke logs: `/tmp/genesys-observable-smoke-final-build.log` and
  `/tmp/genesys-observable-smoke-final-ctest.log` (3/3 passed).
- Kernel logs: `/tmp/genesys-observable-kernel-config.log`,
  `/tmp/genesys-observable-kernel-build.log`, and
  `/tmp/genesys-observable-kernel-ctest.log` (CTest: 1,929 passed, 4 disabled,
  0 failed; 1,933 discovered).
- GUI build log: `/tmp/genesys-observable-gui-build.log` (303 actions, success).
- Developer Manual build log: `/tmp/genesys-observable-manual-build.log` (success;
  existing out-of-page annotation warnings in generated PDF).
- Figure-specification validator: `./validate_figure_specs.sh` (exit 0).
- AI docs validator: `python3 scripts/validate-ai-docs.py` (exit 0).
- Doxygen generation log was streamed; PDF log is
  `/tmp/genesys-observable-doxygen-pdf.log` (failed on existing Unicode delta).
- Sanitizer config/build/test logs:
  `/tmp/genesys-observable-asan-config.log`,
  `/tmp/genesys-observable-asan-build.log`, and
  `/tmp/genesys-observable-asan-test.log` (ASan+UBSan, LSan enabled; 28/28 pass,
  no diagnostics; no equivalent WIP baseline comparison).
- Test totals: unit 1,928 passed / 4 disabled / 0 failed (1,932 discovered);
  kernel 1,929 passed / 4 disabled / 0 failed (1,933 discovered); smoke 3/3;
  sanitizer-focused binary 28/28. No test was disabled by this change.
- New observable tests reside in
  `source/tests/unit/test_plugins_continuous_diffusion.cpp`.
- Persistence fields and concrete factories are unchanged by the Observable API;
  existing serialized component time-window fields remain supported subject to
  the local-origin validation noted above.

### Decisions and limitations

- **Confirmed in code:** deterministic continuous `HUM-SCI-002` decision recorded
  by the maintainer is implemented in this path.
- **Confirmed in code:** zero synchronization is synchronous and out-of-calendar.
- **Decision still required for broader interaction:** invalidate/replay already
  calculated work when an ordinary discrete event changes a continuous input, and
  retract/recompute any observer-generated events derived from that work.
- **Status rationale:** this unresolved invalidation behavior is scientifically
  material under the authorized handler freedom. The implementation does not
  silently invent rollback or event retraction, so the result is
  `BLOCKED_BY_ARCHITECTURAL_DECISION` for those cases; the independent Observable
  and deterministic calendar path is implemented and tested.
- **Separate future decision:** reconcile stochastic CTMC holding times with the
  global event calendar; it is outside this deterministic Observable change.

### Publication

- **Confirmed by execution:** all checks reported by `gh pr checks 550` passed
  for validated SHA `d3462b987c2a492cd45031ce6d0b9e354a2f2666`, including unit,
  kernel, smoke, GUI application, worker, shell, optimizer, AI Assistant, Data
  Analyser, Debian package build/lifecycle, archive/link inspection, plugin target
  graph, documentation governance, launcher, and GUI diagnostics.
- PR #550 remains open, draft, and mergeable, targeting `WorkInProgress`. No merge
  was authorized or performed.
