---
document_type: backlog
authority: executable-task-source
owner: project-maintainer
last_updated: 2026-09-18
review_cadence: on-status-change
status: active
tracks: 511
---

# GenESyS Autonomous Agent Backlog

## 1. Purpose

This is the only approved source for work an AI agent may execute without a new material human decision. A task may run only when its status is `ready`, its environment is available, dependencies are resolved and no maintainer freeze excludes it.

Detailed executed evidence belongs under `history/evidence/`; durable technical design belongs under `reference/`. Historical task entries below retain enough provenance to identify what was integrated without turning a merged PR into proof that a broader subsystem is complete.

## 2. Status values

- `ready` — fully specified and eligible for execution;
- `running` — actively owned by one bounded branch/PR or local worktree;
- `blocked-review` — implementation/evidence prepared but waiting for required review;
- `blocked-dependency` — cannot proceed until another technical dependency or approved prerequisite is resolved;
- `paused` — planned/executable work intentionally not active; explicit maintainer activation is required;
- `closed` — not currently being developed; **no completeness, correctness or validation claim is implied**; a closed task may later be re-evaluated, resumed, replaced or found incomplete;
- `done` — the explicitly bounded task scope was accepted, validated at its declared level and integrated; this does **not** certify that a broader feature, subsystem or initiative is complete;
- `done_confirmed` — strongest completion state: all currently approved required scope for the bounded task/initiative is implemented, functioning, tested and verified at the final relevant commit, required documentation/evidence is synchronized and no required non-deferred item remains;
- `cancelled` — intentionally removed from the plan.

GitHub issue/PR state `closed` is transport/repository metadata and must not be translated into `done` or `done_confirmed` without the evidence required by this backlog.

No historical task is upgraded to `done_confirmed` merely because an older PR merged or an older test run was green. The status must be supported by current-plan acceptance criteria and final-commit evidence.

## 3. Completed documentation migration

### AUTO-DOC-001 — Establish canonical governance layer

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #512
- Merge: `958cdc6f63c02d004f1ffdf55e104b58a245bb88`
- Validation: run `29929616027`, ordinary tests and GUI GMDD green.

### AUTO-DOC-002 — Consolidate normative governance and architecture

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #513
- Merge: `b48697e77d39b25cafc19271ce574bdead60f94d`
- Validation: run `29931594603`, ordinary tests and GUI GMDD green.

### AUTO-DOC-003 — Consolidate current state and plans

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #514
- Merge: `53b49f7518509823fe2265a3f017b5aa76f09d2f`
- Validation: run `29933330431`, ordinary tests and GUI GMDD green.

### AUTO-DOC-004 — Consolidate executed evidence

- Priority: `P1`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #515
- Merge: `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca`
- Validation: run `29934227250`, ordinary tests and GUI GMDD green.

### AUTO-DOC-005 — Consolidate technical references and active root

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #516
- Merge: `d375d9e68e5c1dc84e214a772fb15cb05944f0d8`
- Validation: run `29936506990`, ordinary tests and GUI GMDD green.
- Result: technical references, canonical routing and removal of superseded active guides/plans/redirects.

### AUTO-DOC-007 — Consolidate retained oldies governance

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #517
- Merge: `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5`
- Validation: run `29938004455`, ordinary tests and GUI GMDD green.
- Result: one tracker for 25 retained files; every file remains review-pending and not deletion-ready.

### AUTO-DOC-006 — Enforce documentation governance

- Priority: `P0`
- Status: `done`
- Environment: `github` or `local`
- Issue/PR: #511 / #518
- Merge: `610d8ab21c87cfd11663af78370b39262cf4da81`
- Validation:
  - documentation-governance run `29938886903`: passed;
  - ordinary CI run `29938886807`: configure, build, CTest and GUI GMDD passed.
- Result: `scripts/validate-ai-docs.py` plus GitHub Actions enforce the governed structure.

## 4. Integrated bounded work and historical checkpoints

### AUTO-APP-003 — Implement per-user runtime launcher and dispatcher

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #521
- Merge: `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`
- Validation: Launcher CI green (33 focused tests, dispatcher isolation, install contract), ordinary regression CI green on final PR head.
- Result: `genesys-launcher`/`genesys-dispatch`, XDG runtime selection/update validation, public application dispatch and system-fallback contract integrated.
- Remaining boundaries such as runtime-release signing/publishing are separate tasks/decisions.

### AUTO-PKG-001 — Execute Debian package lifecycle validation

- Priority: `P1`
- Status: `done`
- Environment: `github` and `local`
- Issue/PR: #522
- Merge: `d8fce9562617525657e8cbf9870b32323364773f`
- Validation: GitHub Actions run `32421072334` plus disposable Ubuntu 24.04 local/container lifecycle validation; five packages, Lintian/AppStream and full install/reinstall/remove/purge lifecycle green.
- Result: Launcher/dispatcher integrated into the Debian package split; runtime release signing/publication remains out of scope.

### AUTO-ARENA-001 — Close Arena compatibility audit for Advanced Transfer and parser-variable scope

- Priority: `P1`
- Status: `done`
- Environment: `local`
- Branch: `WiP202608/arena-advanced-transfer-closeout` (merged, deleted)
- Merge: PR #527, 2026-08-30, merge commit `ebd6f30e8c662ae74da6f5745472235090a830ca`.
- Delivered: Advanced Transfer/material-handling bounded additions, Flow Process classification and parser/Arena variable-scope mapping.
- Validation at merge time: focused Arena/MaterialHandling tests 28/28 and `tests-smoke` 3/3 green; kernel/unit limitations unrelated to the bounded scope remain recorded in status/history.

### AUTO-MODAL-001 — Migrate ModalModel onto the Network bridge

- Priority: `P1`
- Status: `closed`
- Environment: `local`
- Branch: `WiP202608/modal-network-implementation` (historical implementation branch)
- Merge: PR #528 (`e0185163`, documentation/architecture consolidation) and PR #529 (`fdae135b`, implementation phases), merged 2026-08-31 into `WorkInProgress`.
- Historical scope integrated:
  - `DefaultNode` migration toward the network/data-definition boundary;
  - `DefaultNetwork` activation frame/result, port schema and activation counter;
  - `ModalModelDefault` network bridge and input/output binding path;
  - `EFSMNetwork`;
  - structural `GraphNetwork` / `DirectedGraphNetwork` / `DirectedAcyclicGraphNetwork` family;
  - finite time-homogeneous `MarkovChainNetwork` DTMC support;
  - pragmatic fixed-inscription `ColoredPetriNetNetwork` subset;
  - replacement of the exercised legacy `std::rand()` path with GenESyS sampler infrastructure;
  - focused unit tests for the introduced network types and bridge behavior.
- Historical executed evidence: the final recorded 2026-08-31 checkpoint reports full build green and `tests-kernel-unit` 1810/1810 executed tests passed, 0 failed, with four pre-existing disabled tests.
- Interpretation of this status: the historical development session is no longer active. `closed` **does not mean the broader ModalModel/DefaultNetwork architecture is complete** and does not constitute `done_confirmed`.
- Current-head verification: **pending local confirmation**. The 2026-09-18 GitHub-only reconciliation did not build/run the current `WorkInProgress` HEAD.
- Superseded historical requirement: compatibility with old persisted ModalModel `.gen` files is not a requirement for the continuation work.
- Continuation: `AUTO-MODAL-002` and [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md).

## 5. Active bounded work

None currently.

`AUTO-MODAL-002` is intentionally `paused` until the maintainer starts the local continuation. `AUTO-MODAL-001` is `closed` because its historical development session ended; that status makes no completeness claim.

## 6. Paused technical tasks

These tasks remain paused until the maintainer explicitly activates one. Completion of another task does not resume them automatically.

### AUTO-MODAL-002 — Complete and verify ModalModel / DefaultNetwork architecture

- Priority: `P1`
- Status: `paused`
- Environment: `local`
- Source plan: [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md)
- Maintainer decisions already recorded: `HUM-MODAL-001` through `HUM-MODAL-005`.
- Starting point: current `origin/WorkInProgress`; the local agent must record the actual HEAD before doing any work rather than assuming the 2026-09-18 remote checkpoint remains current.
- Purpose: verify the current implementation against the approved network-centered architecture and complete only gaps demonstrated by current code, build, tests and runtime evidence.
- Guidance, subject to local verification:
  - re-establish `DefaultNetwork`/`DefaultNode` ownership, port, activation, check, reset and current-format persistence contracts;
  - re-establish `ModalModelDefault` input/output binding, presence, activation, zero/one/multiple-output entity handling, cloning/consumption and persistence contracts;
  - implement/verify the maintainer-approved three-mode EFSM conflict policy;
  - verify Graph supported invariants/algorithms without adding implicit Entity movement;
  - mathematically validate current DTMC semantics, RNG, reset and persistence;
  - define and fully validate the pragmatic GenESyS CPN subset without automatically expanding to full CPN semantics;
  - inventory current uses of legacy ModalModel classes, migrate current-required callers, and then quarantine under `ModalModel/deprecated/` or delete when locally justified;
  - before moving `.cpp` files under `deprecated/`, correct the current recursive component-source discovery so that deprecated sources are actually excluded from the build;
  - ensure focused unit tests exist for every supported network type and fill real gaps only;
  - use required aggregate regressions and focused ASan/LSan/UBSan where ownership/lifetime paths change;
  - defer broad GUI editing until the backend completion gate is satisfied; then revalidate `MODAL_NETWORK_GUI_ARCHITECTURE.md` against the current GUI before implementing it.
- Explicit non-requirements for this task:
  - historical `.gen` compatibility/migration;
  - CTMC;
  - MDP/controlled/time-inhomogeneous Markov models;
  - dynamic graph traversal/random walk/routing/entity movement;
  - automatically completing every advanced CPN feature;
  - Cellular Automata migration itself (planned separately after this architecture is `done_confirmed`);
  - broad dynamic-plugin or unrelated CMake refactoring.
- Acceptance for `done_confirmed`:
  - every required, non-deferred item in the source plan is either demonstrated already satisfied or implemented and verified at the final current commit;
  - focused unit tests are green for all supported network types and the ModalModel adapter;
  - required ordinary/kernel/smoke regressions are green, or unrelated baseline failures are precisely characterized according to governance;
  - current-format persistence round trips and plugin/factory registrations are verified;
  - stochastic semantics are reproducible under controlled seeds;
  - relevant ownership/lifetime changes have focused sanitizer evidence;
  - required backend/GUI integration in the approved current scope is functional and validated;
  - documentation, evidence, backlog/status and manual impact are synchronized;
  - no required non-deferred gap remains classified only by assumption.
- Stop/reassess conditions:
  - current code already supersedes a recommendation in the source plan;
  - a proposed legacy move breaks a still-valid current contract;
  - a new material scientific/architectural decision is discovered;
  - a broad unrelated refactor would be required;
  - test/sanitizer evidence reveals a larger defect boundary.

### AUTO-APP-001 — Validate standalone HTTP Worker GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: preset/build, PID-associated window, bounded liveness, controlled teardown and ordinary CI green.
- Non-goal: no worker security redesign.

### AUTO-APP-002 — Validate standalone main GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: `gui-app` preset/build/Xvfb startup evidence before minimal interaction work.

### AUTO-TEST-001 — Remove four historical duplicate Search/Remove test blocks

- Priority: `P2`
- Status: `paused`
- Environment: `local`
- Acceptance: active focused tests remain green, exact inventory updated and ordinary/kernel/smoke paths green.
- Stop: connector-only environments must not replace the large source file blindly.

### AUTO-QT-001 — Remove active Qt5 fallback

- Priority: `P1`
- Status: `paused`
- Environment: `local` preferred
- Acceptance: active references mapped, Qt6 presets/tests green and no GUI redesign.

## 7. Completed technical baseline

Do not reopen bounded completed work without new evidence:

- CI trigger corrections and AI test aggregation;
- Phase 0 kernel/smoke workflow;
- solver contract stabilization;
- active Search/Remove coverage;
- Queue/Station/Delay/Resource lifecycle corrections;
- focused plugin-completion ownership sanitizer;
- optimizer copy/move barrier;
- plugin target/codemodel/link evidence;
- shell, worker, Data Analyser, Optimizer and AI Assistant startup validations;
- per-user runtime launcher/dispatcher (`AUTO-APP-003`, PR #521) and its integration into the Debian package with lifecycle validation (`AUTO-PKG-001`, PR #522).

This section is a baseline index, not a claim that broader product areas are `done_confirmed`.

## 8. Activation and completion rules

A paused task becomes eligible only after the maintainer changes it to `ready` and confirms scope, validation and stop conditions.

A `closed` task is inactive and makes no completion claim. Before resuming it or deriving a replacement task, re-check the current code and current plan.

A task moves to `done` only after its explicitly bounded required validation is green, evidence is reviewed, the PR/change is integrated as applicable, status/backlog/changelog are synchronized and remaining boundaries are explicit. `done` must never be extrapolated to a broader subsystem or initiative.

A task or initiative moves to `done_confirmed` only after the complete currently approved **required, non-deferred** scope has been verified at the final relevant commit, including build/tests/runtime/scientific or functional evidence appropriate to that scope. Optional future extensions do not block `done_confirmed` unless they are promoted into the current approved scope.

If evidence is unavailable, use explicit language such as `verification pending`, `current status not confirmed`, or identify the exact test/inspection required. Never upgrade status by inference.