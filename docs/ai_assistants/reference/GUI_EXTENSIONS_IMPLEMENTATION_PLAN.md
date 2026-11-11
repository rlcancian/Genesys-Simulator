---
document_type: implementation-plan
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-11-11
status: active-execution-revised-v2
tracks: AUTO-GUI-003
---

# Main GUI Graphical Extensions — Implementation Plan (v2, 2026-11-11)

## 0. Human-directed revision and change history

**Version 1:** approved and recorded 2026-11-11 with eight steps `G0–G7`. Its architecture, intended feature set, module boundaries and deferred dynamic-loading constraints remain authoritative via [GUI_EXTENSIONS_ARCHITECTURE.md](GUI_EXTENSIONS_ARCHITECTURE.md).

**Version 2, 2026-11-11 — explicit maintainer decision `HUM-ARCH-005`:** the first execution revealed an **overly burdensome audit and validation strategy**. Multiple agent sessions completed preparation, inventory, local GUI configuration/build and tests, yet no graphical feature was implemented because generated dependency graphs, repeated broad baselines, evidence consolidation and reporting were treated as serial *prerequisites*. This was an **error in the execution plan** relative to the objective of making functional progress. The maintainer instructed that work prioritize *actual implementation*, with a **small, coherent, impact-directed test set at each step**, then deeper regression and assurance when a demonstrably functional product exists.

This is a change of **execution sequence and validation depth, not acceptance of known bugs or permission to misstate results**. Version 2 supersedes Version 1's repeated pre-implementation checks, exhaustive per-stage test matrix and serial stop gates; original G0–G7 functional goals remain. A failure in a directly affected path must be fixed or that path explicitly disabled; no red/UNKNOWN evidence may be reported as PASS. The [targeted governance provision](../GOVERNANCE.md) applies to this **experimental graphical-extension branch only**; normal merge/release gates remain mandatory.

## 1. Goal, scope and operating rules

Implement optional graphical extensions of the main Qt6 GUI that provide registered menus/actions/toolbar entries, windows/docks, model-scene animations, component renderers and alternative GUI services, while preserving kernel/parser/simulation-plugin independence from Qt. Generic GUI host must not import *new* optional concrete plugin headers or require plugin-specific switch/enum branches when adding an extension. Existing concrete couplings are migrated incrementally, not removed via one risky rewrite.

Current environment: Ubuntu 24.04, C++23 where configured, Qt6, CMake/Ninja and Google Test. Work on `WiP202611/gui-extensions-architecture-implementation`, starting from existing draft [PR #575](https://github.com/rlcancian/Genesys-Simulator/pull/575) and the current remote head; no fresh parallel implementation branch for the same task without an explicit justified split. Prior history: PR #574 merged architecture docs; the source-backed [G0 inventory](../history/evidence/2026/11/2026-11-11_gui_extensions_g0_inventory.md) exists.

**Never expand the scope into:** `dlopen`/`QPluginLoader`, dynamic unload, hot reload, ABI/SDK implementation, separate shared targets, a parallel scene/editor or framework, broad directory moves, general EFSM/Decide/Property Editor cleanup, model/kernel science or parser rewrites, automatic dependency installers, wholesale application of student PR #436. Keep static registration now, with stable extension/type IDs and factories compatible with future dynamic loading.

**Continuous execution:** after each small code slice, build its affected target, run the relevant focused tests and continue if they pass. Commit meaningful functionality, push to GitHub at each functional checkpoint, and keep PR #575 accurate; do not stop for routine documentation handoff or seek repeated maintainer confirmation. Diagnose, correct and retry focused failures. Stop only for actual inability to proceed safely, material design choice not covered by this architecture, potentially destructive behavior, or a governance gate requiring human approval.

## 2. Two levels of validation — deliberately lean before integration

### 2.1 Development loop (G1–G6)

**One fast check set per functional slice, not per edit or file:**

1. Configure/build the affected `gui-app` or existing focused GUI test target using CMake/Ninja (reuse a valid build tree; do not repeatedly rebuild everything from scratch).
2. Run **1–3 focused automated tests** exercising the changed behavior (one happy path, one meaningful failure/absence case, and one state/persistence case if relevant). Reuse existing GUI-GTest harnesses and test targets.
3. Perform one short Qt offscreen or interactive scene check **when** the slice changes menu/toolbar placement or scene interaction; screenshots are not a substitute for action/scene behavior checks.
4. Record command, result and commit in a **compact existing evidence/progress file**; do not create another full audit or lengthy log dump. An intermediate proof is `PASS for its measured scope` only, not production assurance.

**Do not require at every slice:** full generated CMake File API graph; complete transitive header inventory; all ~2,000 unit tests; kernel/smoke suite when untouched; repeated sanitizer, ABI, packaging, PDF generation, every exception test or exhaustive dependency matrix. No arbitrary test-count quota beyond the bounded relevant scenarios.

**Safety exceptions:** if changing kernel/plugin semantics, event ordering, persistence loss behavior, ownership/lifetime, or API compatibility, add the **specific focused negative/regression test before accepting that change**; use a sanitizer only for a genuine lifetime risk. Never silence failing relevant tests, change CI to make checks optional, discard unknown extension data, or omit a required security check.

### 2.2 Integration/assurance loop (after the first functional MVP, at G7)

Before declaring the completed implementation ready for merge: build `gui-app`, run relevant focused GUI interaction and persistence tests and **ordinary `tests-unit` regression** on the final source head; use `tests-kernel-unit`/`tests-smoke` if their code paths or runtime behavior were affected; targeted ASan/UBSan/LSan when ownership changes; required CI, documentation-governance checks and manual-impact review. Record failures, baseline exceptions and results. Mandatory repository checks still govern merges. **Do not merge a failing/unknown mandatory check without an explicit documented waiver.** Defer release-grade ABI/unload/packaging/performance/security matrix to the separate future feature/release process, not to a false `done_confirmed` claim.

## 3. Fast development sequence (same eight milestones G0–G7)

### G0 — Short baseline, then start coding

**Already performed:** GitHub source/coupling map, before/after ownership/persistence matrix and test candidates in the G0 inventory; a previous CI baseline passed, and the local Codex session **reported** successful GUI configuration/build and `tests-unit` execution but did not publish command logs/commit. These local results are **reported, not independently verified**. The generated comprehensive File API/include graph remains `UNKNOWN` and is **not** a blocker for the first registered animation.

**Closure rule:** verify from the current workspace or retained terminal logs that (a) the relevant GUI target builds and (b) at least the existing impacted GUI tests run, or rerun only the absent narrow check; add a concise G0 result (SHA, command, pass/fail/unknown) to the existing G0 inventory; commit/push. A full generated target/include graph and 100% repository include census are **optional deferred evidence**, to revisit if a concrete compilation/dependency conflict emerges. A missing optional graph **must not cause another session to terminate without implementing G1**.

Do not declare unobserved tests PASS. If local evidence is unavailable, execute a minimal GUI build and focused GUI suite immediately rather than restarting the audit.

### G1 — Existing extension contract: registry and dependencies

- Preserve `GuiExtensionPlugin` as a lean pure abstract C++ interface and existing `registerContributions`. Reuse `GuiExtensionRegistry`, `GuiExtensionManager` and `GuiExtensionPluginCatalog`.
- Add only the metadata, minimal typed dependency categories (model, graphical extension, system capability) and registration IDs needed by the first animation; keep `requiredModelPlugins()` backward compatible. Validate missing required model/GUI dependency and duplicate IDs deterministically; absence disables contribution without changing simulation.
- Do not implement a general dependency installer, dynamic library manager, complex resolver, or broad replacement of `GuiExtensionRuntimeContext`.
- **Lean gate:** GUI target builds; focused tests for valid registration, duplicate/missing dependency and no-kernel-dependency case. Next G2 may begin immediately.

### G2 — Registered graphical animation/factory, menu and toolbar (first visible feature)

- Register an animation **descriptor plus type-specific factory** as a contribution rather than hardcoded `DrawingMode`/`createPlaceholderAnimation` branch. Use the existing scene/editor, action creation, placement and undo mechanisms; avoid a second QGraphicsScene or per-plugin menus in `MainWindow`.
- Place the action in `View → Animations` and an existing animation toolbar, without a new per-plugin host QAction declaration.
- Supply a separate, statically registered graphical extension under `source/applications/gui/genesys/extensions/plugins/`; it produces at least one **real type-specific QGraphicsItem/AnimationPlaceholder-derived visualization** with visible state, not merely a menu message. Reuse legacy animation bases only if compatible.
- **Lean gate:** GUI build, one focused registered-tool/in-scene insertion test (position and selection), and one undo/remove/re-add or repeated-insertion test. Execute a single bounded GUI interaction check. Preserve existing drawing modes until migrated.

### G3 — Minimal event updates and safe graphical persistence

- Extend existing GUI-side `SimulationEventController` (or a narrowly scoped GUI-owned adapter) to forward the minimum safe event/snapshot needed by the MVP animation, with correct GUI-thread delivery. Avoid direct per-item kernel callback subscriptions and new kernel dependencies.
- Preserve existing `.gui` animation records. Save/reload the new registered animation's common and extension-owned state with stable IDs, schema and target identity.
- Implement **lossless retention of unknown/absent-extension graphical records by default**; avoid data destruction on ordinary save. A user-confirmed discard UI may be completed after the first retention path, but do not offer a destructive implicit fallback. Do not claim preservation without a test.
- **Lean gate:** one known animation save/reload test, one missing-extension `load→save→reinstall→reload` opaque record test, plus one event-driven GUI-state update check. Proceed as soon as relevant tests pass; defer exhaustive corrupt-field variants.

### G4 — First real simulation-domain extension, then optional legacy migration

- Move **at least one** existing simulation-domain visualization into a concrete graphical extension; priority candidates Queue/Resource/Station. Choose the lowest-risk candidate based on current code, record the choice succinctly and keep its actual kernel data/GUI behavior semantics. The feature must appear in the **model scene**, not exclusively a standalone window.
- Add a second existing domain family only if its migration is small and nonblocking; complete Queue/Resource/Station incrementally as subsequent deliverables.
- Keep `GraphicalModelComponent` Queue-pointer coupling and current legacy modes until their replacement is actually tested; do not make removal of every preexisting plugin include an MVP gate. Each newly added extension should avoid new host-side concrete includes.
- **Lean gate:** real model fixture updates the in-scene visualization; one bounded relevant state transition test and save/reload regression. Scientific simulation result must not change.

### G5 — Biological/cellular scene visualizations

- Adapt the existing `BacteriaColony` and `CellularAutomata` graphical viewers, in scoped increments, to contribute animations insertable into `ModelGraphicsScene` with stable target identity; prefer reuse of their existing rendering and state access rather than a new visualization stack.
- Existing Tools/Extensions windows remain available until scene version reaches equivalent essential behavior. Do not couple this task with a new CA/DefaultNetwork model backend.
- **Lean gate per family:** one actual in-scene update plus one save/reload/disable-and-reenable check. Record missing optional features; do not call unfinished migration complete.

### G6 — Renderer/provider extensibility

- Add small registered component-renderer and service-provider contracts only as demanded by actual examples, retaining Classic/Modern host fallback.
- Demonstrate one alternative representation and one selectable GUI service (report/chart/Expression Builder or simpler existing contract-compatible service), with deterministic fallback when disabled.
- **Lean gate:** focused provider selection/fallback and style rendering tests; GUI builds and existing default remains available.

### G7 — Consolidation, documentation and integration

- Reconcile code, architecture docs, status/backlogs, User/Developer Manual impact and evidence in the existing governance structures; only publish PDFs when actual affected manual changes require them.
- Run the **integration/assurance loop `2.2** once on the final candidate head, with reasonable diagnostics if a suite fails. Review PR #575 (or bounded follow-on PRs if the existing PR was already integrated), required checks and compatibility. Keep draft until ready.
- Close individual milestones only for demonstrated behavior. `AUTO-GUI-003` reaches `done_confirmed` only with its complete approved scope, actual tests and required review/integration; partial MVP is not `done_confirmed`.
- No automatic stable-branch promotion or undocumented waiver.

## 4. Expanded MVP — deliver functional breadth before full assurance

A **meaningful interim MVP** is reached after G1, G2, the minimal G3 slice **and at least one domain-specific in-scene visualization from G4**. It must visibly demonstrate:

1. A statically registered, optional graphical plugin with stable ID and declared model/GUI/system dependency metadata;
2. A plugin-contributed menu + animation toolbar tool, whose factory creates a type-specific movable/selectable scene item **without new domain-plugin includes or branches in generic host**;
3. Live/current simulation or model-state-driven update through the GUI-side path, not only a static rectangle;
4. Save/reopen of that item, including extension data, plus preservation on re-save with the graphical extension absent;
5. A **real simulation-domain example** whose visual state reflects a model object, not an inert demonstration;
6. Bounded tests for registration, creation, state update and round-trip, plus one GUI build and a short interaction check.

**MVP excludes:** full migration of all existing animations and all bio/cellular viewers, all interchangeable services/renderers, dynamic loading, hot-unload safety, exhaustive unrelated regression suite at each intermediate commit and full performance/ABI validation. These remain G4–G7 or future scoped work; they are not silently discarded.

Allow staged implementation **in the same workstream**: `MVP-A` is registered tool+factory+visual update (G1/G2 + partial G3) and `MVP-B` adds retained persistence and one real domain extension. Do not stop work after MVP-A merely to request another prompt; implement MVP-B once focused tests permit.

## 5. Minimal reporting and Git collaboration

- Use the existing `AUTO-GUI-003` backlog, `STATUS.md` and the [G0 evidence](../history/evidence/2026/11/2026-11-11_gui_extensions_g0_inventory.md) for G0; add concise incremental evidence under the existing `history/evidence` mechanism when implementation creates new findings. Prefer one short incremental implementation record, **not** a separate long report for every test or edit.
- Commit real working slices and push at milestone checkpoints; put changed paths, relevant command/results and remaining limitations in commit/PR text. Preserve uncommitted user changes; use the existing worktree when available. Do not reset, clean or force push.
- Use PR #575 as the active continuation vehicle; update its title/body from G0-only to reflect the new implementation scope. Avoid duplicate PRs. Keep it draft while code is evolving; follow normative review/merge policy when ready.
- Do not block on canceled historical CI jobs if the current local **scoped** tests pass; do not call those canceled checks passed. Review current final-head mandatory CI before merging.
- When necessary to prevent a destructive compatibility change, apply a narrow stop gate, document the obstacle and proceed with independent functionality rather than broad refactoring.

## 6. Deferred dynamic loading/unloading (unchanged)

Dynamic graphical plugin packaging, `dlopen`/`QPluginLoader`, unload guards, ABI stable C interface with a Qt adapter, cross-version compatibility, ref counts, delayed callbacks, library teardown and hot reload remain **future-only** requirements, not tasks for G0–G7. Existing core package policy requiring a stable versioned C ABI remains in force. Maintain a plausible future boundary with registered factories/stable IDs and application-local code; do not implement an ABI or promise binary independence now.

## 7. Governance status and next execution

This is the **approved v2 execution plan**, effective immediately under `HUM-ARCH-005`, not a new plan replacing the approved architecture. It resolves the previously blocking G0-generated-graph condition without asserting that missing evidence exists. It authorizes an executor to move directly from minimal current baseline to functional G1/G2 code, continuing to G3 and the first G4 domain example without new routine human check-ins.

If a material **new architectural choice**, data loss, security change, broken relevant test, merge blocker or external authorization is required, stop that impacted path and report it precisely; otherwise prioritize implementation. Any required full-suite regression and manuals/CI/PR checks remain part of G7 rather than serial prerequisites before every source file.
