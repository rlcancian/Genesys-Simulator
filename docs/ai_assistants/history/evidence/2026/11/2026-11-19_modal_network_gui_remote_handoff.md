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
