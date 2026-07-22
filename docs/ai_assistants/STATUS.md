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

Use this file for current branch state, validated baselines, open blockers, documentation-migration progress, and the next eligible work. Detailed run logs and commit-specific evidence belong under `history/evidence/`; executable and human-controlled work belong in the two canonical backlogs.

Older handoffs, matrices, inventories and plans are historical snapshots and do not override this file.

## 2. Repository state

- Repository: `rlcancian/Genesys-Simulator`.
- Active integration branch: `WorkInProgress`.
- Latest integrated documentation checkpoint: `958cdc6f63c02d004f1ffdf55e104b58a245bb88` — D0 governance layer through PR #512.
- Stable promotion target: `20262`, only near the end of the second semester of 2026.
- Release readiness: **not established**.
- Scheduled/autonomous tasks that could modify the repository: paused by maintainer instruction during this documentation migration.
- Active migration tracker: issue #511.

## 3. Technical baseline

Confirmed intended platform:

- Ubuntu 24.04;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6-only support direction;
- Google Test with the configured system/bundled fallback.

Recent executed GitHub runner baseline includes CMake 3.31.6, Ninja 1.13.2 and G++ 13.3.0. These exact versions describe recorded CI runs, not immutable minimum requirements.

## 4. Exact core test baseline

Latest exact Phase 0 inventory currently retained as the baseline:

- 1,721 tests registered;
- 1,717 tests executed and passed;
- 0 failed;
- 4 historical duplicate Search/Remove blocks disabled.

Equivalent active Search/Remove tests are mandatory, so those four disabled blocks are source-cleanup debt rather than current behavioral coverage gaps.

Validated core paths include:

- ordinary `tests-unit` configure/build/CTest;
- focused GUI GMDD diagnostics;
- kernel configure/build/direct runner/inventory/CTest;
- three smoke tests: simulator start, continuous system, LSODE;
- focused plugin-completion ASan/LeakSanitizer;
- AI plugin tests in ordinary and direct-runner graphs;
- legacy solver regression contract;
- Search/Remove runtime behavior;
- Queue, Station, Delay, and Resource first-use lifecycle/accounting;
- optimizer non-copy/non-move ownership contract.

No later production test-graph change has established a different exact inventory. Documentation and startup-workflow changes preserved ordinary CI but did not redefine the Phase 0 count.

## 5. Integrated bounded corrections

Completed work includes:

- CI branch/path trigger corrections;
- AI test aggregation;
- reusable Phase 0 validation;
- legacy Simpson quadrature stabilization and unsupported-derivative fail-fast behavior;
- active Search/Remove tests;
- Queue, Station, Delay and Resource lifecycle corrections;
- plugin-completion temporary-model/helper ownership correction with focused ASan/LSan;
- unsafe optimizer copy/move operations deleted;
- static plugin target/codemodel/link/symbol evidence;
- shell startup/argv validation;
- worker public-health validation;
- Data Analyser GUI startup validation;
- Optimizer GUI startup validation;
- AI Assistant GUI startup validation without credentials or provider calls.

## 6. Application validation matrix

| Application/path | Validated scope | Current status | Main remaining gap |
|---|---|---|---|
| Shell | preset, build, scripted commands, plugin count, clean exit | partially validated | model load/run and autoload deployment contract |
| Worker | preset, build, loopback health, exact JSON, bounded exit | partially validated | bind policy, auth, TLS, quotas, protected endpoints, isolation |
| Main GUI | focused GUI GMDD tests | partially validated | standalone startup and minimal model interaction |
| Data Analyser GUI | preset/build/Xvfb window/liveness/teardown | startup validated | import, analysis, fitting, charts, export, persistence, scientific correctness |
| Optimizer GUI | preset/build/Xvfb window/liveness/teardown | startup validated | real algorithms, evaluation, constraints, convergence, Level 3 workflow |
| AI Assistant GUI | no-credential preset/build/Xvfb window/teardown | startup validated | provider workflow, credentials, redaction and failure handling |
| HTTP Worker GUI | target/preset known | not independently validated | bounded startup and workflow contract |
| Do Experiments GUI | intentionally absent | not started | backend/workflow/product specification |
| Model-specific applications | historical bounded sweep | partially validated snapshot | revalidate failures, timeouts and known Gro compile breakages |

Startup evidence never implies complete functional or scientific maturity.

## 7. Plugin architecture status

Generated evidence confirms:

- `genesys_plugins_components` and `genesys_plugins_components_minimal` compile the same 84 component translation units;
- GLPK compile configuration differs between them when GLPK is present;
- the effective runtime graph primarily uses the target named `minimal`;
- one focused continuous-diffusion test reaches both archives;
- duplicate compilation/storage is real;
- active duplicate-symbol failure was not demonstrated;
- runtime feature selection remains ambiguous.

Implementation is blocked on human decision issue #492. Do not remove, rename, alias, repartition or dynamically migrate these targets before the decision is recorded.

## 8. Ownership and memory status

Confirmed only for exercised paths:

- temporary plugin-completion `Model` lifetime uses RAII;
- helper-owned Counter and StatisticsCollector responses are released;
- focused plugin-completion ASan/LeakSanitizer exits cleanly;
- `OptimizerDefaultImpl1` cannot be copied or moved while legacy owning pointers remain.

Not established:

- repository-wide leak freedom;
- complete thread safety;
- broad UBSan/Valgrind coverage;
- safe ownership for every legacy raw-pointer path.

## 9. Scientific and maturity boundary

- passing builds/tests do not establish numerical, statistical, biochemical or biological validity;
- authoritative reference packages remain pending for broad scientific validation;
- the optimizer backend remains a scaffold despite GUI startup validation;
- whole-cell and biochemical capabilities remain experimental/research-oriented;
- AI virtual-cell direction is strategic and neuro-symbolic-mechanistic, not implemented predictive functionality;
- supported functionality is intended to reach at least Level 3 before semester-stable inclusion.

## 10. Current human blockers

| Issue/backlog | Decision | Effect |
|---|---|---|
| #492 | canonical static component target architecture | blocks target correction |
| #496 | shell `autoloadplugins.txt` deployment/search/fallback contract | blocks autoload correction |
| #500 | worker bind-address default/configuration | blocks listener hardening |
| `HUM-SEC-002` | worker authentication architecture | blocks deployable controlled-intranet profile |
| `HUM-SCI-001` | numerical/statistical reference packages | blocks scientific validation claims |
| `HUM-OPT-001` | first optimizer algorithm and benchmark package | blocks real optimizer implementation |
| `HUM-VC-001` | first AI virtual-cell organism/use case/data package | blocks bounded scientific program |
| `HUM-REL-001` | final supported set and promotion gate | blocks eventual `20262` promotion only |

## 11. Documentation migration

### D0 — canonical governance layer

- Status: `done`.
- PR: #512.
- Merge: `958cdc6f63c02d004f1ffdf55e104b58a245bb88`.
- Final CI: ordinary tests and GUI GMDD diagnostics green.
- Source branch removed automatically after merge.

### D1 — normative source consolidation

- Status: `in review`.
- Branch: `WiP20260722/ai-docs-normative`.
- PR: #513.
- Scope: consolidate branch/promotion/documentation/security/maturity policy and retire four competing policy documents to migration notices.

### D2 — current-state consolidation

- Status: `in progress`.
- Branch: `WiP20260722/ai-docs-status`.
- Scope: make this file and the two backlogs the only operational sources; retire five competing plan/matrix/inventory/handoff documents.

### Remaining phases

- D3: relocate dated execution evidence and completed integration records;
- D4: consolidate topic guides into a small `reference/` set and remove temporary redirects;
- D5: add documentation-governance CI and root allowlist;
- D6: consolidate oldies governance and classify retained historical files without deleting them before the gate.

## 12. Current autonomous eligibility

During the migration freeze, only issue #511 documentation work may execute.

Do not start unrelated source, CMake, runtime, plugin, security, numerical, application, package or release work until migration completion is explicitly recorded here and the maintainer resumes those activities.

## 13. Next action

1. Finish and merge D1 after green CI.
2. Open D2 against the resulting `WorkInProgress` state.
3. Continue D3–D6 in bounded reviewable PRs.
4. Declare completion only after canonical root, references, history, archive tracker and documentation CI are all stabilized.
