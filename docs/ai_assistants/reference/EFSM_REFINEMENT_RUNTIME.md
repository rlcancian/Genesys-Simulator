# EFSM refinement lifecycle and modal transition qualifiers

**State:** remote implementation awaiting build, tests and desktop validation.  
**Branch:** `WiP20261119/modal-network-editor-gui10-14`.  
**Integration:** not merged into `WorkInProgress`.  
**Contract:** current GenESyS numerical, discrete-event network activation, not strict Ptolemy II fixed-point execution.

## Source contracts

- `source/plugins/data/ModalModel/DefaultNetwork.{h,cpp}`: public `initializeRefinement()`, `fireRefinement(frame)`, `postfireRefinement()` hooks. The default adapter initializes a network using its replication-reset entry point, executes a single `activate()`, and reports that it can continue in postfire. Subclasses can override these phases.
- `source/plugins/components/ModalModel/FSMState.{h,cpp}`: optional `Refinement Network` property, referencing an **existing**, model-owned `DefaultNetwork`. No implicit creation or ownership transfer; typed classname/name persistence and ModelDataManager lookup. A refinement may be shared, but reinitialization/historical state remains owned by the referenced Network.
- `source/plugins/data/ModalModel/EFSMNetwork.h` and `source/plugins/components/ModalModel/EFSMNetwork.cpp`: phase ordering, state transitions, immediate microsteps, choice policy, refinement lifecycle, output/set actions, error/termination outcomes, cycle validation.
  The EFSM Property Editor additionally exposes `Max Immediate Transitions Per Activation` (positive integer) and a read-only `Last Refinement Error` message.
- `source/plugins/components/ModalModel/DefaultTransitionExtensions.{h,cpp}`: transition flags `Default`, `Nondeterministic`, `Immediate`, `Preemptive`, `History`, `Error`, `Termination`.
- `source/applications/gui/genesys/extensions/plugins/efsm/EFSMGuiExtensionPlugin.cpp` and transition properties undo command: editable booleans with undo/redo.

## Execution semantics in the current discrete-event subset

For a single call to `DefaultNetwork::activate(frame)` of an EFSM:

1. Start with its current state, initializing the state refinement if needed. Input ports are forwarded to the refinement by matching **port names**, preserving presence/value semantics.
2. On state entry, evaluate enabled **immediate preemptive** transitions before any non-immediate preemptive ones. On later activations, only ordinary preemptive transitions are eligible; immediate transitions are entry-only.
3. If no preemptive transition is chosen, call `fireRefinement(frame)` on the active state's refinement, if any. Its present outputs are forwarded to same-named EFSM output ports. An exception during fire is handled only by a matching `Error` transition, or otherwise propagates to the caller.
4. After a successful refinement fire, evaluate immediate non-preemptive (when newly entered), then ordinary non-preemptive transitions. Default transitions serve as fallbacks within each selection phase.
5. For a chosen ordinary transition, execute exit and output actions, invoke the fired refinement's `postfireRefinement()`, evaluate and commit Set Actions, initialize destination refinement unless `History` applies, then change the EFSM state and evaluate entry action. The restricted action language is `destination=expression;...` with numeric named output ports or existing Variables.
6. If no ordinary transition is selected, a false `postfireRefinement()` result makes `Termination` transitions eligible **after** ordinary/default ones. Output actions on termination transitions are invalid because postfire is too late to produce them. If the refinement postfire throws, an eligible `Error` transition may recover; absent a handler, the exception propagates.
7. After taking a transition, the newly entered state may take zero-time immediate transitions in the same `activate()`. Ordinary transitions cannot run a second time merely because a new state was entered. A configurable default budget of 64 immediate microsteps guards against endless cycles.
8. `History` prevents reinitialization of a previously initialized destination refinement. A non-history entry explicitly initializes it. On replication reset the initialization registry and last refinement error are cleared.
9. The parent EFSM's activation counter represents **one** external activation, even when immediate internal microsteps occur. Each nested refinement has its own counter.
10. Recursive synchronous activation of a Network is rejected at the `DefaultNetwork::activate()` entry point using an exception-safe per-thread activation guard; validation also detects refinement dependency cycles.

Transitions in the same selection phase use the existing conflict policy (`MODEL_ERROR`, deterministic priority or nondeterministic choice). If all enabled transitions permit nondeterminism, a reproducible sampler can select one. A `MODEL_ERROR` conflict does not silently fall through to a different phase.

## Qualifier-specific invariants

| Flag | Required source/refinement conditions |
|---|---|
| Immediate | Entry-only transition; may chain in one dispatch, subject to configured bound |
| Preemptive | Guard evaluated before refinement fire; source refinement is not fired when transition is selected |
| History | Previously initialized destination refinement continues without resetting |
| Error | Requires source refinement; only enabled by actual exception from refinement fire/postfire. Cannot also be immediate, preemptive or termination |
| Termination | Requires source refinement; enabled only after refinement postfire returns false and no ordinary/default transition wins. Cannot be immediate/preemptive or produce output actions |

The runtime exposes `getLastRefinementError()` to inspect the last thrown error message. No exception is treated as ordinary success. A handled postfire failure does not commit the earlier selected normal transition.

## Legacy events, stochastic gating and transition action diagnostics

These clarifications describe the current **GenESyS discrete-event subset**, not
new architectural services and not the Ptolemy II general token/event scope.

- `Input Event` and `Trigger Event` are optional exact names of existing
  EFSM **input ports**. Every configured name must be present in the current
  `NetworkActivationFrame`; a present value of zero still satisfies the
  condition. When both fields are configured, both presences are required.
  Unknown names fail `EFSMNetwork::_check()`.
- The short-circuit order is **input-port presence → guard expression →
  Bernoulli probability gate**. Thus absent events do not evaluate stochastic
  guards and a false guard does not consume RNG from the probability gate.
- `Probability Expression` is a **per-transition probability of eligibility**,
  not a categorical distribution across outgoing relations. Its runtime value
  must be finite and in `[0,1]`; `0` always rejects and `1` always admits
  without drawing RNG. Interior probabilities use the EFSM sampler. Dynamic
  probabilities are validated **at activation time**; static `_check()`
  validates expression syntax only to avoid consuming RNG during validation.
  Once Bernoulli-gated relations have been identified, the preexisting
  deterministic-priority / model-error / nondeterministic conflict policy chooses
  a relation. With no enabled ordinary relation, `Default` candidates are
  considered and gated independently.
- The runtime action subset remains `name=expression;...`. Duplicate output
  port or Variable destinations in the same action list are rejected instead
  of relying on ambiguous last-write-wins behavior. `Output Actions` are
  evaluated before `Set Actions`; assignment right-hand sides of `Set Actions`
  are evaluated before updating any destination Variable.
- For `Error` recovery after a failed refinement `postfire`, recovery output
  expressions/actions can still be emitted and the source state's exit action
  is evaluated. Termination transitions also execute exit actions, but their
  output actions remain invalid under `_check()`.
- On **every external activation**, immediate and non-immediate transitions
  from the current state participate in their common preemptive or
  non-preemptive phase, including default fallback and conflict priority.
  Only within microsteps after a transition in the same activation are
  candidates restricted to immediate transitions. Consequently, an immediate
  transition whose guard was initially false remains eligible on a later
  external activation.
- Graphical transition properties provide a read-only explanation of the
  event and Bernoulli semantics; primary validation remains in `EFSMNetwork`.
- `FSMState::getRefinementNetwork()` checks the live ModelDataManager registry
  before using a cached non-owning reference. A still-live target remains
  associated when renamed; persistence can resolve the typed name to a
  reloaded target. Loading new association fields detaches the previous
  association without deleting the referenced Network.

**Irreversibility limitation:** the default child `fireRefinement()` executes an
atomic `activate()`, and legacy expression evaluation may itself have side
effects or consume randomness. If a later `postfireRefinement()` throws, the
parent retracts uncommitted output values and may run an `Error` transition,
but it cannot rollback already committed child state or arbitrary parser side
effects. No transactional guarantee should be inferred or advertised.

## Ptolemy II equivalence boundary — NOT COMPLETE

The Ptolemy II `FSMDirector` implements an actor-level `fire()` / `postfire()` protocol with tentative choice actions, state/transition refinements, refinement-produced input visibility and fixed-point compatibility. GenESyS currently has a **single atomic, committed `activate()`** and an additional optional refinement lifecycle. Therefore:

- The default `fireRefinement()` delegates to `activate()`, so the child may **commit state during fire**. It cannot be rolled back to implement arbitrary fixed-point semantics.
- Nested `DefaultNetwork` refinements are supported, but full arbitrary Ptolemy actors, transition refinements, multirate actors and concurrent director scheduling are not implemented.
- Error transitions catch C++ exceptions from refinement lifecycle operations. Some GenESyS errors are logged rather than thrown; these do not automatically become Ptolemy model-error events. `errorMessage`, `errorClass` and `errorCause` have no full parser string/object-token binding yet.
- Input/output ports carry optional **double** values, not Ptolemy's general tokens/events/channels. `Trigger Event` and `Input Event` legacy fields are not a Ptolemy event scope.
- Termination output actions are rejected in the current atomic API. Immediate microsteps have a configured hard bound; exceeding it raises an error after the already committed microsteps.
- Only one referenced refinement Network per state is supported; multi-refinement firing order and postfire aggregation are not.
- This implementation must **not** be claimed as full Ptolemy II behavioral equivalence without the above capabilities and independent conformance tests.

Official references:
- https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII11.0.1/doc/codeDoc/ptolemy/domains/modal/kernel/Transition.html
- https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII11.0.1/doc/codeDoc/ptolemy/domains/modal/kernel/FSMDirector.html
- https://ptolemy.berkeley.edu/books/Systems/chapters/ModalModels.pdf

## Validation gates (no PASS without execution)

- Build with actual CMake/Ninja presets (`gui-app`, `tests-unit`, `tests-kernel-unit`), verify no source/generated out-of-sync or ABI/visibility issues.
- Execute `source/tests/unit/test_efsm_network.cpp` including the new `EFSMNetworkLifecycle.*` tests. Also run GUI property/undo tests in `test_gui_efsm_diagram_persistence.cpp`.
- Test manual state refinement association and removal through the Qt Property Editor, no DataDefinition creation, save/reopen real `.gen`/`.gui` model and an actual `ModalModel` entity flow.
- Test nested EFSM termination and history preservation across more than one external activation; test error behavior in fire and postfire, unhandled exceptions, `MODEL_ERROR` conflict policy and immediate loop protection.
- Run focused ASan/UBSan cases on network reference deletion, state/transition undo, and nested refinement failure recovery.
- Validate that original GUI-010—GUI-014 behaviors and unrelated Network specializations have not regressed.
- Any failed build, failing test or unverified critical semantics blocks PR/merge.

**Evidence classification:** code has been committed remotely. No local build, CTest execution, Xvfb GUI run or sanitizer execution has occurred as part of this work.
