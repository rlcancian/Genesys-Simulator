---
document_type: status
authority: current-state
owner: project-maintainer
last_updated: 2026-09-18
update_on: merged-change-or-material-status-change
status: active
tracks: 511
---

# GenESyS Current Status

## 1. Purpose

This is the single current operational state for AI-assisted work in `rlcancian/Genesys-Simulator`.

Use it for current branch/checkpoint state, validated baselines, blockers and next eligible work. Detailed executed results belong under `history/evidence/`; tasks and decisions belong in the two canonical backlogs.

A historical task/PR being merged or `closed` is not proof that the corresponding broader subsystem is complete. For autonomous-task semantics, including the stronger `done_confirmed` state, use `BACKLOG_AUTONOMOUS.md`.

## 2. Repository state

- Active integration branch: `WorkInProgress`.
- `WorkInProgress` checkpoint directly inspected for the 2026-09-18 ModalModel/Network reconciliation: `4c745d7181c1584c7cf859a4db4ca93d46c95615` (`Enabling gui bilds in clion`, 2026-09-02).
- AI-assistant documentation migration D0–D6: **complete** as a bounded documentation-governance task; this does not imply product/subsystem completion.
- Issue #511: closed as completed for that documentation-governance scope.
- Stable promotion target: `20262`, only near the end of the second semester of 2026.
- Release readiness: **not established**.
- GenESyS manual restructuring and governance follow-up: in progress on `docs/manual-genesys-restructure-20260722`; PDF regeneration completed 2026-08-20 (Section 14).
- Per-user runtime Launcher/Dispatcher (PR #521, merge `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`) and its integration into the Debian package (PR #522, merge `d8fce9562617525657e8cbf9870b32323364773f`) are integrated. Section 14 records the current package/lifecycle validation scope.
- Governance reconciliation after the Launcher/Debian merges (PR #523, merge `7855af78c06583eaf4732a62aaa7ea085160da7d`) is integrated.
- Arena ↔ GenESyS compatibility audit and Advanced Transfer/parser closeout (PR #527, merged 2026-08-30) is integrated: `Distance`, `Segment`, `Conveyor`, `Transporter` data definitions; completed `Storage`/`Store`/`Unstore`/`DropOff` and `Access`/`Exit`/`Start`/`Stop`/`Move` components; the compatibility matrix closed through Advanced Transfer, with Flow Process classified and the parser/Arena Variables Guide overlap documented. `Network`, `NetworkLink`, `ActivityArea` and CTMC-class features remain intentionally out of that scope — see [`reference/ARENA_GENESYS_COMPATIBILITY.md`](reference/ARENA_GENESYS_COMPATIBILITY.md). A dedicated maintainer-decision note for `Process::AllocationType` versus the internal `Delay` allocation category is [`reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md`](reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md).
- Arena validation snapshot at merge time: focused Arena/MaterialHandling tests green (28/28), `tests-smoke` green (3/3), `tests-kernel-unit` green except the preexisting `genesys_test_optimizer_ownership_contract_NOT_BUILT` failure; `tests-unit` showed 13 unrelated WholeCell/Bio failures tied to a plugin-loading path involving `attribute.so`, outside that scope and not yet triaged into a dedicated backlog entry.

### ModalModel / DefaultNetwork current state

- The network-centered ModalModel implementation from PR #528 (merge `e0185163`) and PR #529 (merge `fdae135b`), both 2026-08-31, is present in `WorkInProgress`.
- Confirmed in the current remote source tree/documentation:
  - `DefaultNetwork` / `DefaultNode` network core exists;
  - `ModalModelDefault` contains the network bridge path;
  - `EFSMNetwork` exists;
  - `GraphNetwork`, `DirectedGraphNetwork` and `DirectedAcyclicGraphNetwork` exist;
  - `MarkovChainNetwork` implements the initial finite time-homogeneous DTMC direction;
  - `ColoredPetriNetNetwork` implements the initial pragmatic/fixed-inscription CPN direction;
  - legacy ModalModel classes/scaffolding remain present beside the network-centered implementation;
  - Cellular Automata code remains present outside the completed network migration;
  - `source/plugins/components/CMakeLists.txt` currently collects component `.cpp` files recursively, so a future `ModalModel/deprecated/` quarantine requires an explicit build exclusion before moving source files there.
- Historical executed evidence from 2026-08-31 reports the then-final implementation checkpoint building successfully and `tests-kernel-unit` passing 1810/1810 executed tests, with four preexisting disabled tests. These results are **historical evidence for those recorded commits**, not current-HEAD verification.
- Current `WorkInProgress` build/test/runtime verification for the complete ModalModel/DefaultNetwork plan: **verification pending locally**. The 2026-09-18 reconciliation is GitHub-only and did not execute CMake, Ninja, CTest, GUI interaction, model runs or sanitizers.
- `AUTO-MODAL-001` is now `closed`: this means its historical development session is inactive and **does not mean complete**.
- `AUTO-MODAL-002` is the continuation task and is currently `paused` pending explicit maintainer activation for a local agent.
- The current verification/completion guide is [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md).
- Maintainer decisions recorded on 2026-09-18:
  - historical `.gen` compatibility is not a requirement for this migration/completion effort;
  - legacy ModalModel implementation may be removed from the active build and temporarily quarantined under `source/plugins/components/ModalModel/deprecated/` after current dependency/build analysis;
  - the initial CPN objective is a pragmatic subset sufficient for GenESyS, not automatic completion of every advanced CPN feature;
  - EFSM multiple-enabled-transition semantics shall be configurable among model error, nondeterministic selection with reproducible GenESyS RNG, and deterministic selection by priority;
  - Cellular Automata is intended to migrate to `DefaultNetwork`, but only after the current ModalModel/DefaultNetwork architecture is fully functional, tested, validated and eligible for `done_confirmed`;
  - broad network GUI/editor work follows backend stabilization and local revalidation of the proposed GUI architecture.
- The proposed GUI/editor architecture remains [`reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`](reference/MODAL_NETWORK_GUI_ARCHITECTURE.md); it is planning, not evidence of implementation.
- CTMC, MDP/controlled Markov models, dynamic graph traversal/routing/movement, advanced/full CPN typing/binding/concurrent firing and the later Cellular Automata migration are not current completion requirements unless separately activated.

## 3. Technical baseline

Intended platform:

- Ubuntu 24.04;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6-only support direction;
- Google Test through the configured system/bundled fallback.

Recent retained CI evidence used CMake 3.31.6, Ninja 1.13.2 and G++ 13.3.0. Those exact versions describe recorded runs, not immutable minimum requirements.

## 4. Exact core test baseline

Latest retained exact Phase 0 inventory:

- registered: 1,721;
- executed/passed: 1,717;
- failed: 0;
- disabled: 4 historical duplicate Search/Remove blocks.

Equivalent active Search/Remove tests are mandatory, so the four disabled blocks are source-cleanup debt rather than current behavioral coverage gaps.

A later historical ModalModel implementation checkpoint reported 1810/1810 executed kernel-unit tests passing on 2026-08-31. Because that evidence was for a later but specific implementation checkpoint and the current 2026-09-18 HEAD has not been locally re-run in this reconciliation, **the exact current test inventory/status is not confirmed**. `AUTO-MODAL-002` must establish the current baseline before modifying behavior.

Validated historical core paths include ordinary unit CI, GUI GMDD diagnostics, kernel/direct runner/CTest inventory, three smoke tests, focused plugin-completion ASan/LSan, AI plugin tests, legacy solver regression, Search/Remove runtime, Queue/Station/Delay/Resource lifecycle and the optimizer non-copy/non-move contract.

## 5. Integrated bounded work

Integrated bounded work includes:

- CI trigger corrections and AI test aggregation;
- reusable Phase 0 validation;
- legacy Simpson/unsupported-derivative stabilization;
- Search/Remove and runtime statistics/accounting lifecycle corrections;
- plugin-completion ownership correction and focused sanitizer;
- optimizer copy/move barrier;
- static plugin target/codemodel/link/symbol evidence;
- standalone shell and worker health validation;
- Data Analyser, Optimizer and AI Assistant GUI startup validation;
- the historical 2026-08-31 ModalModel/Network implementation slices recorded under `AUTO-MODAL-001`.

Executed details are indexed in [`history/evidence/2026/07/VALIDATION_LEDGER.md`](history/evidence/2026/07/VALIDATION_LEDGER.md), Git/PR history and the applicable task/reference records. Integration of a bounded slice must not be interpreted as `done_confirmed` for a broader subsystem.

## 6. Application status

| Application/path | Validated scope | Status | Main remaining gap |
|---|---|---|---|
| Shell | preset/build/scripted commands/plugin count/exit | partially validated | model load/run and autoload deployment |
| Worker | preset/build/loopback health/exact JSON/bounded exit | partially validated | bind, auth, TLS, quotas, protected endpoints, isolation |
| Main GUI | focused GMDD tests | partially validated | standalone startup and minimal interaction |
| Data Analyser GUI | Xvfb window/liveness/teardown | startup validated | analysis workflows and scientific correctness |
| Optimizer GUI | Xvfb window/liveness/teardown | startup validated | real algorithms and Level 3 workflow |
| AI Assistant GUI | no-credential Xvfb startup/teardown | startup validated | provider/credentials/redaction/failure workflow |
| HTTP Worker GUI | target/preset known | not independently validated | bounded startup/workflow |
| Do Experiments GUI | intentionally absent | not started | backend/product/workflow definition |
| Model-specific apps | historical bounded sweep | snapshot only | current revalidation and known failures |

Startup does not imply functional or scientific maturity.

## 7. Open architecture, security and science boundaries

- `AUTO-MODAL-002`: local verification/completion of current ModalModel/DefaultNetwork architecture, currently paused;
- issue #492: canonical static component-target architecture;
- issue #496: shell `autoloadplugins.txt` deployment/search/fallback contract;
- issue #500: worker bind-address contract;
- `HUM-SEC-002`: worker authentication architecture;
- `HUM-SEC-004`: runtime signing public key provisioning for per-user Launcher updates;
- `HUM-SCI-001`: authoritative numerical/statistical reference packages;
- `HUM-OPT-001`: initial optimizer algorithm/benchmark package;
- `HUM-VC-001`: initial AI virtual-cell organism/use case/data package;
- `HUM-REL-001`: final supported set and promotion gate.

The Modal-specific decisions formerly open under `HUM-MODAL-*` were reconciled on 2026-09-18; implementation/verification work remains governed by `AUTO-MODAL-002` and the completion plan.

## 8. Ownership, scientific and maturity boundaries

Confirmed only for exercised ownership paths: temporary plugin-completion Model uses RAII, helper responses are released, focused ASan/LSan is clean, and `OptimizerDefaultImpl1` cannot be copied/moved.

Not established: repository-wide leak freedom, thread safety, broad UBSan/Valgrind, complete optimizer behavior, broad numerical/statistical validation, biological predictive validity or release readiness.

For ModalModel/Network specifically, ownership/lifetime correctness at the current HEAD remains **verification pending** until local focused inspection/tests/sanitizers are executed as appropriate.

Whole-cell/biochemical/AI virtual-cell work remains experimental/research-oriented. Software maturity and scientific claim level remain independent.

## 9. Documentation migration result

| Phase | Status | PR / merge | Result |
|---|---|---|---|
| D0 | done | #512 / `958cdc6f63c02d004f1ffdf55e104b58a245bb88` | canonical layer and runbooks |
| D1 | done | #513 / `b48697e77d39b25cafc19271ce574bdead60f94d` | normative governance consolidated |
| D2 | done | #514 / `53b49f7518509823fe2265a3f017b5aa76f09d2f` | sole current state and backlogs |
| D3 | done | #515 / `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca` | date-first evidence ledger |
| D4 | done | #516 / `d375d9e68e5c1dc84e214a772fb15cb05944f0d8` | technical references and active-root cleanup |
| D6 | done | #517 / `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5` | single oldies tracker; 25 retained files protected |
| D5 | done | #518 / `610d8ab21c87cfd11663af78370b39262cf4da81` | local and GitHub Actions governance enforcement |
| Completion | done | #519 / `c023a2ef3722a2b8ea0e33db2b9fb0dd002f31a1` | final canonical record and issue closure basis |

These `done` values describe the bounded documentation-migration tasks under their historical acceptance criteria. They are not a general product-level `done_confirmed` claim.

## 10. Final governed structure

The top level of `docs/ai_assistants/` contains exactly:

- `README.md`;
- `GOVERNANCE.md`;
- `ARCHITECTURE.md`;
- `STATUS.md`;
- `BACKLOG_AUTONOMOUS.md`;
- `BACKLOG_HUMAN.md`.

Supporting material is routed through:

- `runbooks/`;
- `reference/`;
- `history/`;
- `archive/`;
- retained non-authoritative `oldies/`.

The structure is enforced by `.github/workflows/genesys-docs-governance.yml` and `scripts/validate-ai-docs.py`.

## 11. Historical retention state

- `archive/OLDIES_REVIEW.md` is the only active tracker for the 25 retained historical files.
- No content file under `oldies/` was deleted or modified by D6.
- Every retained file remains `retained-review-pending` and not deletion-ready.
- Deletion remains prohibited before 2026-11-01 and additionally requires individual review, explicit maintainer approval and a dedicated deletion PR.

## 12. Autonomous eligibility after migration

The documentation-migration-specific freeze has ended.

Tasks marked `paused` remain inactive until explicit maintainer activation. A `closed` task is also inactive but, unlike `done`/`done_confirmed`, carries no completion claim. See `BACKLOG_AUTONOMOUS.md` for the normative task-status meanings.

`AUTO-MODAL-002` is currently paused and is the intended next local continuation for the ModalModel/DefaultNetwork effort.

## 13. Ongoing governance

- Future AI-assistant documentation changes must pass the focused governance workflow.
- Canonical facts, tasks, decisions, evidence and historical material must remain in their designated locations.
- The completed documentation migration record is `history/migrations/ai_docs_governance_completion_20260722.md`.
- Unknown or unexecuted ModalModel/Network facts must remain explicitly marked as verification pending until local evidence is recorded.

## 14. Launcher and Debian packaging

- Per-user runtime Launcher/Dispatcher (`AUTO-APP-003`): PR #521, merge `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`. `genesys-launcher` and `genesys-dispatch` select between a validated per-user runtime (`~/.local/share/genesys/current`) and a system fallback, with fail-closed signature verification and no network activity outside the launcher.
- Launcher integrated into the Debian package (`AUTO-PKG-001`): PR #522, merge `d8fce9562617525657e8cbf9870b32323364773f`.
- Package split: `genesys-common` (launcher, dispatcher, `/etc/genesys/update.conf`, public `genesys-mcp`), `genesys-shell`, `genesys-worker` (also owns the legacy `genesys-web` command), `genesys-gui` (depends on the three above), and `genesys-web` (payload-free transitional package depending on `genesys-worker`).
- `.github/workflows/genesys-debian-package.yml` build job (`dpkg-buildpackage`, package/AppStream/Lintian verification) and lifecycle-validation job (`packaging/linux/validate-debian-lifecycle.sh`) are both green on the PR-head run `32421072334` (head `368091693c613d5d090cc564f028c957a38c4668`) and on the post-merge `WorkInProgress` push run `32422514020` (merge `d8fce9562617525657e8cbf9870b32323364773f`): `lintian` and `lintian --fail-on error` report zero findings across the five produced `.deb` files, `appstreamcli validate --no-net` passes, and the full install/ownership/non-root-user/system-fallback/per-user-runtime/admin-policy/six-invalid-runtime-case/integrity/reinstall/remove/purge lifecycle completes successfully.
- `genesys-mcp` has a public dispatcher entry point but intentionally no Debian system fallback (`source/applications/mcp/` is Python, out of the Debian build contract); the dispatcher returns a controlled diagnostic (exit 127) instead.
- Remaining boundary: `HUM-SEC-004` (runtime signing public key provisioning) is required before per-user runtime updates can be enabled; as shipped, `/etc/genesys/update.conf` keeps remote updates disabled and `require_signature=true`. No GitHub Release, PPA/APT repository, or package signing was produced by this work; `genesys-debian-packages` is a workflow artifact, not a distribution channel.