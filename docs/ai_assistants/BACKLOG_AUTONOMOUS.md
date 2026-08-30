---
document_type: backlog
authority: executable-task-source
owner: project-maintainer
last_updated: 2026-08-30
review_cadence: on-status-change
status: active
tracks: 511
---

# GenESyS Autonomous Agent Backlog

## 1. Purpose

This is the only approved source for work an AI agent may execute without a new material human decision. A task may run only when its status is `ready`, its environment is available, dependencies are resolved and no maintainer freeze excludes it.

## 2. Status values

- `ready` — fully specified and eligible;
- `running` — owned by one active branch/PR;
- `blocked-review` — prepared but waiting for review;
- `blocked-dependency` — waits for another task or decision;
- `paused` — executable but disabled by maintainer instruction;
- `done` — accepted, validated and merged;
- `cancelled` — intentionally removed.

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
- Result: six technical references, canonical routing and removal of superseded active guides, plans and redirects.

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
- Result:
  - `scripts/validate-ai-docs.py` validates the final structure locally;
  - `.github/workflows/genesys-docs-governance.yml` enforces it on relevant PRs;
  - exact root allowlist, links, front matter, backlog IDs, evidence placement and oldies retention are checked;
  - source branch removed automatically.

## 4. Completed application and packaging work

### AUTO-APP-003 — Implement per-user runtime launcher and dispatcher

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #521
- Merge: `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`
- Validation: Launcher CI green (33 focused tests, dispatcher isolation, install contract), ordinary regression CI green on final PR head.
- Scope delivered:
  - `source/applications/launcher/` with `genesys-launcher` and `genesys-dispatch` (Qt6/C++23);
  - XDG configuration/runtime paths, deterministic user/system runtime selection, update manifest/version/platform validation, bounded HTTPS transport, SHA-256 verification, fail-closed signature verification, safe archive extraction, partial install, atomic activation, retention, logging and non-shell (`execv`) process launch;
  - `genesys-gui`, `genesys-shell`, canonical `genesys-worker`, legacy `genesys-web` compatibility, and existing Python `genesys-mcp` entry point.
- Non-goals honored: no Debian package layout/wrapper/control/install-file redesign in this PR (delivered separately by `AUTO-PKG-001`/PR #522); no release bundle publication/signing workflow; no Worker authentication/network exposure redesign; no MCP rewrite or Python package installation.
- Reconciled 2026-08-20 while executing `AUTO-PKG-001`: this task was recorded `running` after its PR had already merged; re-verified against current `source/applications/launcher/` and a fresh green Launcher CI run before marking `done`.

### AUTO-PKG-001 — Execute Debian package lifecycle validation

- Priority: `P1`
- Status: `done`
- Environment: `github` and `local`
- Issue/PR: #522
- Merge: `d8fce9562617525657e8cbf9870b32323364773f`
- Authorization: this task was recorded `paused` (see historical Section 6 baseline); explicit maintainer instruction dated 2026-08-20 authorized executing it, including integrating the Launcher (`AUTO-APP-003`) into the Debian package, fixing the discovered Lintian and lifecycle-script defects, and merging when all objective criteria were satisfied.
- Scope delivered:
  - integrated the stable Launcher/Dispatcher into the Debian install tree (`/usr/libexec/genesys/`), added the `genesys-common` package (launcher, dispatcher, `/etc/genesys/update.conf`, public `genesys-mcp` entry) and split the GUI/Shell/Worker packages into public wrapper + Debian-provided system fallback, keeping `genesys-web` as a payload-free transitional package;
  - fixed real Lintian findings on GitHub Actions run `32407760833`: `depends-on-essential-package-without-using-version` (dropped an unneeded explicit `tar` dependency on an Essential package), `copyright-without-copyright-notice` (added copyright years), `no-manual-page` (added five section-1 manual pages); confirmed by re-inspection that `hardening-no-pie` and `possible-gpl-code-linked-with-openssl` do not occur on this branch (`readelf` confirms PIE binaries with no OpenSSL linkage);
  - found and fixed four real, previously unexercised defects in `packaging/linux/validate-debian-lifecycle.sh` (a `set -e`/`pgrep && fail` function-return bug, missing `sudo` on five per-user-home path checks, an insufficiently bounded GUI-startup poll, and `apt-get purge` failing to resolve locally-installed no-conffile packages by name) — the lifecycle job had never previously run to completion because the build job was always blocked earlier by Lintian;
  - validated locally (disposable Ubuntu 24.04 Docker container with `CAP_SYS_PTRACE`, matching GitHub Actions runner fidelity) and on GitHub Actions run `32421072334`: `dpkg-buildpackage` produces exactly 5 packages, `lintian --fail-on error` and `lintian` (no filter) report zero findings, `appstreamcli validate --no-net` passes, and the full lifecycle script (install, ownership, non-root user, system fallback, per-user runtime, admin-policy override, six invalid-runtime cases, file-integrity, reinstall/conffile, remove, purge) completes with exit 0;
  - updated `packaging/linux/README.md` and the Developer/User manual chapters (`chapter_packaging_and_ci.tex`, `chapter_user_installation.tex`) to document the current five-package split and the Launcher/system-fallback/per-user-runtime contract; regenerated `docs/ManualGenESyS.pdf`.
- Non-goals honored: no GitHub Release, PPA/APT repository, package signing, runtime signing key, or promotion beyond `WorkInProgress`.

## 5. Active bounded work

### AUTO-ARENA-001 — Close Arena compatibility audit for Advanced Transfer and parser-variable scope

- Priority: `P1`
- Status: `running`
- Environment: `local`
- Branch: `WiP202608/arena-advanced-transfer-closeout`
- Base: `origin/WorkInProgress` at `b4fe16289b1aa4d0924e979cf795c8cc4d562e88`
- Current branch head: `4b76a7390e5ed02075a8a21700ff0764436ae152`
- Authorization: explicit maintainer continuation instruction dated 2026-08-30 authorized resuming the advanced Arena ↔ GenESyS front from the existing checkpoint without restarting Phase A or the already-closed Basic/Advanced Process audit.
- Scope:
  - reconcile `Distance` and `Segment` with the current code, registration, tests and the compatibility matrix;
  - finish the remaining Advanced Transfer audit (`Enter`, `Leave`, `PickStation`, `Route`, `Station` as flowchart concept, conveyor stubs, transporter/network gaps) and classify Flow Process without broad re-audits;
  - execute Phase C for the real parser/Arena Variables Guide overlap only, documenting implemented, partial, unsupported and future items;
  - implement only the smallest justified fixes/additions needed to close proven in-scope gaps with tests.
- Non-goals:
  - no restart of the already completed Phase A / Basic Process / Advanced Process analysis;
  - no broad plugin-architecture refactor, dynamic-plugin migration, incidental modernization, or premature ModalModel work;
  - no implementation of unsupported Arena features merely to reach nominal 100% parity.
- Acceptance:
  - `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` reconciled with the current code and final area statuses;
  - new/changed in-scope data definitions or components registered, persistible and covered by focused tests;
  - parser/Arena variables mapping documented from the real current pipeline;
  - focused validation green, plus required aggregate regression levels for any code changes performed;
  - remaining human decisions and future features isolated explicitly.
- Progress snapshot:
  - completed on the branch: `Distance`/`Segment` reconciliation, `Route` persistence/check fixes, minimum executable `Storage`/`Store`/`Unstore`/`DropOff`, minimum executable `Conveyor`/`Transporter` plus `Access`/`Exit`/`Start`/`Stop`/`Move`, and Phase C parser/Arena variables mapping;
  - documentation updated: compatibility matrix closed through Advanced Transfer, Flow Process classification and parser-variable scope, plus dedicated maintainer decision note for `Process::AllocationType` versus internal `Delay`;
  - validation snapshot on 2026-08-30: focused Arena/MaterialHandling tests green (`28/28`), `tests-smoke` green (`3/3`), `tests-kernel-unit` green except the preexisting `genesys_test_optimizer_ownership_contract_NOT_BUILT`, and `tests-unit` still shows unrelated WholeCell/Bio failures tied to plugin-loading paths outside the Arena scope;
  - remains `running` because the branch has been pushed but not yet merged into `WorkInProgress`.

## 6. Paused technical tasks

These tasks remain paused until the maintainer explicitly activates one. Completion of the documentation migration does not resume them automatically.

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

### AUTO-MODAL-001 — Migrate DefaultNode onto the ModalModel/Network bridge

- Priority: `P1`
- Status: `running`
- Environment: `local`
- Branch: `WiP202608/modal-network-implementation`
- Base: `origin/WorkInProgress` at `f40d067fd15c3fb57275dedd9852a7fe4100b2ce`
- Current branch head: `6ed47e55`
- Authorization: explicit maintainer continuation instruction on 2026-08-30 authorized resuming the ModalModel/network architecture from the current `DefaultNetwork` checkpoint without restarting the earlier phase-0 inventory.
- Scope:
  - migrate `DefaultNode` from `ModelComponent` to `ModelDataDefinition`;
  - remove process-connection/dispatch semantics from the node layer;
  - preserve persistence and plugin registration compatibility for existing modal-model models;
  - keep `ModalModelDefault`/`ModalModelFSM`/`ModalModelPetriNet` behavior intact until the generic `ModalModel` adapter phase;
  - add focused tests for the new node contract and any persistence/check regressions touched by the migration.
- Non-goals:
  - no EFSM/DTMC/CPN formalism implementation yet;
  - no generic `ModalModel` adapter rewrite yet;
  - no broad plugin-architecture refactor outside the modal-model path;
  - no restart of the architecture phase-1/phase-2 `DefaultNetwork` work.
- Acceptance:
  - `DefaultNode` compiles as a data-definition-based modal element;
  - plugin loading/persistence remains backward-compatible for the current branch scope;
  - focused tests cover the migrated node contract and continue to pass;
  - remaining architecture follow-ups are left isolated for the next phase.
- Progress snapshot:
  - `DefaultNetwork` is already in place with network activation frame/result, port schema and activation counter;
  - `DefaultNode` has now moved to `ModelDataDefinition`, its old dispatch hook was removed, and node persistence now uses the data-definition serialization path;
  - `ModalModelDefault` now has an optional `DefaultNetwork` bridge with network reference persistence, input/output bindings, activation, zero-output consume semantics, one-output routing, and multi-output cloning;
  - `tests/unit/test_default_network.cpp`, `tests/unit/test_default_node.cpp`, and `tests/unit/test_modal_model_default_network.cpp` cover the base network abstraction, the migrated node contract, the generic adapter bridge, shared-network activation, and invalid network references.
  - remaining phase-4 work: decide how aggressively to migrate legacy node/transition ownership into network specializations and how GUI/editor port synchronization should surface.

## 7. Completed technical baseline

Do not reopen without new evidence:

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

## 8. Activation and completion rules

A paused task becomes eligible only after the maintainer changes it to `ready` and confirms scope, validation and stop conditions.

A task moves to `done` only after required validation is green, evidence is reviewed, the PR is merged, source-branch deletion is confirmed, issue/status/backlog/changelog are updated and remaining boundaries are explicit.
