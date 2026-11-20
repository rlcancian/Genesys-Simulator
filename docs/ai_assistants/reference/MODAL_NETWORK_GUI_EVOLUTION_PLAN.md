---
document_type: implementation-plan
authority: maintainer-approved-execution-plan
owner: project-maintainer
decision_date: 2026-11-12
status: approved-ready-for-local-agent
tracks: HUM-MODAL-003,AUTO-MODAL-005,HUM-ARCH-004
integration_branch: WorkInProgress
implementation_branch_suggested: WiP20261112/modal_model_gui
review_cadence: on-milestone-completion
---

# ModalModel / DefaultNetwork GUI — Complete Evolution and Execution Plan

## 1. Authority, scope and execution mandate

This document records the explicit maintainer authorization of 2026-11-12. Its purpose is FULL, SEQUENTIAL IMPLEMENTATION by a local agent (such as Codex) in the GenESyS checkout, without additional intermediate approval for decisions specified here. The agent must implement milestones G0, A1–A5, B1–B5, C1–C6 and D1–D5, complete the essential tests, document progress and leave one development branch ready for human review and later integration. An unfinished milestone is not success. Do not stop after producing only architecture, scaffolding, a partial GUI or a draft PR.

Read /README.md and /docs/ai_assistants/README.md, then GOVERNANCE.md, ARCHITECTURE.md, STATUS.md, both backlogs, the Modal/Network architecture and GUI plans, and GUI_EXTENSIONS_ARCHITECTURE.md. These are binding references. Reconfirm claims against the source on the agent's synchronized working base. The planning baseline inspected on 2026-11-12 is origin/WorkInProgress commit a7f5140aa3148a1c4b58e69251c104ab6bfab9de; the actual implementation base must be the latest origin/WorkInProgress when local execution starts.

**Execution policy:** functional completion first, proportionate validation, continuous progress. Do not perform repetitive broad audits, speculative modernization, unnecessary compatibility rewrites, unrelated refactors, or exhaustive test campaigns. Run focused tests with each milestone, a GUI build when GUI code changes, and the necessary baseline suites at integration checkpoints. Reuse existing infrastructure. An agent may make ordinary implementation decisions consistent with this approved architecture. Request human intervention only for a genuine blocker or substantive question outside this plan. This approval covers source changes on a new development branch; it does NOT authorize merging into WorkInProgress without final human review.

**Scope:** ModalModelDefault Property Editor association UX, generic Open Network, polymorphic type/identity/persistence safety, EFSM GUI isolation, Graph / directed graph / DAG editor, MarkovChainNetwork editor, ColoredPetriNetNetwork editor, contextual editing, and network visual animation of existing mathematical runtime state/events. Deliver all planned capabilities incrementally in the SAME dedicated development branch, with small traceable commits.

**Non-goals:** changing mathematical semantics or simulation calendar, converting structural graphs into Entity-flow graphs, adding CTMC/CellularAutomata-to-DefaultNetwork migration, implementing dynamically loadable GUI libraries or a new C ABI, replacing the Qt scene framework, broad kernel/GUI refactors, modifying unrelated plugins, general release/promotion, or automatic final merge.

## 2. Verified source architecture and invariants

- ModalModelDefault is a ModelComponent, holds at most one non-owning DefaultNetwork* and may initially hold nullptr. A Network is a model-managed ModelDataDefinition; multiple adapters may share it. ModalModelDefault::_check rejects a missing Network. Constructing an incomplete model during editing is allowed.
- GraphNetwork ALREADY inherits DefaultNetwork; DirectedGraphNetwork inherits GraphNetwork; DirectedAcyclicGraphNetwork inherits DirectedGraphNetwork. Preserve this hierarchy; no inheritance rewrite is authorized. EFSMNetwork, MarkovChainNetwork and ColoredPetriNetNetwork also derive from DefaultNetwork.
- GraphNetwork currently has structural graph algorithms; do not invent runtime activation, Entity routing, a current vertex or random walks. Base DefaultNetwork is concrete and inert on activation; do not make it abstract. Its direct creation may remain available only if explicitly labeled generic/inert, without implicit default creation in the Property Editor.
- The main component palette exposes ModelComponents, not all mathematical DataDefinitions. Domain objects are not interchangeable with process components/connections.
- The current Property Editor can select existing derived DefaultNetwork objects but defaults to constructing DefaultNetwork; generic typed creation is missing. Existing MainWindow Create/Select actions target EFSM specifically.
- The current network editing tab uses the Network object's getId() as editing-context level, not adapter ID; two adapters opening the same Network share context and mathematical object. Retain model+Network identity for multi-model GUI safety.
- The EFSM GUI MVP from PRs #551/#552 is a valid regression baseline (8/8 previously accepted); it does not prove Graph/DTMC/CPN GUI implementations.
- Existing GUI extension contracts are GuiExtensionPlugin, GuiExtensionRegistry, GuiExtensionManager and GuiExtensionPluginCatalog in source/applications/gui/genesys/extensions. Use ONE graphical-extension framework. Static registration first; future dynamic loading remains possible but is NOT in scope.
- Current ModalModelDefault persistence uses networkName for its reference. This needs an identity audit to prevent ambiguous cross-type name resolution; any migration must retain documented compatibility.

Relevant code anchors (verify at execution HEAD):
source/plugins/components/ModalModel/ModalModelDefault.cpp and .h;
source/plugins/data/ModalModel/{DefaultNetwork,EFSMNetwork,GraphNetwork,DirectedGraphNetwork,DirectedAcyclicGraphNetwork,MarkovChainNetwork,ColoredPetriNetNetwork}.h;
source/kernel/simulator/{SimulationControlAndResponse,GenesysPropertyIntrospection}.h and related implementations;
source/kernel/simulator/model/ModelDataManager.*;
source/applications/gui/genesys/{mainwindow.cpp,controllers/GraphicalContextMenuController.cpp,controllers/PluginCatalogController.cpp,graphicals/ModelGraphicsScene.cpp,graphicals/ModelGraphicsView.cpp,propertyeditor/ObjectPropertyBrowser.cpp,services/EFSMNetworkDiagramProjection.cpp,services/EFSMNetworkDiagramCodec.cpp};
source/applications/gui/genesys/extensions/{GuiExtensionContracts.h,GuiExtensionManager.*,GuiExtensionPluginCatalog.*}.

## 3. Approved user interaction contract — binding decision

### 3.1 ModalModelDefault context menu
The ONE Network-related contextual action is **Open Network**:
- enabled only if the adapter has a non-null, resolvable, live, model-registered DefaultNetwork of any concrete subtype;
- disabled otherwise; must never create an object, select an alternative reference or silently repair an invalid one;
- opens/reuses the associated Network's own graphical context through the existing shared scene/tab mechanism;
- does not depend on whether the Network's topology is empty; an empty valid Network is openable.
There must be NO **Create Network** or **Select Network** in the ModalModelDefault context menu. Remove previous **Create EFSM Network** and **Select EFSM Network** entry points from menus, including main Model-menu shortcuts that bypass the Property Editor. Preserve other non-Network actions. The generic Submodel action remains for Submodel, with appropriately distinct naming.

### 3.2 GraphicalModelDataDefinition context menu
For a graphical ModelDataDefinition whose concrete object is a registered DefaultNetwork subtype, expose the same **Open Network** action, operating on that object. Other DataDefinitions must not acquire fake Network commands.

### 3.3 Property Editor is the sole association surface
The ModalModelDefault **Network** property supports:
1. No Network / unassigned state on construction, with no implicit Network creation.
2. Select from existing model-managed DefaultNetwork-compatible objects, showing concrete type, object name and unambiguous identity.
3. Create a NEW Network with explicit supported concrete subtype selection and name, using registered kernel PluginManager factories and ModelDataManager; never silently default to DefaultNetwork.
4. Replace the reference with another compatible existing or newly created Network, after validation of binding/port consequences.
5. Remove/detach the reference without deleting the model-managed object; preserve shared Networks.
6. Edit the referenced Network's properties from the Property Editor, separately from opening its graphical editor.

Do not make the base DefaultNetwork abstract. If offered as an explicitly chosen type, label its inert activation semantics. Discover only available, valid concrete plugin types; do not use GUI hardcoded class names as the permanent discovery mechanism. Prefer the existing polymorphic property descriptor/creatable-types mechanism, with a narrowly scoped factory extension if the current control cannot create a requested subtype. Avoid regression in other DataDefinition-reference properties.

Double-click, if supported, may only open a VALID associated Network. With no Network it must not create a Network or launch a second selection flow; at most focus the Network property.

### 3.4 Runtime and UX correctness
Permit incomplete editing without making model-check pass incorrectly. Before simulation execution, a missing required Network must be rejected through existing validation; distinguish mere reference validity from formalism execution capability. A structural GraphNetwork may have a graphical editor but no custom runtime activation; disclose this honestly, and never invent Entity-flow semantics. A rename, replacement, removal or load must not leave stale cached GUI references/tabs.

## 4. Cross-cutting architectural contracts

**Domain versus view:** No Qt/GUI dependency in kernel, parser or simulation plugins. Domain topology/state is the sole truth; graphical coordinates/styles/animation items are GUI data. GraphEdge, EFSMTransition, MarkovTransition and CPNArc are NOT process Connections. Preserve actual C++ domain classifications; GraphNode/FSMState/PetriPlace etc. are not necessarily draggable ModelComponents.

**Shared identity:** Editor context key is root Model identity + live Network identity; context editing level equals Network getId(). Reopen from multiple adapters must reuse the same domain and appropriate graphical tab. Do not duplicate mathematical nodes, edges, tokens, transitions or other state. Graphical views may differ without cloning domain objects.

**Persistence:** Model semantics belong to .gen and graphical layouts to .gui. Preserve current EFSM format and known fixtures; add compact, versioned per-formalism graphical sections/records only where necessary. Typed reference resolution should prefer verified stable identity in the same model, with a safe legacy fallback and explicit ambiguity diagnostics; preserve historical readable files and avoid silently choosing an arbitrary duplicate name. Unknown graphical records should not be discarded just because an extension is unavailable. Test save-load-save and cross-version behavior proportionally.

**Lifetime:** Network references are non-owning. Distinguish model-owned and transition-owned objects from scene-owned graphics. GUI extension disable, model close, tab close, undo/redo and simulator shutdown must not leave dangling callbacks or pointers.

**GUI extensions:** Preserve a single registration/manager/catalog. Generic GUI host owns scene, tabs, selection, navigation and event dispatch. Concrete graphical extensions own EFSM/Graph/DTMC/CPN actions, palettes, projections, editors, renderers and visual interpretation. Prefer location source/applications/gui/genesys/extensions/plugins/ for newly extracted concrete code; do not eagerly move unrelated files. Extend contribution categories only when a working end-to-end slice requires it. Maintain static linking/registration and a future dynamic-capable separation. Coordinate narrowly with AUTO-GUI-003: this plan authorizes ONLY capabilities needed by the ModalNetwork scope; it does not activate the unrelated Queue/Resource/Bacteria/CellularAutomata migrations.

## 5. Execution sequence and acceptance gates

All milestones are REQUIRED for the declared full scope. Implement in the order below, allowing narrow dependency-driven interleaving. After each milestone, run the focused tests and build surfaces actually affected, record pass/fail and next steps, then continue automatically. A failed focused gate must be fixed before further dependent work; a not-yet-implemented feature must be marked incomplete, not passed.

### G0 — Bounded inventory and initial fixtures
- Confirm latest source classes, control contracts, extension entry points, menu paths, data ownership, test target names and relevant .gen/.gui formats.
- Create or identify the minimum regression fixtures: existing EFSM flow; two adapters sharing EFSM; structural Graph/directed/DAG; Markov DTMC; CPN; null Network; duplicate names across concrete classes.
- Record a compact source-to-requirement matrix and a baseline focused test run. G0 is a bounded preparation step, not a multi-day repository-wide audit.
- PASS: documented exact target entry points, reliable first tests and no unknown blocking contract.

### A1 — ONE generic Open Network
- Remove the dedicated EFSM create/select actions from component context menu and main-menu access; preserve their functionality exclusively via Property Editor in A2/A3.
- Rename/adapt opening to **Open Network** for ModalModelDefault and Network DataDefinition items; preserve **Open Submodel** for real Submodel contexts.
- Use current owner resolution, registration validation and tab reuse; no creation via open/double-click.
- PASS: only Open Network on adapter context; disabled for null/stale references; same object opens from adapter or graphical Network definition, including empty Network. No regression for existing Submodel.

### B1 — Focused reference-identity audit, before broader selection
- Inspect ModalModelDefault::_resolveNetworkReference, networkName persistence, ModelDataManager lookup, object IDs during save/reopen, duplicate names by type and model ownership.
- Specify the smallest backward-compatible typed reference encoding. Check whether class+ID survives .gen reload; if not, use a verified stable alternative. A legacy ambiguous reference must fail clearly rather than bind to the first object.
- PASS: a reproducible identity matrix including legacy and collisions; precise implementation path for B2.

### A2 — Choose existing Network in Property Editor
- Enumerate registered model instances assignable to DefaultNetwork, not merely EFSM lists.
- Display concrete type, name and identity; bind by selected instance identity, not ambiguous display name; support initial null, current reference, and selection of another reference.
- PASS: EFSM/Graph/DirectedGraph/DAG/Markov/CPN select correctly; duplicate names across subtypes do not change the intended object; cancellation is a no-op.

### A3 — Typed creation exclusively in Property Editor
- Use the registered subtype's factory; explicitly choose concrete type and name before creation; handle unavailable plugins and creation/registration errors.
- Commit the model-managed object and adapter association coherently. If creation or registration fails, leave no half-associated or leaked object.
- Avoid hardcoded subtype selection directly in generic ObjectPropertyBrowser or MainWindow. Prefer extension of current generic control typed-creation hook or a narrowly scoped provider wired through the existing property contract.
- PASS: correct dynamic type, class registration, name and association for each supported subtype; default remains null until explicitly configured; no accidental plain DefaultNetwork.

### B2 — Safe typed persistence and restore
- Add backward-compatible reference identity fields only if B1 proves necessary, without silently changing unrelated serialization.
- Resolve exact registered object, detect invalid/ambiguous/missing references, and preserve current supported .gen files.
- PASS: each concrete type round-trips with the exact adapter association; duplicate names and damaged references fail safely; current EFSM GUI save/reopen-to-kernel-activation regression stays green.

### B3 — Transactional replacement and port/binding reconciliation
- Evaluate old/new input/output port counts, names and connections BEFORE modifying reference. Reuse matching bindings only when unambiguous; never silently discard user expressions or process connections.
- Present incompatibility/confirmation in GUI; cancel leaves old association unchanged. On accepted change, preserve logical consistency and model editability; do not delete old Network. Support undo/redo when appropriate for this editor.
- PASS: same-schema, increased/decreased ports, shared references, cancel and failure tests; no stale graphical references.

### B4 — Ownership/lifetime and validation closure
- Verify non-ownership under reference replacement/removal, model deletion, network deletion (where permitted), scene/tab closing and undo; ensure model check remains authoritative for simulation.
- Use targeted ASan/UBSan only when pointer/lifetime paths change; do not claim global leak freedom.
- PASS: focused lifetime and invalid-reference tests, no demonstrated dangling use or double deletion.

### A4 — Complete replacement/removal UX
- Surface the B3-safe operations in the Property Editor; allow remove-reference without deleting a shared Network and reattach later.
- PASS: all association transitions (null->new, null->existing, existing->other, existing->null) work from one property surface, including save/reopen.

### A5 — Finish concise UX and regression
- Display concrete network type, name, identity and structural/inert capability where relevant. Provide separate Property Editor Edit Properties and context Open Network.
- PASS: end-to-end Create -> ModalModelDefault -> Dispose edit flow works for each registered subtype, without EFSM-specific creation shortcuts.

### B5 — Only necessary physical-structure cleanup
- Inspect actual includes/translation-unit placement in components/ModalModel and data/ModalModel. Change physical placement only if demanded by completed GUI integration or concrete build/layering defects. No broad directory migration and no GraphNetwork inheritance change.
- PASS: no unnecessary source moves; applicable dependency/build test passes.

### C1 — Scoped EFSM GUI coupling inventory
- Identify EFSM-specific code in mainwindow.cpp, GraphicalContextMenuController, ModelGraphicsScene, EFSMNetworkDiagramProjection/Codec, undo commands and serializer.
- Define minimal responsibilities and a static implementation entry point for the ModalNetwork graphical extension.
- PASS: compact file-to-responsibility map, with no generic GUI knowledge needed for concrete EFSM handlers at the target end state.

### C2 — Extract EFSM handlers/dialogs
- Move EFSM-specific action handlers and presentation code into concrete GUI-only code, using existing scene/editor services. Preserve existing behavior, undo/redo and .gui layout.
- PASS: focused EFSM editor tests and GUI build remain green; MainWindow shrinks in domain-specific behavior.

### C3 — Register actions through existing GUI extension mechanism
- Use GuiExtensionPlugin::registerContributions and GuiExtensionRegistry; retain existing addAction behavior and stable identifiers. Add narrowly scoped contextual-action support only if necessary for actual Network editor operations.
- PASS: actions appear only in appropriate contexts, do not conflict, and no parallel registry is introduced.

### C4 — Isolate EFSM projection/renderer/codec integration
- Keep shared serializer/scene orchestration generic; move subtype-specific EFSM branches behind a registered GUI-only projection/persistence contribution where supported, with safe legacy load/round-trip.
- PASS: EFSM topology/layout/undo/remove/restore tests unchanged and no new hardcoded EFSM dispatch in the generic host.

### C5 — Minimal network-editor contribution contract
- When D2 first requires a second formalism, add the smallest optional descriptor supporting formalism identification, allowed tools, action predicates, projection/refresh and graphical persistence. Avoid universal virtual interfaces with irrelevant methods.
- PASS: EFSM and Graph concrete editors use the same host without explicit per-class branches in MainWindow.

### C6 — Static extension integration verified
- Keep registration static, optional dependency behavior, deterministic dispatch and stable IDs. Disable/remove a graphical extension without affecting simulation kernel behavior or corrupting its persisted graphical records.
- PASS: GUI and focused unit smoke pass with/without the optional contribution as feasible; no Qt/GUI header in kernel/simulation plugins. Do not implement dlopen/QPluginLoader.

### D1 — Enforce context-specific editor tools
- In process context, preserve ModelComponent palette and process connections. In a Network context, expose only the registered formalism's operations and mathematical items; reject inappropriate drag/drop at the actual scene insertion boundary, not only by hiding icons.
- Domain classes remain their actual C++ categories: FSMState and GraphNode are data definitions; EFSMTransition/MarkovTransition are relation objects, GraphEdge and PetriPlace/CPNTransition/CPNArc are data definitions; no fake ModelComponents.
- PASS: ordinary process components cannot be dropped into network formalism canvas; legitimate formalism-specific actions remain available; no topology/ownership mutation on rejected drop.

### D2 — Complete Graph, directed graph and DAG editors
- Render and edit GraphNode/GraphEdge, geometry and edges, weighted/parallel/self-loop cases as supported by the actual domain; distinguish undirected/directed/DAG rules.
- Preserve GraphNetwork structural meaning, graph algorithms and negative-weight/cycle validation. Add per-network .gui graphics round-trip with stable identities and undo/redo for topology changes.
- PASS: create/edit/delete/undo/redo, save/reopen and mathematical graph tests, including DAG cycle rejection, without introducing Entity routing.

### D3 — Complete MarkovChainNetwork editor
- Render/edit MarkovState and its real MarkovChainNetwork::MarkovTransition representation, initial/current state and available transition probabilities using existing API and ownership rules.
- Support domain-valid stochastic rows and reproducible activation. Add projection, property editing, undo/redo, graphical save/reload and invalid-probability diagnostics.
- PASS: round-trip and deterministic seeded DTMC execution unchanged; diagram matches domain and allows creating/editing all supported mathematical topology.

### D4 — Complete ColoredPetriNetNetwork editor
- Render/edit PetriPlace, CPNTransition and CPNArc with bipartite constraints, markings, arc/inscription/guard capabilities limited to the actual implemented CPN subset.
- Add projection, property editing, undo/redo and graphical save/reload, preserving model-level membership and domain ownership.
- PASS: supported marking/firing and persisted topology tests, bipartite edge rejection, graphical round-trip and no process Connection substitution.

### D5 — Visual animation and event/state update for all supported formalisms
- Reuse existing SimulationEventController/kernel observation paths; add a small GUI-thread-safe dispatch/observation bridge for Network rendering when required. No simulation-plugin Qt dependencies or GUI callbacks escaping object lifetime.
- EFSM: visually indicate current state and fired transition from actual network execution.
- Markov DTMC: visually indicate sampled current state and transition; represent stochastic choices faithfully and reproducibly, never animate unexecuted candidates as executed.
- CPN: show real marking changes and fired transition/arc effects consistent with the actual supported runtime.
- Graph family: no fabricated simulation-time traversal; implement truthful visualization of structural algorithm results (e.g., selected BFS/DFS/shortest path) as analysis playback or highlighting, clearly differentiated from runtime simulation. An inactive structural graph has no fictitious simulation event animation.
- Handle start/pause/step/restart/replications and tab/model close; avoid GUI work on kernel threads; throttle only when doing so does not falsify event-by-event meaning.
- Preserve associated graphical configuration across .gui save/reopen and absent-extension round-trip, using the approved GUI extension persistence principles.
- PASS: a focused runnable GUI fixture per formalism demonstrates the appropriate visual state/result; simulation output before/after graphical extension is unchanged; replay/step and shutdown do not leak callbacks or use stale pointers.

## 6. Verification protocol — necessary and sufficient

Do NOT run the entire repository test matrix after every small edit. For each milestone:
1. Build its touched targets with the actual repository CMake + Ninja presets and C++ standard.
2. Run the closest existing Google Test/Qt tests and add only targeted success, failure and regression tests for the changed contract.
3. When GUI changes, confirm the gui-app target builds and run its focused headless Qt test or an equivalent reproducible UI interaction.
4. Run tests-unit, tests-kernel-unit and tests-smoke at substantial integration checkpoints and at the final branch HEAD, subject to actual available configured presets; do not claim PASS if not executed.
5. Use AddressSanitizer/UBSan only for changed ownership/lifetime paths or observed faults. Check Qt GUI thread affinity for animation/event changes.
6. Record exact commands, targets, observed results and residual limitations in a short markdown progress log. Historical evidence does not substitute for testing current branch changes.
7. Fix failures introduced by the plan before proceeding, without expanding into unrelated cleanups.

Minimum scenarios: absent Network; explicit subtype creation; existing subtype selection; type/name display; duplicate-name and missing identity; replacement with incompatible port counts; cancellation; shared two-adapter Network; Open Network disabled/enabled; ModelDataDefinition context open; EFSM saved/reopened execution; Graph directed/DAG, Markov stochastic and CPN firing persistence; invalid drag/drop; undo/redo; visual animation of actual state; lossless missing graphical extension. Explicitly check both .gen mathematical model and .gui visual overlay.

Gate outcomes are PASS / FAIL / UNKNOWN. Do not mark UNKNOWN as PASS. Treat genuinely unresolved in-scope failing features as blockers and continue with independent milestones if safe; final completion requires every authorized capability implemented or a clearly reported, substantive blocking deviation. Stop for maintainer only if an essential design decision would change mathematical semantics, ownership architecture, persistence compatibility without safe migration, plugin ABI policy, security/cost constraints or another explicit boundary.

## 7. Local agent workflow and progress bookkeeping

- Start at the repository root; verify a clean worktree. Preserve any local changes with an explicit safe backup/stash or separate worktree; never reset, delete or overwrite uncommitted work.
- Fetch and synchronize from origin/WorkInProgress. Verify source base and documentation; use a dedicated branch, suggested WiP20261112/modal_model_gui (valid alternative if unavailable or conflicting). Do NOT implement on WorkInProgress.
- Create a concise progress file, suggested docs/ai_assistants/history/evidence/2026/11/2026-11-12_modal_network_gui_implementation_progress.md, with a milestone checklist [ ]/[x], source commits, commands, pass/fail and outstanding items. Keep it updated at meaningful boundaries, not on every function.
- Make small coherent commits with milestone prefixes (e.g., modal-gui(A1):, modal-gui(B2):) and source/test changes grouped intelligibly; avoid one giant end-of-task commit.
- Continue G0 through D5 autonomously. No intermediate architecture discussion or approval for covered decisions. Avoid excessive tests, cosmetic refactors, repetitive status messages and unrelated PRs.
- Maintain a current progress summary so the maintainer can assess scope and remaining work. Pushing the development branch is allowed once safe and authenticated; do not force-push or rewrite other contributors' history. A draft PR is optional, not a substitute for completion.
- On completing ALL milestones, run final relevant unit/kernel/smoke + GUI verification, check .gen/.gui round-trips and key runtime demonstrations, and report commit/branch, validated tests, unresolved risks and precise final review steps.
- Leave the branch ready for maintainer review. Do NOT merge into origin/WorkInProgress automatically.

## 8. Document coordination, cross-plan boundaries and definition of done

Decision HUM-MODAL-003 authorizes this scope; executable task AUTO-MODAL-005 is the coordinating backlog entry. It does not retroactively alter the accepted 8/8 EFSM MVP status. GUI extension task AUTO-GUI-003 remains independently gated for OTHER plugin families; use its approved existing contracts only where needed for the ModalNetwork vertical slice.

During implementation, reconcile touched docs (especially MODAL_NETWORK_GUI_ARCHITECTURE.md's obsolete create-on-double-click wording) and update STATUS/backlogs/evidence at appropriate boundaries; maintain traceability to this plan. Do not declare broad graphical-extension framework completion if only the ModalNetwork subset is implemented.

**DONE only if:** A1–A5, B1–B5, C1–C6, D1–D5 are implemented in code (or B5 is explicitly documented as unnecessary), each has focused validation, the final supported GUI and model workflows work, no known in-scope regression is left unfixed, the progress report gives a traceable matrix, and the development branch is ready for human review. Merely writing a plan, passing compilation, updating docs, or creating a draft PR is NOT completion.
