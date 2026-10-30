---
document_type: implementation-plan
authority: maintainer-approved-plan
owner: project-maintainer
last_reviewed: 2026-10-30
review_cadence: on-modal-gui-contract-change
status: active
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# ModalModel / DefaultNetwork — Qt6 Network Diagram Implementation Plan

## 1. Authority, decision and scope

Maintainer approval: 2026-10-29, in the GenESyS ModalModel architecture discussion.
Development branch: `WiP20261029/modalmodel-architecture`, created from `WorkInProgress`.
Integration target: `WorkInProgress`. This plan is **not** evidence of a tested GUI implementation.

Read `../README.md`, `../GOVERNANCE.md`, `../ARCHITECTURE.md`, `../STATUS.md`,
`../BACKLOG_HUMAN.md`, `../BACKLOG_AUTONOMOUS.md`, and `../runbooks/LOCAL_AGENT.md`
or `../runbooks/GITHUB_AGENT.md` as appropriate. The mandatory entrypoint remains
`/docs/ai_assistants/README.md`.

This plan refines `MODAL_NETWORK_GUI_ARCHITECTURE.md` and must be read with
`GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md` and
`MODAL_NETWORK_COMPLETION_PLAN.md`.
The existing G0–G6 milestone names are retained.

### DEC-MODAL-GUI-20261029 — Approved architectural contract

1. The top-level process canvas represents `ModelComponent` instances connected by
   ordinary process `Connection` objects carrying `Entity` instances.
2. `ModalModelDefault` remains a `ModelComponent` **adapter** that activates an
   attached `DefaultNetwork : ModelDataDefinition`; it does not own a nested process
   model merely because the GUI opens a hierarchical view.
3. **Maintainer revision, 2026-10-29 — Alternative A adopted.** A Network opens
   through the **existing hierarchical submodel scene/view/tab infrastructure**
   using the Network's `ModelDataDefinition::getId()` as the context identifier.
   The editor reuses `ModelGraphicsScene`, `ModelGraphicsView`, the existing
   model-level filtering and common controls, introducing a formalism-specific
   projection/interaction policy rather than a second independent Network scene.
   This is an **editing submodel/context**, not a nested *process* simulation.
4. Network nodes, states, places, transitions, graph edges and arcs retain their
   existing mathematical types and ownership. They do **not** become
   `ModelComponent` or ordinary process `Connection` objects.
5. The `DefaultNetwork` domain model is the authority for topology, simulation state,
   validation and activation. GUI items are non-owning representations of the
   domain model. Network GUI data stores only visual/layout information.
6. Network state is shared when multiple `ModalModelDefault` adapters point to
   the same Network. Opening two views must not clone Network objects or create
   separate mathematical states.
7. The `.gen` semantic model must remain executable without a GUI. The existing
   `.gui` format includes the kernel model followed by comment-prefixed visual
   overlay sections; the new diagram layout belongs in that GUI overlay.
8. The GUI must not invoke Network activation, dispatch `Entity` objects, or introduce
   an independent simulation/event scheduler.
9. G1 starts with a read-only **EFSM** diagram. Editing, Network creation, other
   formalisms, runtime overlays and optional architecture extensions follow only
   after their respective phase gates.
10. **Level contract (DEC-MODAL-LEVEL-20261029):** `level == 0` is the root
    editing context; a nonzero `level` is the `getId()` of the
    `ModelDataDefinition` whose nested editing context contains the element.
    `ModelComponent` already inherits `ModelDataDefinition`, so both may
    identify an editing context. This is a context-membership key **not** an
    ordinal depth, an instruction to dispatch `Entity`, or a transfer of
    ownership. A Network's node/edge data definitions use `network.getId()`;
    graph/CPN incidence and EFSM/DTMC non-`ModelDataDefinition` transitions
    continue to come from the owning Network's typed topology APIs.
11. A single Network shared by multiple ModalModel adapters still has one
    domain state and one editing context identity. Neither adapter's ID replaces
    the shared Network ID.
12. Drawings may reuse one scene infrastructure but **not** interchangeable
    semantics. Projection, allowed tools, deletion/undo/redo and transition
    rendering must be formalism-aware and delegate mutations to the domain.

The approved decision **does not change** the backend hierarchy, object ownership,
process execution semantics or persistence of mathematical state.

## 2. G0.1 — Current-code inventory (GitHub inspection, 2026-10-29)

The following are **confirmed by direct file inspection on the new branch**, not
by compiling, running the GUI, or executing tests.

| Area | Observed implementation | Implication for the GUI work |
|---|---|---|
| Network bridge | `source/plugins/components/ModalModel/ModalModelDefault.h` has `getNetwork()`, `setNetwork()`, `getNetworkName()`, `setNetworkName()` and binding APIs | G1 resolves the attached Network by the existing adapter contract; do not create a new adapter |
| EFSM domain | `source/plugins/data/ModalModel/EFSMNetwork.h` exposes states, transitions, initial/current state and conflict policy | First read-only rendering reads existing EFSM topology; no semantic update |
| DTMC domain | `source/plugins/data/ModalModel/MarkovChainNetwork.h` exposes Markov states, named/probabilistic transitions and network-owned state | A later renderer must not interpret a probability arc as a process connection |
| Process scene | `source/applications/gui/genesys/graphicals/ModelGraphicsScene.h` subclasses `QGraphicsScene` and supplies process-component/connection creation and diagram-definition helpers | Reuse Qt Graphics View patterns, not the process connection-creation path |
| Existing data-definition item | `graphicals/GraphicalModelDataDefinition.h` subclasses `QGraphicsObject`, binds a non-owning `ModelDataDefinition*` | Rendering/mouse handling may be reused selectively; diagram item ownership and lifecycle require review |
| Existing diagram link | `graphicals/GraphicalDiagramConnection.h` represents `ATTACHED`/`INTERNAL` data-definition visual associations | **Not** a formal-network edge; do not confuse with `EFSMTransition` or `GraphEdge` |
| Existing hierarchical tabs | `mainwindow.cpp` implements `_ensureSubmodelViewContext()`, `_closeSubmodelViewContext()`, `QTabWidget`, and “Open Submodel” | **Reuse submodel tabs and level-filtered view**; for a ModalModel resolve its attached Network as the context owner |
| Tab context | `mainwindow.h` has `GraphicalModelViewContextKind::{RootModel, Submodel}`; the submodel context stores `modelLevel`, owner and graphics view | Preserve `Submodel` as the hierarchical context; add a type-specific projection policy **if necessary**, not a second scene class |
| Current submodel filter | `MainWindow::_canOpenSubmodelFor()` checks components/data definitions by `getLevel() == owner->getId()`, while `_ensureSubmodelViewContext()` calls `setModelLevelFilter()` | **Reuse this contract for Network data definitions.** This filter selects the context; the formalism supplies its semantic relations. Never confuse context filtering with edge traversal or ownership |
| Context menu | `controllers/GraphicalContextMenuController.cpp` builds the item menu and currently appends the existing Open Submodel action for component/data-definition selections | G1 may add a distinct, type-checked `Open Network...` action without replacing general Submodel behavior |
| Property editor | `controllers/PropertyEditorController.cpp` and `propertyeditor/` select/edit process controls and data definitions | G1 is read-only; G2/G4 must inspect editability, notification and commit pipelines before exposing Network properties |
| Catalog | `controllers/PluginCatalogController.h` exposes categorized plugin-tree insertion/selection | G2 can reuse discovery where appropriate; do not display network edges as process component plugins |
| GUI persistence | `services/GraphicalModelSerializer.cpp` uses `saveGraphicalModel()` / `loadGraphicalModel()`, emits comment-prefixed `Graphical Model Data Definitions` and `Graphical Model Components` sections, parses `GuiSection` | Add a dedicated, versioned **Network diagram overlay section**; preserve existing sections and legacy load |
| Diagram reconstruction | `services/GraphicalModelBuilder.cpp` reconstructs model graphics, including data-definition associations | Keep process diagram reconstruction unchanged; build Network views from a resolved `DefaultNetwork` |
| Undo/redo | `graphicals/ModelGraphicsScene.h` exposes `getUndoStack()`; existing actions reside in `actions/` | Read-only G1 must create no undo actions; G3 tests command ownership and atomicity |
| Qt6 build | `source/applications/gui/genesys/CMakeLists.txt` provides `genesys_gui_application` / `genesys-gui`, with C++23 | Use the existing `gui-app` preset and the normative Qt6 baseline; the existing CMake Qt5 fallback is outside this work |
| Unit harness | `source/tests/unit/CMakeLists.txt` has `genesys_add_unit_test()`, `gtest_discover_tests()`, and conditional Qt GUI tests | G0.2 may add headless record-codec tests following actual target/link patterns |

**Evidence boundary:** this is a source inspection, not a statement that any new
Modal Network GUI behavior exists or that current HEAD passes build/tests.

### G0.1 follow-up: level filtering can support a Network context

The initial recommendation to separate Network scenes has been **superseded**
by explicit maintainer approval of Alternative A on 2026-10-29.

**Confirmed in code:** `ModelDataDefinition::getLevel()` and
`setModelLevel()` exist; `_canOpenSubmodelFor()` already considers both
`ModelComponent` and `ModelDataDefinition` instances; Network load routines
for EFSM, Markov, Graph and CPN set the child `ModelDataDefinition` level to
the Network's ID. `ModelComponent : ModelDataDefinition`.

**Confirmed gap:** the process-oriented `GraphicalModelBuilder` starts from
visible process components and their attached/internal data and uses
`GraphicalDiagramConnection` for those associations; correct `level`
alone does not yet render a Network's formal transitions. G1 must support
level-scoped data-definition discovery even with zero process components,
then render the Network's typed topology using a formalism-specific adapter.

**Initial bounded code slice:** on programmatic insertion of states/nodes/edges,
places/CPN transitions/arcs into their Network, set the accepted data
definition's `modelLevel` to the Network's `getId()`. Apply only on successful
registration: rejected edges/arcs remain unchanged. Match the load-time contract
without modifying mathematical transitions that are **not** data definitions.
Four focused Google Test cases cover EFSM, Markov, Graph and CPN.
**Validation update (2026-10-29):** CMake/Ninja/CTest passed at source SHA
`5a37c7ae` on [GitHub Actions run 40961067698](https://github.com/rlcancian/Genesys-Simulator/actions/runs/40961067698), including all ten new tests (1920 executed, zero failed, four legacy disabled). These tests alone did not close G0.1b; the later local GUI/manual verification was reviewed and accepted on 2026-10-29. G0.2 was subsequently implemented and accepted on 2026-10-30 with the precisely documented legacy empty-name `.gen` deviation.

**Identity/persistence limitation:** `ModelDataDefinition::_saveInstance()`
persists `id`, `name` and other data but not `_modelLevel`; current Network
loaders reconstruct levels. A general nested process submodel may require a
separate persistence contract. `getId()` is `Util::identification` (currently
`unsigned long`), while `getLevel()/setModelLevel()` use `unsigned int`.
The existing GUI casts the owner ID to `unsigned int`; whether widening the
level type is necessary is a dedicated compatibility/ABI and persistence gate,
**not authorized as a cosmetic change**.

**Ownership limitation:** `level` encodes context membership, not an
exclusive-owning pointer. A Network can be shared across adapters; its child
objects should not be reused simultaneously in multiple distinct Network
contexts without an explicit supported multi-membership model.

**Other important limitation:** the existing process `Submodel` plugin has
TODOs in load/save/check; its execution currently forwards entities. It must
not be presumed to implement general hierarchical editor semantics.

### G0.1 risk discovered: the current `.gui` file is a combined artifact

Current `GraphicalModelSerializer::saveGraphicalModel()` writes the canonical
kernel model first and then appends GUI visual data. The logical distinction
between `.gen` semantics and `.gui` layout **does not imply** two physically
independent files must always be present. New records must integrate with this
actual format and existing loaders.

### G0.1b — Level-only model data projection (code + CI passed; manual gate pending)

**Source-backed patch as of 2026-10-29:**

- `GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer()` now
  enumerates registered `ModelDataDefinition` objects whose
  `getLevel() == scene->modelLevelFilter()` for **non-root** contexts.
  Previously the builder could discard the data-definition layer when there
  were no visible process components or associated definitions.
- A direct level member does not require a `ModelComponent` in that level,
  a `SimulationControl` reference or an `INTERNAL`/`ATTACHED` relation.
  Existing visibility toggles continue to apply to *association discovery*;
  they do not hide the context's direct members.
- Only non-root `level != 0` gains this direct-membership discovery. The
  root canvas keeps its existing association-based data-definition visibility.
- Existing graphical items and user positions are reused. New members with
  no component or data-definition layout link receive an ID-ordered,
  collision-avoiding vertical fallback; G1 formalism-specific layout will
  supersede the fallback for an EFSM. This is **not** a mathematical
  transition renderer.
- `source/tests/unit/test_gui_submodel_level.cpp` is registered in the
  existing `genesys_test_gui_gmdd_layout` target and checks:
  (1) EFSM states in a level with zero process components, including association
  visibility toggles off and no invented semantic links;
  (2) switching contexts without altering Network state;
  (3) preserving manual positions across synchronizations;
  (4) direct context membership when an arbitrary data definition owns a level;
  (5) the complete GraphicalModelBuilder rebuild path with zero process
  components; and (6) unchanged root-level behavior for an unreferenced
  data definition.

**Validation update:** all six `GuiSubmodelLevel.*` tests passed in the full
CMake/Ninja/CTest run `40961067698` at SHA `5a37c7ae`; the GUI GMDD diagnostic job also passed. This demonstrates a tested Qt6 scene layer, **not** a `gui-app` startup or live interactive navigation result. Additional documentation-only commits do not change that tested source, but the final-head verification gate still applies.
The Developer Manual's `chapter_gui_architecture.tex` has been updated; LaTeX compilation, PDF regeneration (388 pages) and bounded GUI interaction were completed in the local Codex execution and accepted for milestone 2 on 2026-10-29.

**Deliberate non-goals:** no EFSM transitions are drawn yet, no owner resolution
from `ModalModelDefault` is wired to a tab, no `.gui` Network layout schema
is introduced, and no generic Entity-flow or domain ownership rule changes.

## 3. G0.2 — Network diagram persistence contract (implemented; accepted with explicit exception)

The logical `NetworkDiagramRecord` outlined below was implemented for EFSM
as `EFSMNetworkDiagramLayout` plus `EFSMNetworkDiagramCodec`, integrated with
`GraphicalModelSerializer` in the versioned `.gui` section
`Graphical EFSM Network Diagrams`. The general record remains a conceptual
contract, not a generic C++ class or a codec for other formalisms.

The **logical** version-1 record is:

| Field | Required contract |
|---|---|
| `schemaVersion` | Explicit `1`, reject/skip unknown future major versions safely |
| `networkRef` | Stable reference scoped to the parent model: Network class/type + name; include persistent ID only after validating ID stability across `.gen` load |
| `nodes[]` | Logical element reference (class/type + name or validated stable key), position and optional presentation metadata |
| `relations[]` | Formalism-specific relation reference, optional bend/label coordinates; must distinguish parallel relations and unnamed arcs |
| `viewport` | Optional zoom/center and other GUI-only view metadata |
| `extensions` | Optional forward-compatible presentation fields only |

Rules for the on-disk format:

- Put the record into a **distinct, comment-prefixed, versioned GUI section**
  in `.gui`; keep existing `GuiSection` parsing behavior and loading of
  previously saved models unchanged.
- Do not include mathematical topology, initial/current states, transition
  priorities/guards/actions, transition probabilities, markings or `Entity` data.
- Use a deterministic serialization order and the existing GUI text-encoding
  conventions where compatible; malformed records must not corrupt the model.
- A missing or orphaned visual reference is ignored or reported as a layout
  warning. It must never synthesize domain objects.
- A missing GUI record uses a deterministic auto-layout fallback in the viewer;
  `.gen` loading and simulation work independently.
- Multiple views of the same Network share **one Network identity** and its
  domain state. Layout can be per-Network; per-view viewport metadata must not
  create a second mathematical Network.
- **Identity gate for this delivery:** prove stable references for EFSM
  states (including duplicate display names where permitted), unnamed EFSM
  transitions and parallel EFSM relations. If current persistence cannot
  distinguish them, stop and record the limitation; do not silently select
  the wrong transition or change kernel ownership solely for GUI convenience.
  Identities for Graph/DTMC/CPN relations are deferred until their editors
  are explicitly authorized.
- The initial EFSM viewer does not require persisted layout to display a diagram:
  building the view from the current domain topology with deterministic
  positions is a valid G1 fallback.

G0.2 must add headless tests for: empty and populated EFSM diagram records,
Unicode/escaping, deterministic round-trip, unknown-version and malformed
records, missing EFSM elements, distinct unnamed/parallel EFSM transition
identities and loading an old `.gui` without Network sections. Other formalism
codecs are not a requirement of the frozen MVP.

**G0 gate:** approved record format + unit tests executed and green; no
mandatory regressions. G0.1 documentation alone does **not** close G0.

## 4. FROZEN delivery plan — eight milestones, no implicit sub-phases

**Maintainer scope freeze: 2026-10-29.** The deliverable is a **working
EFSM graphical editor** inside the already implemented submodel editor:
draw EFSM states and transitions, open the attached Network, create/select
the Network, edit states/transition properties, undo/redo, save/reload and
execute the same semantics without GUI dependencies. The shared
context/level infrastructure must remain adaptable to other Network types,
but other formalism editors are **not** an acceptance requirement.

This plan has **exactly eight milestones**. The labels G0.1, G0.1b, G0.2,
G1.1, G1.2, G2, G3/G4 and G6 are *fixed identifiers*, not decimal
sequences: there will be no implied G0.3–G0.9 or G1.3–G1.9, nor extra
milestones or automatic scope expansion.

| # | Existing reference | Bounded deliverable | Acceptance gate | Status as of 2026-10-30 |
|---|---|---|---|---|
| **1** | **G0.1** | Approve Alternative A and owner-ID `level` context contract; inspect existing code and record plan | Maintainer decision + architecture/backlog source links committed | **DONE** |
| **2** | **G0.1b** | Finish verification of Network-level registration and direct level-only GUI data-definition projection, including mandatory manual | Final-head CMake/Ninja/CTest, six GUI and four domain regressions, `gui-app` build, short interaction check, LaTeX compile, regenerated `docs/ManualGenESyS.pdf`, dated evidence | **ACCEPTED — 2026-10-29** |
| **3** | **G0.2** | Persist only **EFSM** diagram layout/identity as versioned GUI overlay | Deterministic save/reload including unnamed/parallel EFSM transitions; invalid/unknown records safe; preexisting `.gui` loads; `.gen` semantics unchanged; focused tests green | **ACCEPTED 2026-10-30.** The accepted exception for empty transition names was subsequently corrected by saving the `transitionName` field even when empty; omitted legacy fields retain their existing load fallback. Regression: `EFSMNetworkTest.PersistencePreservesExplicitlyEmptyTransitionName`. |
| **4** | **G1.1** | Draw read-only EFSM states and formal transitions in existing `ModelGraphicsScene` | Empty/normal/parallel EFSM cases; no fake `ModelComponent` or process `Connection`; layout clear; GUI tests green | **Projection and layout behavior are covered by executed Qt tests.** Shared-scene tests cover empty/single/multiple states, unnamed parallel transitions, self-loops, initial/final styling, deterministic fallback, persisted positions, and unchanged domain topology. The MainWindow flow now exercises actual scene construction and saved coordinates. Full final-head baseline, CI, and separate manual mouse/keyboard verification remain pending. |
| **5** | **G1.2** | Open attached EFSM Network through the existing submodel tabs at `Network.getId()` | Open/switch/close/reopen and no-Network handling; multiple adapters share one Network; root GUI unaffected | **Core navigation/lifecycle paths have focused MainWindow tests.** These cover shared-tab reuse from two adapters, empty Network, missing Network, close/reopen, root-model switching and closing a root with an open Network tab. The integrated save/close/reopen path now also opens the loaded Network and checks restored geometry. Final-head baseline/CI and separate manual UI verification remain pending. |
| **6** | **G2** | Create/select/attach an EFSM Network using existing GUI/Property Editor patterns | New and existing Network attachment, correct references and `.gen` save/reload; no duplicate domain objects | **Create/select/attach are exercised through GUI actions.** Same-name Networks are disambiguated by registered ID; adapter bindings use an undoable dialog. The integrated MainWindow regression creates and associates a Network through the GUI and verifies it survives `.gen`/`.gui` save/reload without duplication. Final-head baseline/CI remain pending. |
| **7** | **G3/G4** | Edit EFSM states/transitions and essential guard/action/properties through domain APIs; undo/redo | Create/change/delete, invalid-operation rejection, undo/redo, backend checks and persistence; targeted ownership sanitizer if appropriate | **In progress, with integrated GUI actions covered.** Commands cover state creation/deletion/properties/initial flag and transition creation/deletion/properties. The MainWindow regression creates states and a directed transition, edits guard/output/priority, performs undo/redo, and verifies persisted semantics. A separate action regression now creates a self-loop; it caught and fixed an out-of-range second-endpoint access in the one-state path. Existing targeted ASan/UBSan covers self-loop creation and ownership-sensitive paths. Mixed-command/lifetime review, final baseline/CI, and separate manual UI verification remain. |
| **8** | **G6 (delivery)** | Final integration/regression, manuals, evidence and review | `gui-app` and full relevant CTest suites green; bounded real GUI end-to-end scenario; manual PDF current; draft PR reviewed and merged to `WorkInProgress` only with maintainer approval | **NOT STARTED** |

**Progress definition:** each accepted milestone contributes **12.5 percentage
points**. Report `N/8 = NN.N%` on every subsequent iteration. Implementation
in progress, green CI for an *earlier* head or a checked-in test that has not
run **does not** count as a completed milestone. Current accepted progress remains **3/8 = 37.5%**. At the time of G0.2 review, the empty-name `.gen` behavior was an explicitly accepted exception; it was corrected in commit `5f63fee85`, which now saves explicit empty transition names while preserving legacy omitted-field loading. The current source checkpoint and validation are recorded in `history/evidence/2026/10/2026-10-30_modal_efsm_transition_undo_foundation.md`. A maintainer mandate dated 2026-10-30 authorizes sequential execution of all remaining milestones. G1.1 is implemented but not yet accepted; its interactive view verification and broader regression gates remain pending.
Percentage measures agreed deliverables, **not effort or elapsed time**.

**Rule for scope control:** a newly discovered defect is resolved *inside*
the milestone it blocks; it does not automatically add a stage. If the defect
requires a material new architecture/product decision, stop, present evidence
and request explicit maintainer authorization to modify the frozen plan.
Do not expand checklists into new tasks or silently add "G0.3" etc.
A finding that is genuinely outside the MVP is recorded as deferred debt,
not attached to the current acceptance gate.

### What is intentionally excluded from the eight milestones

The former **G5 runtime overlays/animation** and other formalism editors
(Graph/DTMC/CPN/CTMC/MDP), advanced automatic layout, whole-cell modeling,
dynamic plugins, general hierarchical persistence migration and broad GUI
refactoring are **out of scope for this release**. This is a deliberate
scope reduction authorized by the maintainer's request to reach a small,
usable EFSM editor quickly. No G5 is silently queued. Essential bug fixes
needed for an eight-milestone acceptance gate remain in scope.

### Milestone 2: exact remaining work and order

1. **CI evidence (already executed at commit `5a37c7ae`):** GitHub Actions
   `40961067698` completed CMake/Ninja `tests-unit` and CTest green:
   **1920 executed, 0 failed, 4 legacy disabled** (1924 registered).
   All four `Programmatic*BelongToNetworkEditingLevel` and six
   `GuiSubmodelLevel.*` tests are explicitly listed as passed. The
   `Diagnose GUI GMDD tests` job is also successful. After the present
   documentation commits, recheck final PR head; do not reuse this evidence
   for a changed source build without verifying the actual revision.
2. **GUI main executable:** `cmake --preset gui-app` and
   `cmake --build --preset gui-app`, then a bounded root/submodel scene
   and Property Editor interaction/close test (local agent or existing
   fit-for-purpose GUI CI).
3. **Developer Manual:** source chapter
   `docs/developers/ManualGenESyS_subrepo/chapter_gui_architecture.tex`
   is already updated; no current User Manual feature claim. Run
   `cd docs/developers/ManualGenESyS_subrepo`,
   `./validate_figure_specs.sh` and `./make.sh`; verify LaTeX exit code,
   references, generated PDF pages and update
   `docs/ManualGenESyS.pdf` in **this PR**.
4. **Evidence:** record exact SHA, toolchain, CMake/CTest results, GUI
   interaction/negative cases, PDF build/inspection and limitations in
   `docs/ai_assistants/history/evidence/`; reconcile task/STATUS and
   manual impact. **Milestone 2 gates accepted 2026-10-29; G0.2 is now authorized.**

The GitHub connector cannot itself execute the local manual build or commit
the generated binary PDF through its UTF-8 text-file update operation.
This is an environment limitation, **not** a justification to waive the
manual gate. The local agent may finish milestone 2 without any new design
decision.

## 5. Handoff to a future local coding agent

The local agent must start by reading
`/docs/ai_assistants/README.md`, then the repository root README,
governance, architecture, status, backlogs, local runbook and the three
Modal/Network documents referenced in Section 1.

1. Confirm clean Git status, fetch and inspect `origin/WorkInProgress`, and
   check that `WiP20261029/modalmodel-architecture` has not drifted.
2. Inspect implementation (not only headers) of tab/switch/close workflows,
   `GraphicalModelSerializer`, `GraphicalModelBuilder`, level discovery,
   model identity and formalism relations. Respect the new Alternative A
   decision: no independent `GraphicalNetworkScene` as default approach.
3. Validate the Network insertion level tests already added in this branch
   using the actual CMake/Ninja targets. Then validate G0.2 GUI layout
   identity/storage before changing the serializer or test target graph.
4. Implement one bounded concern per commit; use tests first for the codec,
   then add read-only EFSM presentation as an independently reviewable slice.
5. Run `cmake --preset gui-app`, `cmake --build --preset gui-app`,
   focused GUI tests, `cmake --preset tests-unit`,
   `cmake --build --preset tests-unit` and
   `ctest --preset tests-unit --output-on-failure`; run smoke and sanitizers
   for any runtime/ownership change. Adapt only to actually configured targets.
6. For G1, manually or in a supported UI harness verify opening EFSM, switching
   tabs/models, closing a model with an open Network view, reopening the same
   Network from two adapters, and opening with no Network attached.
7. Write dated executed evidence in `history/evidence/`; update the appropriate
   task state, STATUS, applicable manuals and AI changelog before requesting
   integration.
8. Open a **draft PR** into `WorkInProgress` and do not merge without green
   mandatory validation and maintainer review.

## 6. Current delivery and remaining risks

The initial 2026-10-29 delivery and its historical CI evidence are recorded above and in the dated evidence files. Current accepted progress remains **3/8 = 37.5%**. The G0.2 empty-transition-name exception was corrected in `5f63fee85` with a regression test. The read-only EFSM projection, tab resolution and basic create actions are present on the active branch; current limitations and executed results are in the 2026-10-30 checkpoint evidence. The maintainer mandate dated 2026-10-30 authorizes the executor to continue all remaining milestones sequentially; G1.1 is still in validation and no later milestone is accepted.

**Manual impact gate:** the edited Developer Manual source chapter already documents editing-context semantics and level-only graphical reconstruction. It was compiled, visually inspected (section 26.7) and used to regenerate the committed `docs/ManualGenESyS.pdf`; this gate was accepted on 2026-10-29. A projection implementation now exists, but an end-to-end interactive Network-view workflow has not yet been demonstrated; do **not** document the complete editor as an operational User Manual feature. At G1/G2/G3 the User Manual impact is reassessed for actual user-visible behavior.

Risks to resolve locally: stable identity of relation objects across reload,
context ID width/possible truncation, Network-owned child registration/lifetime,
level persistence in generic submodels, formalism-aware rendering inside the
shared scene, multi-view reference invalidation, switching/restoring active tabs,
Qt QObject/GraphicsItem versus kernel object ownership, `.gui` compatibility,
editing command rollback, and regression against existing GUI/property-editor
behavior. Resolve these prerequisites within the milestone they affect; the current maintainer mandate authorizes continuing through G1.1, G1.2, G2, G3/G4 and G6 without intermediate prompts, while preserving each milestone acceptance gate.

### Coordinator operating clarification — 2026-10-30

The goal is eight *deliverables*, not dozens of manual prompt/review loops. The
local agent must self-review, fix all in-scope defects, run focused and relevant
full tests, document, commit and push within **one execution per milestone**.
Coordinator review must distinguish a genuine safety/semantic blocker from
nonblocking debt and must not add new acceptance criteria after implementation.
The accepted G0.2 exception was closed by `5f63fee85`. The 2026-10-30
maintainer mandate supersedes the earlier G1.1-only handoff and authorizes
sequential implementation of all five remaining milestones. Stop only for a
real governance gate, unsafe ownership/persistence change, or reproducible
blocker that cannot be resolved within the approved architecture.
