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

## 1. Scope of this document

This is the single current-state summary for AI-assisted GenESyS work.

It records the latest integrated checkpoint, validated baselines, active blockers, open workstreams, and next eligible work. It does not replace immutable workflow artifacts, pull-request discussions, detailed evidence reports, or source-code inspection.

When this file conflicts with an older handoff, plan, test count, or dated evidence document, this file controls current operational routing. The older document remains evidence for its recorded commit and scope.

## 2. Repository state

- Repository: `rlcancian/Genesys-Simulator`
- Active development branch: `WorkInProgress`
- Latest integrated checkpoint before the documentation-governance branch: `ca47f7f05b0414190fedf73da947dd3d5c5e2456`
- Current documentation-governance branch: `WiP20260722/ai-docs-governance`
- Tracking issue: #511
- Pull request: #512
- Semester-stable target: `20262`, reserved for end-of-second-semester promotion
- Release readiness: **not established**
- Tasks/agents that could alter the repository: paused by the maintainer during this documentation migration

## 3. Technical baseline

Confirmed project baseline:

- Ubuntu 24.04 primary CI/development platform;
- CMake 3.24 or newer;
- Ninja;
- C++23 with compiler extensions disabled;
- Qt6-only intended support policy;
- Google Test system package with configured bundled fallback;
- root `CMakeLists.txt` and `CMakePresets.json` are canonical build entry points.

Recent executed GitHub runner toolchain:

- CMake 3.31.6;
- Ninja 1.13.2;
- G++ 13.3.0;
- Qt 6.4.2 in the latest independent GUI startup workflow.

## 4. Core test baseline

The latest exact kernel inventory remains the validated checkpoint recorded before PR #485 merged:

- registered: 1,721;
- executed/passed: 1,717;
- failed: 0;
- disabled: 4 historical duplicate Search/Remove blocks.

Those four disabled blocks are not current behavioral coverage gaps. Equivalent Search/Remove scenarios are active and mandatory in a focused executable. Removing the duplicate source blocks remains local source-cleanup work.

Validated core paths:

- ordinary `tests-unit` configure/build/CTest;
- focused GUI GMDD tests;
- `tests-kernel-unit` configure/build/direct runner/inventory/CTest;
- `tests-smoke`: simulator start, continuous system, and LSODE;
- focused plugin-completion ASan/LeakSanitizer path;
- AI plugin tests in ordinary and direct-runner graphs;
- legacy solver regression contract;
- Search/Remove runtime behavior;
- Queue, Station, Delay, and Resource first-use lifecycle/accounting;
- optimizer non-copy/non-move ownership contract.

No newer production test-graph change has invalidated the exact 1,721/1,717/4 inventory. Documentation and application-startup workflow changes after that checkpoint preserved ordinary CI, but did not rerun a new exact Phase 0 inventory for this status update.

## 5. Integrated bounded corrections

Completed and integrated work includes:

- CI branch/path trigger corrections;
- AI test aggregation;
- reusable Phase 0 validation;
- legacy Simpson quadrature stabilization and fail-fast unsupported derivative boundary;
- active Search/Remove tests;
- Queue, Station, Delay, and Resource lifecycle corrections;
- temporary plugin-completion Model/helper ownership correction with focused ASan/LSan guard;
- deleted unsafe optimizer copy/move operations;
- generated static plugin target/source/codemodel/link/symbol evidence;
- standalone shell startup/argv validation;
- standalone worker public-health validation;
- standalone Data Analyser GUI startup validation;
- standalone Optimizer GUI startup validation;
- standalone AI Assistant GUI startup validation.

## 6. Application validation matrix

| Application/path | Validated scope | Current status | Main remaining gap |
|---|---|---|---|
| Shell | preset, build, scripted argv commands, plugin count, clean exit | partially validated | model load/run and `autoloadplugins.txt` deployment contract |
| Worker | preset, build, loopback health request, exact JSON, bounded exit | partially validated | bind policy, auth, TLS, quotas, protected endpoints, isolation |
| Main GUI | focused GUI GMDD tests | partially validated | standalone startup and minimal model interaction |
| Data Analyser GUI | preset, build, Qt6/XCB/Xvfb window, bounded liveness/teardown | startup validated | import, analysis, fitting, charts, export, persistence, scientific correctness |
| Optimizer GUI | preset, build, Qt6/XCB/Xvfb window, bounded liveness/teardown | startup validated | algorithms, model evaluation, constraints, convergence, Level 3 workflow |
| AI Assistant GUI | preset, build, no-credential Qt6/XCB/Xvfb window, bounded teardown | startup validated | provider calls, credentials, redaction, failures, functional workflow |
| HTTP Worker GUI | target/preset known | not yet independently validated | bounded startup and workflow contract |
| Do Experiments GUI | intentionally absent | not started | backend/workflow/product specification |
| Model-specific apps | prior bounded sweep exists | partially validated, historical snapshot | revalidate failures, timeouts, model generation, Gro compile breakages |

## 7. Latest GUI validation integrations

### Optimizer GUI

- validation implementation PR: #507;
- implementation merge: `408c62dcd428de38708df4eac11cad287fb84f13`;
- evidence documentation PR: #508;
- evidence documentation merge: `fbf67b76c7d1e86e9431b680aa10de465d657b51`;
- focused workflow and ordinary CI: green for the recorded final head;
- artifact recorded by the evidence document: ID `8526654357`.

### AI Assistant GUI

- validation PR: #510;
- merge: `ca47f7f05b0414190fedf73da947dd3d5c5e2456`;
- focused workflow run: `29919067547`, success;
- ordinary CI run: `29919067854`, success;
- artifact: `genesys-ai-assistant-gui-validation`, ID `8529227458`;
- application window: `AI Assistant`, 860x680, associated with the application PID;
- bounded termination: exit code 143;
- residual application process: none;
- no provider credentials configured and no external AI request executed.

PR #506 was closed without merge because it was superseded by the corrected implementation integrated through PR #507.

## 8. Plugin architecture status

Confirmed generated evidence:

- `genesys_plugins_components` and `genesys_plugins_components_minimal` compile the same 84 component translation units;
- the targets use different GLPK compile configuration when GLPK is present;
- the effective runtime graph primarily uses the target named `minimal`;
- the focused continuous-diffusion test reaches both archives;
- duplicate compilation/storage is real;
- active duplicate-symbol failure was not demonstrated;
- GLPK feature selection is materially ambiguous.

Implementation is blocked on human decision issue #492. No target may be removed, renamed, aliased, repartitioned, or migrated dynamically until that decision is recorded.

## 9. Ownership and memory status

Confirmed for focused paths:

- temporary plugin-completion Model lifetime is RAII-managed;
- helper-owned Counter and StatisticsCollector responses are released;
- focused plugin-completion ASan/LeakSanitizer exits cleanly;
- `OptimizerDefaultImpl1` cannot be copied or moved while legacy owning pointers remain.

Not established:

- repository-wide leak freedom;
- complete thread safety;
- broad UBSan/Valgrind coverage;
- safe ownership for every legacy raw-pointer path.

## 10. Numerical, statistical, optimization, and scientific status

- legacy Simpson quadrature and unsupported derivative behavior are stabilized at the software-contract level;
- broad authoritative numerical/statistical reference packages remain pending;
- optimizer backend remains a scaffold despite GUI startup validation;
- biochemical and whole-cell features remain experimental/research-oriented;
- AI virtual-cell direction is strategic and deferred, not implemented predictive functionality;
- passing software tests do not establish biological, statistical, or numerical validity beyond their explicit oracle and scope.

## 11. Open human-decision blockers

| ID/issue | Decision | Current effect |
|---|---|---|
| #492 | canonical static component archive versus true minimal/full or common-core design | blocks plugin target correction |
| #496 | `autoloadplugins.txt` deployment/search/fallback contract | blocks shell autoload correction |
| #500 | worker bind-address default/configuration contract | blocks worker listener hardening |
| backlog entry `HUM-SEC-002` | worker authentication mechanism | blocks deployable controlled-intranet profile |
| backlog entry `HUM-SCI-001` | authoritative numerical/statistical references | blocks scientific validation claims |
| backlog entry `HUM-OPT-001` | first optimizer algorithm/research package | blocks real optimizer implementation |
| backlog entry `HUM-VC-001` | initial AI virtual-cell organism/use case/data package | blocks bounded scientific program |
| backlog entry `HUM-REL-001` | final supported set and promotion gate | blocks eventual `20262` promotion only |

## 12. Documentation migration status

Phase D0 under issue #511 has been reviewed and approved by the maintainer.

Current D0 state:

- PR #512 remains documentation-only;
- branch: `WiP20260722/ai-docs-governance`;
- ordinary CI run `29925861850`: success;
- `tests-unit` configure/build/CTest: passed;
- GUI GMDD diagnostics: passed;
- canonical documents and runbooks are present;
- no pre-existing Markdown file was moved or deleted;
- maintainer authorized merge and continuation to D1–D6.

The existing post-2026-11-01 `oldies/` retention gate remains unchanged.

## 13. Next eligible work

1. Mark PR #512 ready and merge it into `WorkInProgress`.
2. Create a new bounded branch from the resulting merge commit.
3. Execute D1: consolidate normative governance and architecture sources.
4. Continue D2–D6 in separate reviewable PRs until the structure is complete.

During the migration freeze, no unrelated source, CMake, runtime, plugin, security, numerical, application or release work is eligible.
