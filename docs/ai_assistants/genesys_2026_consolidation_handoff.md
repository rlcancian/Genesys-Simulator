# Handoff — GenESyS post-2026-1 consolidation plan

## 1. Final task state

- Status: `partially-validated`.
- Base branch inspected: `WorkInProgress`.
- Base commit inspected: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.
- Work branch created: `audit/genesys-2026-consolidation-plan-20260720`.
- Commits created: documentation-only commits on the work branch.
- Pull request: pending creation at the time of this handoff file commit.
- Main documents generated:
  - `docs/ai_assistants/genesys_2026_consolidation_plan.md`;
  - `docs/ai_assistants/genesys_2026_test_matrix.md`;
  - `docs/ai_assistants/genesys_2026_module_inventory.md`;
  - `docs/ai_assistants/genesys_2026_consolidation_handoff.md`.
- Existing documents updated:
  - `docs/ai_assistants/current_plans.md`.
- Local validation executed: none; audit performed through GitHub repository/PR inspection only.
- Short observation: the plan and inventories are ready for review, but all configure/build/test results require local or CI execution before status can become `validated`.

## 2. What was done

- Read the mandatory `docs/ai_assistants/README.md` and stable operational guides.
- Confirmed the canonical root CMake/Ninja/C++23 build structure.
- Confirmed the current `WorkInProgress` base commit.
- Inspected CMake target composition for kernel, parser, plugins, tools, applications, tests, and CI.
- Mapped current configure/build/test presets from `CMakePresets.json`.
- Inspected unit/smoke aggregation and detected test-target omissions/overlap risks.
- Inspected selected numerical, ownership, security-sensitive, and incomplete implementations.
- Sampled 2026 CMake/bootstrap and DCS pull requests.
- Distinguished historical PR claims from code confirmed in `WorkInProgress`.
- Created a prioritized consolidation plan with P0/P1/P2/P3 backlog and Phases 0–9.
- Created a test matrix with commands, dependencies, gaps, priorities, and acceptance criteria.
- Created a module inventory with status and future ownership by humans/AI.
- Added a minimal reference entry to `current_plans.md`.
- Created this operational handoff for future assistants.

## 3. Relevant comments made during execution

- [Environment] The task was adapted from a local Codex prompt to a GitHub-only ChatGPT Web audit; no local build/test result was invented.
- [Branches] `WorkInProgress` resolved directly, while historical `2026-1` and requested `build/cmake-bootstrap` did not resolve as current refs through the connector; historical PRs were not treated as current branch state.
- [Build] Root CMake is the canonical entry point, requires CMake 3.24/Ninja/C++23, and does not use `/projects` as its main graph.
- [Plugins] The current runtime links a static connector and static plugin libraries; the production build is not a dynamic-plugin architecture.
- [Plugins] The same component source tree is recursively compiled into full and minimal static plugin libraries; this must be mapped before any dynamic migration.
- [Tests] `genesys_test_ai_plugins` exists but is omitted from the inspected main unit aggregate/runner.
- [CI] The GUI GMDD diagnostics job executes known failing tests and can keep the PR baseline red until fixed or made intentionally non-blocking.
- [Numerics] `SolverDefaultImpl1` has confirmed correctness/undefined-behavior defects and is used by hypothesis-test CDF integration.
- [Optimization] `OptimizerDefaultImpl1` is an explicit scaffold, not a functioning optimization algorithm.
- [Security] Worker tokens use a non-cryptographic PRNG, and AI secret storage embeds the secret in a shell command string.
- [Ownership] Plugin template completion allocates a temporary `Model` without an observed release.
- [DCS] Several contributions were evaluated through controlled PRs; merge/evaluation history must not be confused with code present in the current branch.
- [Documentation] Root README and stable guides contain stale names, paths, presets, qmake statements, and sanitizer commands.
- [Oldies] All historical documents remain `needs-review`; none is deletion-ready merely because a stable guide exists.

## 4. Problems encountered

### 4.1 No local execution environment

- Type: environment.
- Where it occurred: CMake/Ninja/CTest, GUI, worker, sanitizer, packaging, and runtime validation.
- Evidence summarized: this assistant had GitHub connector access only.
- Impact: build/test statuses cannot be promoted to `validated`.
- Decision taken: record repository evidence and exact recommended commands without asserting execution.
- Status: `needs-local-validation`.
- Next recommended action: execute Phase 0 in a clean Ubuntu 24.04 checkout or through controlled CI.

### 4.2 Historical branches not resolvable

- Type: Git/history.
- Where it occurred: attempts to inspect/compare `2026-1` and `build/cmake-bootstrap` as current refs.
- Evidence summarized: direct current-file/ref lookup failed, while historical PR metadata still references `2026-1`.
- Impact: no reliable current branch diff could be produced.
- Decision taken: use PRs as historical evidence only and confirm current implementation directly in `WorkInProgress`.
- Status: `partially-validated`.
- Next recommended action: run local `git branch -a` and `git log --all` if full historical reconstruction is required.

### 4.3 Current CI baseline is ambiguous

- Type: CI/test.
- Where it occurred: `.github/workflows/genesys-ci.yml` and `current_plans.md`.
- Evidence summarized: one normal unit job coexists with a GUI GMDD diagnostic job that returns the known failing test result; prior plans report 16/1657 failures.
- Impact: a PR can remain red even when ordinary unit compilation succeeds; release readiness cannot be inferred.
- Decision taken: classify CI as P0 and require reproduction before other large work.
- Status: `blocked`.
- Next recommended action: run the audit PR, inspect both jobs, and decide which jobs are required status checks.

### 4.4 Plugin target overlap

- Type: build/architecture.
- Where it occurred: `source/plugins/CMakeLists.txt`, `source/plugins/components/CMakeLists.txt`, simulator runtime and continuous-diffusion test linkage.
- Evidence summarized: full and minimal targets recursively compile the same `.cpp` tree; some executables may link both transitively/directly.
- Impact: duplicate symbols, ODR/link-order behavior, increased build cost, and unclear future plugin boundaries.
- Decision taken: do not implement dynamic loading yet; first create an exact source-to-target/link map.
- Status: `blocked`.
- Next recommended action: generate link maps and redesign targets to be non-overlapping.

### 4.5 Numerical correctness blocker

- Type: code/scientific correctness.
- Where it occurred: `source/tools/Continuous/SolverDefaultImpl1.h/.cpp` and callers in hypothesis testing.
- Evidence summarized: `_stepSize` is uninitialized; RK4-like staging/state use is incorrect; three overloads return `0.0`; Simpson requirements are not enforced; non-standard VLA is used.
- Impact: undefined behavior and invalid numerical/statistical results.
- Decision taken: classify as P0; require tests before patch.
- Status: `blocked`.
- Next recommended action: add focused analytical regression tests, then apply the minimum correction or quarantine legacy calls.

### 4.6 Security-sensitive implementations

- Type: security.
- Where it occurred: worker token generation and AI secret storage.
- Evidence summarized: `std::mt19937_64` is used for access tokens; the secret is interpolated into a command string executed by the shell.
- Impact: predictable authorization material and possible same-user secret exposure through process command information.
- Decision taken: classify as P1 before network-facing/public deployment.
- Status: `blocked` for public deployment.
- Next recommended action: define secure OS-backed randomness and remove secret-bearing shell command composition.

### 4.7 Documentation drift

- Type: documentation.
- Where it occurred: root README and stable application/build guidance.
- Evidence summarized: obsolete terminal/web paths and target names, qmake delegation claims, missing current presets, and sanitizer presets that do not exist in current `CMakePresets.json`.
- Impact: developers and assistants can execute invalid workflows or misunderstand architecture.
- Decision taken: document the drift but defer broad correction until the build baseline is rerun.
- Status: `in-progress`.
- Next recommended action: update docs against actual target/preset output after Phase 0.

## 5. Corrections or adjustments made

> No functional correction was made; this round was limited to analysis, documentation, planning, and a minimal current-plan reference.

Documentation adjustments:

- File: `docs/ai_assistants/genesys_2026_consolidation_plan.md`.
  - Change: created the main evidence-based plan, backlog, phases, governance, and prompts.
  - Reason: provide a bounded consolidation roadmap.
  - Risk: low; claims still require local validation where marked.
  - Validation: Markdown review, path/ref review, PR review.

- File: `docs/ai_assistants/genesys_2026_test_matrix.md`.
  - Change: created test/preset/module matrix with acceptance criteria.
  - Reason: make validation executable and auditable.
  - Risk: low; current results are not claimed.
  - Validation: compare with `CMakePresets.json`, CMake test registration, and actual local/CI runs.

- File: `docs/ai_assistants/genesys_2026_module_inventory.md`.
  - Change: created module/build/test/documentation/risk inventory.
  - Reason: prevent broad work without impact mapping.
  - Risk: low; inventory is intentionally explicit about evidence level.
  - Validation: compare with current repository tree and build graph.

- File: `docs/ai_assistants/current_plans.md`.
  - Change: appended a small entry referencing the new plan, matrix, inventory, and handoff.
  - Reason: make the consolidation effort discoverable.
  - Risk: low; existing content was preserved.
  - Validation: internal links resolve in the PR branch.

## 6. Files created or changed

| File | Type | Purpose | Status |
|---|---|---|---|
| `docs/ai_assistants/genesys_2026_consolidation_plan.md` | new | main consolidation plan | created |
| `docs/ai_assistants/genesys_2026_test_matrix.md` | new | build/test/application/scientific validation matrix | created |
| `docs/ai_assistants/genesys_2026_module_inventory.md` | new | module and risk inventory | created |
| `docs/ai_assistants/genesys_2026_consolidation_handoff.md` | new | operational memory for future assistants | created |
| `docs/ai_assistants/current_plans.md` | changed | references to all consolidation artifacts | updated |

## 7. Validation performed

- Presets detected: `tests-unit`, `tests-kernel-unit`, `tests-smoke`, shell/model-specific, worker, main GUI, HTTP worker GUI, Data Analyser GUI, Optimizer GUI, and AI Assistant GUI variants.
- Unit build/test: `not-executed`; repository structure inspected.
- Kernel build/test: `not-executed`; repository structure inspected.
- GUI: `not-executed`; prior repository evidence reports three GMDD failures.
- Web/worker: `not-executed`; current executable is worker-oriented, and security risks were identified by source inspection.
- Shell/modelSpecific: `not-executed`; presets/targets inspected.
- CI/GitHub Actions: workflow inspected; audit-branch run pending PR creation.
- Limitations: GitHub-only environment, no compiler, no Qt runtime, no sanitizer, no package installation, no local branch graph.

## 8. State of the created plan

- Phases defined: 0 through 9.
- Priorities defined: P0, P1, P2, P3.
- Most critical modules/concerns:
  - CI and current failing tests;
  - legacy continuous solver/statistical integration;
  - whole-cell/runtime/metabolic failures;
  - overlapping/static plugin build architecture;
  - worker/AI security;
  - optimizer incompleteness/ownership;
  - parser FunctionRegistry status;
  - GUI GMDD behavior.
- First recommended actions:
  1. reproduce baseline;
  2. isolate P0 failures;
  3. repair aggregate test coverage;
  4. map plugin link graph;
  5. add regression tests before code changes.
- Main risks: scientific invalidity, undefined behavior, permanent red CI, ABI/lifecycle breakage, security exposure, misleading incomplete capabilities.
- Human decisions required: required CI checks, dynamic plugin ABI/API, Qt5 compatibility, optimizer algorithm/product status, biological/statistical acceptance criteria, release promotion.

## 9. Pending work

### 9.1 Technical pending work

- Correct or quarantine `SolverDefaultImpl1` after tests.
- Remove overlapping plugin source compilation.
- Define dynamic plugin ABI/API/version/lifecycle policy.
- Resolve optimizer ownership/copy semantics and implementation status.
- Fix secret/token security boundaries.
- Determine current parser FunctionRegistry implementation decision.
- Map ownership in Model/ModelSimulation/SimulationScenario/TraceManager/PluginManager.

### 9.2 Test pending work

- Execute `tests-unit`, `tests-kernel-unit`, and `tests-smoke`.
- Reproduce the 16 previously reported failures.
- Add AI plugin test to aggregate baseline.
- Add direct legacy solver and trusted statistical reference tests.
- Add WCM with/without GLPK matrix.
- Add plugin ABI/load/unload tests after design.
- Add worker auth/security tests.
- Add application startup smoke tests.

### 9.3 Documentation pending work

- Reconcile root README with actual presets/targets/layout.
- Correct application guide references to obsolete terminal/web paths.
- Review all `oldies/` files by theme.
- Validate Doxygen generation and internal links.
- Document supported SBML/whole-cell/parser/plugin contracts.

### 9.4 Governance pending work

- Decide required CI jobs and branch protection checks.
- Decide whether the audit PR should remain draft until CI evidence exists.
- Create issues or bounded branches for P0/P1 items.
- Update this handoff after every consolidation round.

### 9.5 Pending human decisions

- Dynamic plugin ABI/toolchain compatibility policy.
- Whether Qt5 fallback remains supported.
- Which statistical/numerical reference standards are authoritative.
- Optimizer algorithm and user-visible maturity label.
- Network-facing worker deployment/security policy.
- Whole-cell/biochemical semantic acceptance criteria.
- Branch promotion/release gate for 2026-2.

## 10. Recommended next actions

1. Reproduce the ordinary baseline.
   - Objective: determine exact current configure/build/test state.
   - Module: build/tests/CI.
   - Reference: `genesys_2026_test_matrix.md`.
   - Acceptance: exact failing tests and first failures recorded for all three baseline presets.
   - Execution: autonomous AI locally; no functional changes.

2. Inspect the audit PR CI.
   - Objective: determine whether the normal unit job and GUI diagnostic job behave as documented.
   - Module: GitHub Actions.
   - Reference: plan Phase 0.
   - Acceptance: run URLs/artifacts and required-check impact recorded.
   - Execution: AI; human decision if required checks change.

3. Add regression tests for the legacy solver.
   - Objective: prove each current numerical defect before correction.
   - Module: `source/tools/Continuous`.
   - Reference: test matrix P0 rows.
   - Acceptance: tests fail for the identified reasons on the unmodified implementation.
   - Execution: AI with numerical review.

4. Apply the minimum solver correction or quarantine.
   - Objective: eliminate UB and invalid results without rewriting the continuous subsystem.
   - Module: continuous/statistics.
   - Reference: plan Phase 1.
   - Acceptance: analytical and trusted-reference tests pass.
   - Execution: AI; scientific review required.

5. Repair unit-test aggregation.
   - Objective: ensure declared test targets, especially AI plugins, execute in CI.
   - Module: `source/tests/unit/CMakeLists.txt`.
   - Reference: test matrix aggregation section.
   - Acceptance: target appears in aggregate and CTest run.
   - Execution: autonomous AI.

6. Reproduce WCM/runtime/metabolic failures.
   - Objective: separate code defect, dependency mode, nondeterminism, and test defect.
   - Module: whole-cell/runtime/plugins.
   - Reference: plan P0 backlog.
   - Acceptance: each failure has a minimal filter, cause classification, and owner.
   - Execution: AI + domain review.

7. Generate the plugin source-to-target/link map.
   - Objective: eliminate uncertainty around full/minimal overlap.
   - Module: plugin CMake/runtime linkage.
   - Reference: plan Phase 2.
   - Acceptance: every plugin source maps to intended target(s), and overlaps are justified or removed.
   - Execution: autonomous AI; architecture review before edits.

8. Harden worker tokens and AI secret handling.
   - Objective: remove security blockers before public/network deployment.
   - Module: worker/AI tools.
   - Reference: plan P1 backlog.
   - Acceptance: OS-backed secure randomness and no secret in argv/logs.
   - Execution: AI with human security review.

9. Decide parser FunctionRegistry disposition.
   - Objective: determine whether #429 was rejected, renamed, superseded, or should be reintroduced.
   - Module: parser.
   - Reference: plan Phase 3.
   - Acceptance: explicit decision and compatibility test plan.
   - Execution: AI analysis; human semantic decision.

10. Reconcile documentation after baseline.
    - Objective: make all documented paths/presets/targets executable.
    - Module: README/stable guides.
    - Reference: plan Phase 8.
    - Acceptance: no stale command/path identified in this audit remains.
    - Execution: autonomous AI with review.

## 11. Guidance for the next AI assistant

Next assistant: first read `docs/ai_assistants/README.md`, then `genesys_2026_consolidation_plan.md`, `genesys_2026_test_matrix.md`, `genesys_2026_module_inventory.md`, and this handoff. The current state is a GitHub-only, documentation-level audit; it is not a validated build baseline. The safest next action is Phase 0: reproduce `tests-unit`, `tests-kernel-unit`, and `tests-smoke` on a clean branch from the designated base commit, record exact failures, and make no functional changes. Do not delete `oldies/`, do not migrate all plugins/namespaces/ownership at once, do not change scientific algorithms without failing regression tests and reference values, and do not alter stable branches directly. Preserve compiler/build/test logs, branch/commit identifiers, CTest names, dependency modes, and all evidence supporting status changes.

## 12. Limitations and uncertainties

- No local build/test/runtime/package execution was possible.
- No full local Git history/branch graph was available.
- The historical `2026-1` ref was not available for a current comparison through the connector.
- PR descriptions may contain unverified claims.
- Only selected high-risk sources were inspected deeply.
- Ownership candidates outside explicit source findings remain hypotheses to validate.
- Current exact test count/failure count is unknown until rerun.
- Qt GUI behavior, dynamic loading, external compilers, Python integration, and worker networking were not executed.
- Statistical, numerical, biological, and biochemical correctness requires independent reference validation and human domain review.

## 13. Operational result

Result: consolidation plan created and ready for review, with local/CI validation pending.
