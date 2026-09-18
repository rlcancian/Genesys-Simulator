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

## 2. Repository state

- Active integration branch: `WorkInProgress`.
- Current remote `WorkInProgress` HEAD confirmed on 2026-09-18: `4c745d7181c1584c7cf859a4db4ca93d46c95615` (`Enabling gui bilds in clion`, authored 2026-09-02).
- Latest integrated documentation checkpoint: `c023a2ef3722a2b8ea0e33db2b9fb0dd002f31a1` — completion record through PR #519.
- Final documentation-governance completion validation:
  - documentation-governance run `29939815697`: passed;
  - ordinary CI run `29939816032`: configure, build, CTest and GUI GMDD diagnostics passed.
- AI-assistant documentation migration D0–D6: complete according to its historical acceptance record. Issue #511 is closed; under the current status semantics, historical closure/completion wording is not automatically equivalent to the newer `done_confirmed` state unless explicitly revalidated under that criterion.
- Stable promotion target: `20262`, only near the end of the second semester of 2026.
- Release readiness: **not established**.
- GenESyS manual restructuring and governance follow-up: in progress on
  `docs/manual-genesys-restructure-20260722`; PDF regeneration completed
  2026-08-20 (Section 14).
- Per-user runtime Launcher/Dispatcher (PR #521, merge `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`)
  and its integration into the Debian package (PR #522, merge
  `d8fce9562617525657e8cbf9870b32323364773f`) are integrated. Section 14
  records the current package/lifecycle validation scope.
- Governance reconciliation after the Launcher/Debian merges (PR #523,
  merge `7855af78c06583eaf4732a62aaa7ea085160da7d`) is integrated.
- Arena ↔ GenESyS compatibility audit and Advanced Transfer/parser
  closeout (PR #527, merged 2026-08-30) is integrated: `Distance`,
  `Segment`, `Conveyor`, `Transporter` data definitions; completed
  `Storage`/`Store`/`Unstore`/`DropOff` and `Access`/`Exit`/`Start`/`Stop`/`Move`
  components; the compatibility matrix closed through Advanced Transfer,
  with Flow Process classified and the parser/Arena Variables Guide overlap
  documented. `Network`, `NetworkLink`, `ActivityArea` and CTMC-class
  features remain intentionally out of this scope — see
  [`reference/ARENA_GENESYS_COMPATIBILITY.md`](reference/ARENA_GENESYS_COMPATIBILITY.md).
  A dedicated maintainer-decision note for `Process::AllocationType` versus
  the internal `Delay` allocation category was also added:
  [`reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md`](reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md).
  Validation snapshot at merge time: focused Arena/MaterialHandling tests
  green (28/28), `tests-smoke` green (3/3), `tests-kernel-unit` green except
  the preexisting `genesys_test_optimizer_ownership_contract_NOT_BUILT`
  failure (Section 4); `tests-unit` still shows 13 unrelated WholeCell/Bio
  failures tied to a plugin-loading path involving `attribute.so`, outside
  this scope and not yet triaged into a backlog entry.
- The ModalModel/network-of-computation implementation cycle represented by
  PR #528 (merge `e0185163`) and PR #529 (merge `fdae135b`), both 2026-08-31,
  is integrated. Historical evidence from that cycle records
  `DefaultNetwork`/`DefaultNode`, the `ModalModelDefault` bridge,
  `EFSMNetwork`, the `GraphNetwork` family, `MarkovChainNetwork` and
  `ColoredPetriNetNetwork`, with a kernel-unit snapshot of 1810/1810 executed
  tests passed and 4 preexisting disabled tests. This is historical evidence,
  not current-HEAD execution evidence.
- `AUTO-MODAL-001` must now be interpreted as **closed**: its development
  cycle is inactive, but this does **not** prove that the full approved
  ModalModel/DefaultNetwork architecture is complete. The current Modal/Network
  development is **not `done_confirmed`**.
- Current-HEAD build/test/runtime verification of Modal/Network is pending in
  a local environment with Git, CMake, Ninja, CTest and sanitizer access.
  The continuation and verification guide is
  [`reference/MODAL_NETWORK_COMPLETION_PLAN.md`](reference/MODAL_NETWORK_COMPLETION_PLAN.md).
- Historical `.gen` compatibility is explicitly **not a requirement** for the
  Modal/Network continuation. Current supported persistence/round-trip remains
  a requirement.
- Maintainer decisions recorded 2026-09-18 for the continuation include:
  a pragmatic GenESyS CPN subset rather than a full academic CPN target;
  eventual Cellular Automata migration to `DefaultNetwork`, deferred until the
  current architecture is complete and validated; and configurable EFSM
  multiple-enabled-transition semantics with three modes: model error,
  nondeterministic random choice through the GenESyS RNG/sampler, and
  deterministic priority.
- Legacy modal classes may be moved to an excluded `deprecated/` subdirectory
  or physically removed after current dependency analysis proves the safe path;
  they are not to be retained solely for historical `.gen` compatibility.
- A Phase 9 GUI/editor architecture remains proposed rather than established as
  implemented; backend contracts must be stabilized and revalidated first:
  [`reference/MODAL_NETWORK_GUI_ARCHITECTURE.md`](reference/MODAL_NETWORK_GUI_ARCHITECTURE.md).

## 3. Technical baseline

Intended platform:

- Ubuntu 24.04;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6-only support direction;
- Google Test through the configured system/bundled fallback.

Recent CI evidence used CMake 3.31.6, Ninja 1.13.2 and G++ 13.3.0. Those exact versions describe recorded runs, not immutable minimum requirements.

## 4. Exact core test baseline

Latest retained exact Phase 0 inventory:

- registered: 1,721;
- executed/passed: 1,717;
- failed: 0;
- disabled: 4 historical duplicate Search/Remove blocks.

Equivalent active Search/Remove tests are mandatory, so the four disabled blocks are source-cleanup debt rather than current behavioral coverage gaps.

Validated core paths include ordinary unit CI, GUI GMDD diagnostics, kernel/direct runner/CTest inventory, three smoke tests, focused plugin-completion ASan/LSan, AI plugin tests, legacy solver regression, Search/Remove runtime, Queue/Station/Delay/Resource lifecycle and the optimizer non-copy/non-move contract.

No later production test-graph change has established a different exact inventory in the canonical baseline. Modal/Network historical evidence records later focused/regression counts for the 2026-08-31 implementation checkpoint, but those counts must be re-established on the current `WorkInProgress` HEAD before being promoted to current execution evidence.

## 5. Integrated bounded work

Completed historical work includes:

- CI trigger corrections and AI test aggregation;
- reusable Phase 0 validation;
- legacy Simpson/unsupported-derivative stabilization;
- Search/Remove and runtime statistics/accounting lifecycle corrections;
- plugin-completion ownership correction and focused sanitizer;
- optimizer copy/move barrier;
- static plugin target/codemodel/link/symbol evidence;
- standalone shell and worker health validation;
- Data Analyser, Optimizer and AI Assistant GUI startup validation.

Executed details are indexed in [`history/evidence/2026/07/VALIDATION_LEDGER.md`](history/evidence/2026/07/VALIDATION_LEDGER.md).

Historical completion wording in this section records prior task acceptance under the rules then in force; it is not an automatic `done_confirmed` classification under the status semantics adopted on 2026-09-18.

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

- issue #492: canonical static component-target architecture;
- issue #496: shell `autoloadplugins.txt` deployment/search/fallback contract;
- issue #500: worker bind-address contract;
- `HUM-SEC-002`: worker authentication architecture;
- `HUM-SEC-004`: runtime signing public key provisioning for per-user Launcher updates;
- `HUM-SCI-001`: authoritative numerical/statistical reference packages;
- `HUM-OPT-001`: initial optimizer algorithm/benchmark package;
- `HUM-VC-001`: initial AI virtual-cell organism/use case/data package;
- `HUM-REL-001`: final supported set and promotion gate.

For Modal/Network, the decisions recorded on 2026-09-18 remove the previously assumed need for a human decision about historical `.gen` compatibility, CPN target depth, Cellular Automata direction, and EFSM multiple-enabled-transition policy. Any remaining implementation uncertainty in those areas is a current-code/current-test verification question, not permission to invent a new policy.

These other listed boundaries must not be guessed by autonomous agents.

## 8. Ownership, scientific and maturity boundaries

Confirmed only for exercised ownership paths: temporary plugin-completion Model uses RAII, helper responses are released, focused ASan/LSan is clean, and `OptimizerDefaultImpl1` cannot be copied/moved.

Not established: repository-wide leak freedom, thread safety, broad UBSan/Valgrind, complete optimizer behavior, broad numerical/statistical validation, biological predictive validity or release readiness.

For Modal/Network specifically, ownership/lifetime conclusions on the current HEAD remain verification-pending until the local continuation maps current ownership and executes the required tests/diagnostics.

Whole-cell/biochemical/AI virtual-cell work remains experimental/research-oriented. Software maturity and scientific claim level remain independent.

## 9. Documentation migration result

| Phase | Status | PR / merge | Result |
|---|---|---|---|
| D0 | done (historical) | #512 / `958cdc6f63c02d004f1ffdf55e104b58a245bb88` | canonical layer and runbooks |
| D1 | done (historical) | #513 / `b48697e77d39b25cafc19271ce574bdead60f94d` | normative governance consolidated |
| D2 | done (historical) | #514 / `53b49f7518509823fe2265a3f017b5aa76f09d2f` | sole current state and backlogs |
| D3 | done (historical) | #515 / `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca` | date-first evidence ledger |
| D4 | done (historical) | #516 / `d375d9e68e5c1dc84e214a772fb15cb05944f0d8` | six technical references and active-root cleanup |
| D6 | done (historical) | #517 / `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5` | single oldies tracker; 25 retained files protected |
| D5 | done (historical) | #518 / `610d8ab21c87cfd11663af78370b39262cf4da81` | local and GitHub Actions governance enforcement |
| Completion | done (historical) | #519 / `c023a2ef3722a2b8ea0e33db2b9fb0dd002f31a1` | final canonical record and issue closure basis |

All migration source branches through PR #519 were removed automatically after merge.

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

Previously paused technical tasks remain `paused`; they do not resume automatically. A maintainer must explicitly activate the selected next task in `BACKLOG_AUTONOMOUS.md`.

For Modal/Network, the maintainer has approved the continuation direction recorded in `reference/MODAL_NETWORK_COMPLETION_PLAN.md`. Because the canonical autonomous backlog is a large file whose connector response is truncated, this GitHub-only documentation pass must not reconstruct it unsafely. The local continuation must reconcile `AUTO-MODAL-001` to `closed` and add/activate the bounded continuation task (recommended ID `AUTO-MODAL-002`) before autonomous implementation begins.

## 13. Ongoing governance

- Future AI-assistant documentation changes must pass the focused governance workflow.
- Canonical facts, tasks, decisions, evidence and historical material must remain in their designated locations.
- The completed migration record is `history/migrations/ai_docs_governance_completion_20260722.md`.
- `closed` must not be used as evidence of implementation completeness.
- `done_confirmed` is reserved for a task/scope that is fully developed according to its approved plan, built, tested, functioning, verified, documented and accepted with no known unmet acceptance criterion in that scope.
- Existing historical `done` labels remain legacy records until individually reconciled; they are not automatically upgraded to `done_confirmed`.

## 14. Launcher and Debian packaging

- Per-user runtime Launcher/Dispatcher (`AUTO-APP-003`): PR #521, merge
  `d88b4b20b5163891e47c9e63bb04eca68a9e40d0`. `genesys-launcher` and
  `genesys-dispatch` select between a validated per-user runtime
  (`~/.local/share/genesys/current`) and a system fallback, with fail-closed
  signature verification and no network activity outside the launcher.
- Launcher integrated into the Debian package (`AUTO-PKG-001`): PR #522,
  merge `d8fce9562617525657e8cbf9870b32323364773f`.
- Package split: `genesys-common` (launcher, dispatcher,
  `/etc/genesys/update.conf`, public `genesys-mcp`), `genesys-shell`,
  `genesys-worker` (also owns the legacy `genesys-web` command),
  `genesys-gui` (depends on the three above), and `genesys-web` (payload-free
  transitional package depending on `genesys-worker`).
- `.github/workflows/genesys-debian-package.yml` build job (`dpkg-buildpackage`,
  package/AppStream/Lintian verification) and lifecycle-validation job
  (`packaging/linux/validate-debian-lifecycle.sh`) are both green on the
  PR-head run `32421072334` (head `368091693c613d5d090cc564f028c957a38c4668`)
  and on the post-merge `WorkInProgress` push run `32422514020` (merge
  `d8fce9562617525657e8cbf9870b32323364773f`): `lintian` and
  `lintian --fail-on error` report zero findings across the 5 produced
  `.deb` files, `appstreamcli validate --no-net` passes, and the full
  install/ownership/non-root-user/system-fallback/per-user-runtime/
  admin-policy/six-invalid-runtime-case/integrity/reinstall/remove/purge
  lifecycle completes successfully.
- `genesys-mcp` has a public dispatcher entry point but intentionally no
  Debian system fallback (`source/applications/mcp/` is Python, out of the
  Debian build contract); the dispatcher returns a controlled diagnostic
  (exit 127) instead.
- Remaining boundary: `HUM-SEC-004` (runtime signing public key
  provisioning) is required before per-user runtime updates can be enabled;
  as shipped, `/etc/genesys/update.conf` keeps remote updates disabled and
  `require_signature=true`. No GitHub Release, PPA/APT repository, or
  package signing was produced by this work; `genesys-debian-packages` is a
  workflow artifact, not a distribution channel.

## 15. ModalModel / DefaultNetwork continuation checkpoint

Current status: **verification pending; not `done_confirmed`**.

The next local continuation should first prove the current state before editing code. At minimum it should:

1. establish the current build and Modal/Network focused-test baseline;
2. verify `DefaultNetwork` ownership/lifecycle/contracts;
3. verify the `ModalModelDefault` adapter end-to-end, including zero/one/multiple outputs, presence, bindings, entity consumption/cloning, `_check()`, reset and current persistence;
4. verify EFSM and implement only any confirmed gap for the maintainer-approved configurable multiple-enabled-transition policy;
5. verify Graph/DAG advertised invariants and algorithms without adding process-flow semantics;
6. verify the finite time-homogeneous DTMC contract, probability validation and reproducible sampling;
7. define and verify the pragmatic CPN subset before adding any advanced CPN semantics;
8. map current dependencies on legacy modal classes, then move unused retained artifacts to a non-built `deprecated/` directory or remove them if current dependency evidence supports deletion;
9. keep Cellular Automata migration deferred until the current Modal/Network architecture is complete, functioning, tested and validated;
10. address GUI/editor implementation only after backend contracts are stable;
11. reconcile the canonical backlogs and evidence records after each material result.

The authoritative detailed continuation guide is `reference/MODAL_NETWORK_COMPLETION_PLAN.md`.