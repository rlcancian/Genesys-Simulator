---
document_type: status
authority: current-state
owner: project-maintainer
last_updated: 2026-07-22
update_on: merged-change-or-material-status-change
status: active
tracks: 511
---

# GenESyS Current Status

## 1. Purpose

This is the single current operational state for AI-assisted work in `rlcancian/Genesys-Simulator`.

Use it for current branch/checkpoint state, validated baselines, blockers, migration progress and next eligible work. Detailed executed results belong under `history/evidence/`; tasks and decisions belong in the two canonical backlogs.

## 2. Repository state

- Active integration branch: `WorkInProgress`.
- Latest integrated documentation checkpoint: `d375d9e68e5c1dc84e214a772fb15cb05944f0d8` — D4 technical-reference consolidation through PR #516.
- D4 validation: run `29936506990`, ordinary configure/build/CTest and GUI GMDD diagnostics passed.
- Stable promotion target: `20262`, only near the end of the second semester of 2026.
- Release readiness: **not established**.
- Scheduled/autonomous tasks that could modify the repository: paused during this documentation migration.
- Active migration tracker: issue #511.

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

No later production test-graph change has established a different exact inventory.

## 5. Integrated bounded work

Completed work includes:

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
- `HUM-SCI-001`: authoritative numerical/statistical reference packages;
- `HUM-OPT-001`: initial optimizer algorithm/benchmark package;
- `HUM-VC-001`: initial AI virtual-cell organism/use case/data package;
- `HUM-REL-001`: final supported set and promotion gate.

These must not be guessed by autonomous agents.

## 8. Ownership, scientific and maturity boundaries

Confirmed only for exercised ownership paths: temporary plugin-completion Model uses RAII, helper responses are released, focused ASan/LSan is clean, and `OptimizerDefaultImpl1` cannot be copied/moved.

Not established: repository-wide leak freedom, thread safety, broad UBSan/Valgrind, complete optimizer behavior, broad numerical/statistical validation, biological predictive validity or release readiness.

Whole-cell/biochemical/AI virtual-cell work remains experimental/research-oriented. Software maturity and scientific claim level remain independent.

## 9. Documentation migration

| Phase | Status | PR / merge | Result |
|---|---|---|---|
| D0 | done | #512 / `958cdc6f63c02d004f1ffdf55e104b58a245bb88` | canonical layer and runbooks |
| D1 | done | #513 / `b48697e77d39b25cafc19271ce574bdead60f94d` | normative governance consolidated |
| D2 | done | #514 / `53b49f7518509823fe2265a3f017b5aa76f09d2f` | sole current state and backlogs |
| D3 | done | #515 / `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca` | date-first evidence ledger |
| D4 | done | #516 / `d375d9e68e5c1dc84e214a772fb15cb05944f0d8` | six technical references and active-root cleanup |
| D6 | running | branch `WiP20260722/ai-docs-oldies` | one oldies tracker; all 25 retained files protected |
| D5 | blocked on D6 | pending | structural/link/front-matter CI enforcement |

All merged D0–D4 source branches were automatically removed. The D6 branch remains active and must not be deleted before its PR is merged.

## 10. Historical retention state

- `archive/OLDIES_REVIEW.md` is the only active tracker for the 25 retained historical files.
- The former `oldies_inventory.md`, `oldies_review_status.md` and `consolidation_map.md` are removed in D6.
- No content file under `oldies/` is deleted or modified by D6.
- Deletion remains prohibited before 2026-11-01 and additionally requires individual review, explicit maintainer approval and a dedicated deletion PR.

## 11. Current autonomous eligibility

Only issue #511 documentation migration work may execute during the freeze. Unrelated source, CMake, runtime, plugin, security, numerical, application, package and release work remains paused.

## 12. Next action

1. Validate and merge D6.
2. Execute D5 against the final documentation structure.
3. Verify the exact six-file top-level allowlist and all active links/front matter.
4. Close issue #511 and record explicit migration completion only when D5 is green and source-branch cleanup is confirmed.
