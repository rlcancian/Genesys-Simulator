---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# EFSM MainWindow integration checkpoint — 2026-10-30

## Scope and source state

- Repository: `/home/rafaelcancian/Repositories/TEMP_GENESYS_BRANCH_MODALMODEL/Genesys-Simulator`
- Branch: `WiP20261029/modalmodel-architecture`
- Production code checkpoint: `622f4f48ced9ec92d60cbc1e3dad61411abf904b`.
- MainWindow integration regression commit: `5d4ae61eb2ebdb565706b36189929434fc4cb747`.
- Self-loop correction and regression: `64644b2c78a403c9613d286390c04fc7cb34bf92`.
- GUI parallel-transition regression: `fc815a597dd58c48cfe481317f779cf54bf2fccb`.
- Process-flow preservation assertion: `ffafc5ade794a927fbb373fad0a7f805cf620249`.
- Network create/select negative-path regressions: `2bffa8a87698ab95ce8d10054e5b105df38dc559`.
- PR #551 remains draft. No merge or push to `WorkInProgress` was performed.

## Discovered cache-lifecycle defect and fix

`MainWindow::_rebuildViewDependentControllers()` replaces the active
`GraphicalModelSerializer` when the selected graphics view changes. A layout
parsed by the serializer on `.gui` load therefore disappeared before a
Network context could restore it. The file and codec retained the expected
coordinates, but the new Network scene fell back to automatic placement.

The published fix keeps the cache shared between `MainWindow` and each
view-scoped serializer, indexed by root-model identity and Network ID. Closing
a root model removes its cache entry. This keeps two root models isolated and
does not change the `.gen` domain or Network ownership.

## MainWindow integration regression

`EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresEFSMLayoutAndDomain`
now exercises the real MainWindow workflow. The test creates a consistent
process chain, creates and attaches an EFSM through the GUI action, opens the
shared hierarchical Network tab, creates two states, sets the initial state,
creates a directed transition, edits its guard/output/priority, performs
undo/redo, moves a state, saves through the real file dialog, closes the model,
and reloads through the real open dialog. It checks restored topology,
properties, Network identity, and graphical coordinates. It also checks that
the loaded Create-to-adapter and adapter-to-Dispose process connections remain
unchanged; no process connections represent EFSM transitions.

The GUI action regressions also cancel Network creation, reject a conflicting
Network name without attaching or adding an object, then create a valid
Network. The selection action replaces an existing adapter association while
leaving the previously registered Network intact.

The test uses Qt offscreen automation for modal dialogs and selection; it is
not a claim of a separate manual mouse/keyboard session. Prior focused tests
cover multiple adapters sharing one tab, missing Network, tab close/reopen,
root model switching and root model teardown.

## Executed checks

- Focused EFSM GUI selection:
  `EFSMNetworkDiagramPersistence.*:EFSMNetworkEditor.*:EFSMNetworkUndo.*:EFSMNetworkDiagramProjection.*`
  — **37/37 passed** after the integrated workflow, self-loop/parallel actions,
  Network creation cancellation/conflict and association-replacement checks.
- The expanded MainWindow test alone — **1/1 passed**.
- Adversarial review found that the Create EFSM Transition handler validated
  `selectedStates[1]` even for the one-state self-loop path. The handler now
  validates the second endpoint only for a two-state transition. The GUI
  regression creates both parallel transitions and a self-loop through the
  actual action; it verifies the selected-state endpoints and the disabled
  endpoint selectors for a self-loop. The affected test passes. The edited GUI
  target and application target compile, and focused ASan/UBSan covering
  parallel/self-loop creation and integrated save/reopen passed **2/2** with
  leak detection disabled.
- On production-code SHA `622f4f48ced9ec92d60cbc1e3dad61411abf904b`, ordinary unit and
  kernel-unit suites each passed **1,963 tests**, with four existing tests
  disabled; smoke passed **3/3**; `gui-app` configured and built.
- Focused ASan/UBSan at the same code checkpoint passed **3/3** with
  `detect_leaks=0`; LeakSanitizer was disabled due generic plugin-initialization
  allocations previously observed in the process. This is not global leak proof.
- Developer Manual validation and rebuild passed with
  `validate_figure_specs.sh` and `make.sh`; `docs/ManualGenESyS.pdf` has 388
  pages (8,075,902 bytes). The affected text was extracted from pages 240–248
  and checked. XeLaTeX emitted non-fatal `xdvipdfmx` annotation boundary
  warnings during PDF production.
- `python3 scripts/validate-ai-docs.py` passed before this evidence was added;
  it must be rerun on the documentation commit.
- CI for code SHA `622f4f4` had unit, kernel-unit, smoke, app-start and
  documentation-governance checks passing when inspected. The Debian package
  job was still pending; these checks do not apply to the later local test
  change until that change is pushed and its own checks complete.
- The focused suite after the parallel/self-loop corrections at local HEAD
  `2bffa8a87` passes **37/37**, including the process-flow assertions.
- At commit `1a1e973dd` (same production code as `2bffa8a87`), the required
  local baselines passed: tests-unit **1,963/1,963** and tests-kernel-unit
  **1,963/1,963** with four existing disabled tests each; smoke **3/3**; GUI
  application build passed. The later changes are test-only; exact latest-head
  hosted CI remains pending.

## Bounded direct GUI inspection

The `gui-app` executable was started on a private Xvfb display with the XCB
platform. The MainWindow appeared, the Model menu showed the EFSM commands, and
the New Model action opened a second model tab. This check did not manually
complete a full mouse/keyboard EFSM editing workflow; the actual editor action
sequence and file dialogs are covered by the offscreen MainWindow regression
above. The GUI and Xvfb processes were stopped and no residual process remained.

## Milestones and remaining work

The accepted baseline remains **3/8 = 37.5%**. The automated GUI path now
provides integrated evidence for Network creation, editing, undo/redo and
save/reopen, while G1.1/G1.2/G2/G3-G4/G6 still require final review against
their complete acceptance gates. The latest source/test head requires a full
baseline and hosted CI. User Manual instructions, final manual review, and the
complete final delivery/merge gate also remain pending.
