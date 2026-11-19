---
document_type: implementation-handoff
authority: remote-code-inspection-only
owner: ai-assistant
date: 2026-11-19
status: implementation-unverified
tracks: GUI-010 GUI-011 GUI-012 GUI-013 GUI-014
integration_branch: WorkInProgress
implementation_branch: WiP20261119/modal-network-editor-gui10-14
base_commit: 6c57631cfcfa881f3fd57d411de7099ae0bb4d99
---

# Modal / Network GUI remote implementation — local agent handoff


## 2026-11-19 follow-up — EFSM semantics without further kernel architecture changes

The frozen-architecture stabilization work in this branch was based on commit
b958cbc6ab5824a01980c942b0296e4d47af0883. No generic DefaultNetwork,
NetworkActivationFrame or global kernel contracts were modified in this phase.

Changes committed and awaiting compilation:

- EFSMTransition::canFire is now a guard-only finite-value predicate.
- Input Event and Trigger Event require presence of the exact named input ports;
  zero-valued present events count. Invalid names fail EFSMNetwork validation.
- Probability Expression is a reproducible, per-transition Bernoulli
  eligibility gate after event and guard checks. Runtime values must be
  finite and in [0,1]. This is not a row-normalized or categorical choice.
- Every external reaction considers immediate and non-immediate outgoing
  transitions together; same-call microsteps consider immediate only.
  Previously disabled immediate guards remain eligible on future activations.
- Transition action lists reject duplicate Variable/output destinations.
- Termination/Error transitions selected after postfire run source exit
  actions; an Error handler for postfire failure can publish recovery outputs.
- FSMState cached non-owning refinement references are validated against
  ModelDataManager, can follow live renames, and clear outdated associations
  on field reload.
- EFSMNetwork validation rejects transitions with foreign endpoints.
- EFSM graphical properties display read-only semantic guidance.
- New unit and Qt tests cover events, probabilities, RNG consumption,
  immediate selection precedence, postfire recovery, duplicate actions,
  and Network relation integrity.

**Evidence:** source commits only. No local CMake/Ninja build, CTest execution,
Qt/Xvfb testing, sanitizer testing or complete GEN/GUI save/reload test
was performed for this follow-up. All execution checks remain UNKNOWN.
There is no PR, merge or change to WorkInProgress.

Independent local agent gates include compilation of gui-app and tests-unit,
running relevant tests-kernel-unit, gui and smoke suites, statistical
reproducibility checks and sanitizers on refinement/undo paths.
Review the source-backed semantic specification at
docs/ai_assistants/reference/EFSM_REFINEMENT_RUNTIME.md, especially
differences from full Ptolemy II and incomplete rollback of side effects.



## Update — experimental EFSM refinement semantics

This update supersedes the previous statements below that `Immediate`,
`Preemptive`, `History`, `Error`, and `Termination` were unimplemented.

All five qualifiers are now implemented in the **single-activation discrete-event
subset of the GenESyS kernel** and exposed through GUI properties, persistence
and undo/redo. There is a typed, non-owning, selectable `Refinement Network`
reference on `FSMState` and a virtual refinement lifecycle on `DefaultNetwork`:
`initializeRefinement()`, `fireRefinement()`, `postfireRefinement()`.

The implementation now includes staged preemptive/non-preemptive transition
selection, history-controlled reinitialization, immediate microstep chaining
with a configurable limit, exception-driven Error transitions during refinement
fire/postfire, and postfire-driven Termination transitions. Both static
cycle validation and a runtime synchronous activation recursion guard were added.
Nested EFSM refinements can report termination on final states. A dedicated
technical contract with explicit Ptolemy II differences is located at
`docs/ai_assistants/reference/EFSM_REFINEMENT_RUNTIME.md`.

**No compiled/runtime evidence exists for this implementation in the remote
branch.** Kernel and GUI regression tests have been authored but not run.
All newly introduced code must undergo compilation, CTest, Xvfb interaction
and sanitizer tests before the implementation can be marked PASS.
The full Ptolemy II actor/director model is **not equivalent** to this bounded
GenESyS discrete-event implementation: fixed-point fire/postfire rollback,
arbitrary actor/transition refinements, heterogeneous event tokens, and
string/object-valued error scope are still beyond the verified backend contract.
A truthful claim is therefore "semantic discrete-event subset implemented,
unverified", not "full Ptolemy II compatibility".

No PR, merge, or push to `WorkInProgress` is part of this remote stage.



## Update after timeout recovery — revised remote implementation inventory

This section supersedes the narrower scope descriptions below. It records
source changes **committed through GitHub**, not build/test results.

- GUI-010 now has the reusable header-only `NetworkBendGraphicsItem` and
  `NetworkBendDiagramCodec`. Graph/DAG, Markov, and CPN extensions use
  bendable relations, independent graphical .gui route sections, deferred
  undo commands, retained selection across projection rebuilds, and route
  keys combining serialized relation ordinal with source/destination/name.
  EFSM continues using its independent v1 diagram codec. Antiparallel
  relations and multiple self-loops are separated by auto-routing.
  The Markov probability and CPN inscription labels are owned children of
  their relation item, so they follow a dragged bend. Existing tests
  asserting overlay counts were updated for these child labels.
  **All tests are authored only, not executed.**
- GUI-011 now includes genuine `Default` and `Nondeterministic` selection
  support; `Output Actions` writes named numeric output ports, and
  `Set Actions` calculates then writes existing model Variables. These
  actions accept a deliberately restricted `name=expression;...` grammar,
  not the full Ptolemy II action language. Properties have `.gen`
  persistence, GUI editing and undo/redo, plus authored kernel tests.
  Ptolemy `immediate`, `preemptive`, `history`, `error`, and
  `termination` qualifiers remain **unimplemented** because their true
  behavior requires refinement/activation lifecycle contracts not present
  in the verified kernel. Do not expose inert flags or claim equivalence.
- GUI-012 includes initial/final markers and fixes that preserve the
  authoritative initial-state identity across unrelated property edits,
  undo/redo, and persistence load.
- GUI-013 has a structured initial marking dialog, current marking
  read-only inspection, undoable reset, input-token and output-capacity
  readiness diagnostics, and arc inscriptions displayed with each curve.
  The readiness preview deliberately **does not evaluate guards**, to
  avoid consuming RNG or causing parser side effects on repaint. Full
  typed CPN colors/token inscriptions remain outside the current kernel.
- GUI-014 has canonical dot-decimal QDoubleSpinBox editing, bounded
  probabilities with high precision, immediate spin-button commits,
  elimination of a `QtDoubleEdit` integer precision/overflow bug,
  probability row-sum overlays, and fraction-preserving property labels.
  CPN/Markov visual colors are synchronized with the `pen()` of the
  underlying item so legacy selection/highlight tests remain meaningful.

**Critical local verification gates**: compile `gui-app` and
`tests-unit`; execute Qt/Xvfb tests and kernel suites; test actual
`.gui` save/reload (not just codec serialization), rotation and movement
of nodes with manual bends, undo/redo and mouse handling under ASan/UBSan;
check explicit colored CPN firing and Markov numerical editing under
`pt_BR`/`C` locales. The connector returned no pull-request-triggered
CI run for the remote head. Preserve the isolated branch; no PR/merge
in this cycle. Inspect the bounded probability editor's asynchronous
lifetime and ensure that the queued undo commits do not target stale
network relations.

The earlier sections of this file are retained as the chronological
history of the first remote implementation stage.



**Evidence classification: code changes committed remotely, NOT compiled or executed.**
The GitHub-only development environment cannot verify CMake/Ninja builds, CTest,
Xvfb interaction, ASan/UBSan, or simulation behavior. No PR or merge is authorized
in this work cycle. Do not report these changes as `done_confirmed`.

## Implemented changes and actual scope

- **GUI-010 — partial.** `EFSMTransitionGraphicsItem` now exposes an interactive
  midpoint bend handle, quadratic Bézier routing, deferred undo command, retained
  selection across projection rebuilds, and `.gui` route capture/restore via the
  pre-existing `EFSMNetworkDiagramLayout::TransitionLayout::controlPoints` schema.
  The existing auto-routing of parallel EFSM relations remains. **Graph, DTMC/Markov
  and CPN relation handles and their diagram persistence are NOT implemented.**
  The mouse/undo/selection lifecycle requires focused Qt GUI testing.
- **GUI-011 — partial.** A genuine `EFSMTransition::isDefaultTransition()` fallback
  flag, editable GUI checkbox, undo/redo, `.gen` persistence and backend activation
  preference have been added. Existing non-default enabled transitions take precedence;
  otherwise enabled defaults follow the existing network conflict policy.
  **Ptolemy-equivalent `nondeterministic`, `immediate`, `preemptive`,
  `history`, `error`, `termination`, `outputActions`, and `setActions`
  are NOT implemented.** These require a researched refinement/event lifecycle,
  scheduling and action-phase design. Avoid adding inert checkboxes or claiming
  Ptolemy compatibility.
- **GUI-012 — implementation awaiting tests.** Separate non-interactive markers
  distinguish initial and final EFSM states; current-state coloring is preserved.
  Projection refresh preserves relation overlays separately.
- **GUI-013 — partial implementation awaiting tests.** CPN has a table-based
  initial-marking editor (color/count, add/remove, validation), an undoable
  current-marking reset to initial marking, read-only current-marking and
  transition-enabled properties. Initial and current marking deliberately remain
  distinct. Detailed disabled-transition reasons and full in-canvas editing
  are NOT implemented.
- **GUI-014 — implementation awaiting tests.** The probability editor uses
  canonical dot-decimal `QLocale::c()` and rejects grouping; spin buttons submit
  values directly without duplicate focus-loss commits; values are bounded
  to [0,1] with 12 decimal places. `QtDoubleEdit` had an independently observed
  `int(pow(10, decimals))` overflow and an `int`-returning value accessor, both
  corrected. Markov projection displays live outgoing-row sums with error feedback.
  Entering `0,25` is NOT a supported synonym in the canonical editor; deliberate
  behavior under pt_BR must be reviewed. No automatic row normalization.

All changes target the Qt GUI extensions / selected EFSM backend only. No dynamic
plugin overhaul, full Ptolemy refinement system, Markov numerical kernel change,
CPN token type-system migration, or PR/merge has been performed.

## Proposed source-backed regression coverage (new tests are unexecuted)

- `test_gui_efsm_diagram_persistence.cpp`:
  `EFSMNetworkProjection.InitialAndFinalStatesUseDistinctNonInteractiveMarkers`,
  `EFSMNetworkProjection.ManualBendPointsSurviveProjectionRefreshAndLayoutRestore`,
  `MarkovChainNetworkGuiExtension.DecimalProbabilityEditorPreservesFractionUnderBrazilianLocale`,
  `MarkovChainNetworkGuiExtension.SpinButtonCommitsProbabilityWithoutLosingFraction`,
  `MarkovChainNetworkGuiExtension.ProbabilityRowSumsFollowKernelEdits`,
  `ColoredPetriNetGuiExtension.StructuredMarkingEditorAndResetAreUndoable`.
- `test_efsm_network.cpp`:
  `EFSMNetworkTest.DefaultTransitionOnlyFiresWhenOrdinaryRelationsAreDisabled`
  and extended existing transition persistence assertions.

## Mandatory local verification

1. Read the canonical AI assistant README, governance, architecture, current
   status/backlogs and local runbook before working.
2. Fetch `origin/WorkInProgress` and this branch; preserve other agents'
   worktrees and branches. Check for concurrent merges and review conflicts.
3. Inspect the full diff adversarially, especially `ObjectPropertyBrowser`
   asynchronous commit lifetimes, `QtDoubleEdit` numerical behavior,
   `EFSMTransitionGraphicsItem` mouse event/undo lifetime, and `CPN` dialog.
4. Configure and build actual project presets, at minimum:
   `cmake --preset gui-app`, `cmake --build --preset gui-app`,
   `cmake --preset tests-unit`, `cmake --build --preset tests-unit`,
   `ctest --preset tests-unit --output-on-failure`.
   Also run kernel, GUI and smoke suites as required by current governance.
5. Exercise the GUI in a real offscreen/Xvfb or desktop Qt event loop:
   EFSM parallel edges and bend/reselect/undo/save/load; initial/final indicators;
   CPN initial versus current multicolor markings, reset, guard/enabled/firing;
   Markov decimal typing, spin arrows, undo, row sums, save/reload and valid DTMC
   execution. Use ASan/UBSan on item-deletion/undo paths if available.
6. Verify `.gen` remains mathematical source of truth, `.gui` contains only
   layout, legacy/current model files load, shared Networks remain consistent.
7. **Finish remaining GUI-010/011/013 functionality in the same branch** only
   after identifying/test-driving the missing semantics. Do not silently weaken
   requirements or mark incomplete work PASS.
8. Keep the branch unmerged until local evidence, review and required hosted
   checks pass. A later user instruction will authorize PR/merge.

## Known risks / stop gates

- There is **no compiled or executed evidence** on this branch as of this record;
  test names above demonstrate authored checks, not successful execution.
- The EFSM geometry drag handler schedules the undo command after the event
  to avoid destroying its own QGraphicsItem during synchronous overlay refresh;
  confirm pointer validity and last-selection persistence in real Qt.
- `QtDoubleEdit` is a QWidget wrapping QDoubleSpinBox. Confirm locale settings
  apply to the child and numeric callbacks apply exactly once.
- The current EFSM engine ignores parts of `NetworkActivationFrame` and lacks
  full Ptolemy refinement lifecycle; qualified transition flags are a genuine
  architectural extension, not a cosmetic property-editor task.
- An optional legacy `Probability Expression` field in EFSM does not perform
  probabilistic choice (current implementation tests positivity). Do not conflate
  this with DTMC transition probabilities.

**Acceptance:** all five problem scopes PASS in real builds/tests and GUI use,
then and only then consider PR and merge into `WorkInProgress`.
