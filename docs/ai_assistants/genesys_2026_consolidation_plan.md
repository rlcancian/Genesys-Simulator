# GenESyS 2026 Consolidation Plan

## 1. Objective

Establish an evidence-based, incremental consolidation plan for the GenESyS Simulator after the 2026-1 development cycle. The plan prioritizes reproducible build/test baselines, numerical correctness, ownership safety, plugin architecture, application separation, documentation consistency, and release readiness.

This document is a planning and audit artifact. It does not declare the repository consolidated and does not authorize broad refactoring.

## 2. Scope

Base branch and repository state:

- Repository: `rlcancian/Genesys-Simulator`.
- Base branch: `WorkInProgress`.
- Base commit: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.
- Commit date: 2026-07-10.
- Audit date: 2026-07-20.
- Audit environment: ChatGPT Web using the GitHub connector only.
- Local configure/build/test execution: not available in this audit environment.

Primary areas inspected:

- root `README.md`, `CMakeLists.txt`, and `CMakePresets.json`;
- `.github/workflows/genesys-ci.yml`;
- stable documents under `docs/ai_assistants/`;
- `source/kernel/`, `source/parser/`, `source/plugins/`, `source/tools/`, `source/applications/`, and `source/tests/` CMake entry points;
- selected ownership-sensitive, numerical, security-sensitive, and incomplete implementations;
- GitHub pull requests related to CMake bootstrap, DCS work, GUI separation, tests, CI, continuous simulation, cellular automata, parser work, data analysis, and optimization.

Out of scope for this round:

- local CMake/Ninja/CTest execution;
- exhaustive line-by-line audit of every source file;
- functional corrections;
- deletion or full consolidation of `docs/ai_assistants/oldies/`;
- grading or re-grading student work;
- broad namespace, ownership, plugin, or directory migration.

## 3. Confirmed repository state

### 3.1 Documentation read

The audit used the following stable documents as current guidance:

- `docs/ai_assistants/README.md`;
- `current_plans.md`;
- `build_ci_tests.md`;
- `branch_workflow.md`;
- `kernel_development.md`;
- `plugins_development.md` and selected files under `plugins/`;
- `applications_development.md`;
- `tools_and_statistics.md`;
- `modal_and_hybrid_simulation.md`;
- `whole_cell_and_sbml.md`;
- `python_integration.md`;
- `documentation_governance.md`;
- `consolidation_map.md`;
- `oldies_inventory.md`;
- `oldies_review_status.md`.

Confirmed documentation policy:

- stable documents take precedence over `oldies/`;
- historical claims require revalidation against current code;
- `oldies/` must not be removed before explicit review and the deletion gate after 2026-11-01;
- assistants must preserve small, reversible, reviewable changes.

### 3.2 Branch limitations

- `WorkInProgress` resolves to the base commit recorded above.
- The historical branch name `2026-1` appears in pull requests and workflow configuration, but it did not resolve as a current branch through the GitHub connector during this audit.
- `build/cmake-bootstrap` also did not resolve as a current branch.
- Historical PR evidence is therefore used only as historical evidence unless the resulting code is confirmed in `WorkInProgress`.

### 3.3 Build baseline

Confirmed in the root build files:

- CMake minimum version: 3.24.
- Preset schema: version 6.
- Generator: Ninja.
- Language standard: C++23, required, extensions disabled.
- Kernel, parser, plugins, tools, worker, shell, GUI, model-specific applications, and tests are entered from the root `CMakeLists.txt`.
- Legacy `project`/`projects` build entry points are obsolete according to the root README.
- The current root graph builds kernel, parser, plugin, tools, worker, and application targets predominantly as static libraries/executables.
- Google Test uses a system package when available and falls back to bundled sources.

Detected configure/build presets include:

- `tests-unit`;
- `tests-kernel-unit`;
- `tests-smoke`;
- `genesys_shell`;
- `terminal-smart`;
- `terminal-model-specific`;
- `genesys_modelspecific_app`;
- `worker-app` and `genesys_worker_app`;
- `gui-app`;
- `gui-httpworker`;
- `gui-dataanalyser`;
- `gui-optimizer`;
- `gui-ai-assistant`.

No sanitizer preset was found in the current `CMakePresets.json`, despite stale README commands for `asan` and `ubsan`.

### 3.4 Current CI baseline

`.github/workflows/genesys-ci.yml` currently:

- runs for pull requests targeting `2026-1`, `WorkInProgress`, `WiP20261`, `currentStable`, and `master`;
- supports manual dispatch;
- uses Ubuntu 24.04;
- installs CMake, Ninja, GCC, Qt6, Python development files, libSBML, ngspice, R, Octave, and OpenGL dependencies;
- configures, builds, and runs the `tests-unit` preset;
- contains a second GUI GMDD diagnostics job that deliberately executes three known failing tests and returns their exit status.

The workflow has no `push` trigger. No workflow run was associated with the audited base commit through the connector.

## 4. Executive summary

The repository has completed a substantial transition toward a root CMake/Ninja/C++23 build and contains a broad automated test structure. However, the current branch is not release-ready based on repository evidence.

The highest-priority findings are:

1. **Numerical correctness blocker:** `source/tools/Continuous/SolverDefaultImpl1` contains an uninitialized `_stepSize`, an incorrect/incomplete RK4-like implementation, unimplemented overloads returning `0.0`, and Simpson integration without enforcing its mathematical preconditions. Statistical p-value code depends on this integrator.
2. **CI blocker:** the current workflow includes a diagnostic job for three known failing GUI GMDD tests; existing plans report additional whole-cell/runtime/metabolic failures.
3. **Static plugin architecture:** all plugin components/data are recursively collected into static libraries, and the simulator runtime explicitly links a static connector implementation. This is not dynamic plugin packaging.
4. **Duplicate plugin compilation risk:** the same component source tree is compiled into both `genesys_plugins_components` and `genesys_plugins_components_minimal`; some tests link the full library while the runtime already links the minimal library.
5. **Security risks:** worker access tokens use `std::mt19937_64`; `AISecretStore` places the secret in a shell command string passed through `system()` despite a contrary comment.
6. **Incomplete optimization:** `OptimizerDefaultImpl1` is explicitly a scaffold; it does not generate/evaluate candidates, and its raw owning pointers have unsafe default copy semantics.
7. **Ownership/lifetime risk:** `Simulator::_completePluginsFieldsAndTemplate()` allocates a temporary model without an observed delete, and multiple public APIs return owning/observing raw pointers without an explicit ownership contract.
8. **Test aggregation gaps:** `genesys_test_ai_plugins` is declared but is not included in the main unit aggregate/runner; smoke tests are excluded from `tests-unit` by design.
9. **Branch/content divergence:** student work merged historically into `2026-1`, including cellular-automata tests, is not necessarily present in current `WorkInProgress`.
10. **Documentation drift:** root README and application/build guides contain obsolete preset names, paths, target names, qmake statements, sanitizer commands, and application layout descriptions.

The safest next move is Phase 0: restore a trustworthy CI/local baseline, isolate P0 failures, and produce reproducible evidence before architectural migrations.

## 5. 2026 change timeline

This timeline is based on current repository files, stable plans, and GitHub PR evidence. It is not a substitute for a complete local `git log --all` audit.

### Early 2026 — root CMake bootstrap and kernel/test graph

Confirmed themes from PRs beginning with #27 and subsequent bootstrap PRs:

- root `CMakeLists.txt` and `CMakePresets.json` introduced as the canonical build entry point;
- legacy `/projects` retained temporarily but no longer canonical;
- simulator split into support/runtime static libraries;
- parser, statistics, and plugin data moved into explicit CMake targets;
- bundled Google Test fallback added;
- unit and smoke test targets introduced;
- aggregate kernel/test targets and preset-driven workflows added.

### April 2026 — tools and application planning

Confirmed from stable/historical documentation:

- Data Analyser, Optimizer, DOE/RSM, and analysis-study concepts were planned;
- standalone workstation-style GUI direction established;
- separation between reusable backends under `source/tools/` and frontends under `source/applications/gui/` defined.

### May–June 2026 — AI, continuous/hybrid, biochemical, whole-cell, and external integration

Confirmed in current source/build/docs:

- AI provider clients and secret/configuration support added;
- continuous ODE solver factory, RK4, Dormand-Prince, diffusion/Method-of-Lines tests added;
- whole-cell and biochemical plugin trees and tests added;
- external integration through generated C++/Python-facing mechanisms documented;
- modal/hybrid temporal synchronization identified as an unresolved architectural contract.

### June–July 2026 — DCS contributions and controlled evaluations

Relevant PR groups include:

- cellular automata: #422, #427, #428, #430, controlled PRs #445–#448, and merged historical PR #453;
- data analysis/statistical tooling: #423, #424, #433, controlled PR #442;
- parser/FunctionRegistry: #429;
- optimization: #435 and controlled PR #441;
- dynamic linking evaluation: #438;
- controlled GUI animation: #440;
- CI bootstrap: #443/#444.

Important interpretation rule:

- PR descriptions are not proof of current implementation;
- the original `FunctionRegistry` files from #429 were not found at their stated paths in `WorkInProgress`;
- several cellular-automata test files described in merged PR #453 were not found in `WorkInProgress`;
- the optimizer scaffold is present, but the optimization algorithm is explicitly deferred.

### July 2026 — GUI separation, documentation migration, packaging, and CI

Confirmed in current source/docs:

- main GUI moved under `source/applications/gui/genesys`;
- HTTP worker, Data Analyser, Optimizer, and AI Assistant have standalone GUI targets;
- `doexperiments` remains unimplemented and fails configuration when enabled;
- documentation migrated under `docs/` with stable AI-assistant guides and retained `oldies/`;
- Ubuntu 24.04 GitHub Actions unit-test workflow added;
- Debian/Docker/OVA-related packaging work exists, but packaging validation is outside the ordinary unit-test job.

## 6. Current module map

| Area | Confirmed role | Current build shape | Test evidence | Status | Main risk |
|---|---|---|---|---|---|
| `source/kernel/util` | shared utilities/containers | static library | unit target | partially-validated | legacy containers/raw pointers |
| `source/kernel/statistics` | kernel statistics | static library | statistics unit target | partially-validated | numerical assumptions and edge cases |
| `source/kernel/simulator` | runtime, managers, persistence, model lifecycle | support/runtime static libraries | many focused unit targets | partially-validated | ownership hubs, static plugin connector |
| `source/parser` | expression/model parser | static library; optional Bison/Flex regeneration | parser expression tests | partially-validated | missing current FunctionRegistry feature/tests |
| `source/plugins/data` | plugin data definitions | one recursive static library | indirect + WCM tests | partially-validated | all domains aggregated; optional dependencies |
| `source/plugins/components` | model components | full and minimal recursive static libraries | selected plugin tests | partially-validated | duplicate compilation, no per-plugin library boundary |
| `source/tools` | AI, statistics, optimization, continuous solvers, DOE | one static library | selected numerical/tool tests | partially-validated | monolithic target; legacy solver P0; incomplete optimizer |
| `source/applications/shell` | command-line shell | executable | no confirmed dedicated smoke path | needs-local-validation | docs/target naming drift |
| `source/applications/worker` | HTTP/background worker | static core + executable | API router unit target | partially-validated | token generation/security; deployment hardening |
| `source/applications/gui` | independent Qt6 frontends | umbrella + independent targets | GUI render/GMDD tests | blocked | known failing GMDD tests; CI diagnostic job |
| `source/applications/modelSpecific` | generated/selected example apps | preset-selected executable | prior sweep documented | partially-validated | remaining runtime failures and branch drift |
| `source/tests/unit` | Google Test unit suite | many executables + aggregate target | CTest discovery | partially-validated | aggregate omissions; generated method tests are not behavioral coverage |
| `source/tests/smoke` | startup/continuous smoke tests | separate preset | CTest smoke label | needs-local-validation | excluded from ordinary unit baseline |
| `models` | persisted examples/models | runtime fixtures/examples | model-specific prior validation | needs-local-validation | compatibility after parser/plugin changes |
| `docs` | user/developer/AI documentation | source/PDF artifacts | manual review only | partially-implemented | extensive stale references and unreviewed oldies |
| `.github/workflows` | CI and packaging automation | GitHub Actions | workflow evidence only | blocked | intentional failing GUI diagnostics; no push baseline |

## 7. Student developments incorporated or evaluated

| Theme | PR evidence | Current branch evidence | Current classification | Required validation |
|---|---|---|---|---|
| Universal/user-defined cellular automata | #422, #428, #447, #453 | portions of CA code and neighborhood test visible; several #453 tests absent | partially-implemented | inventory exact files; rebuild all CA tests; persistence and compiler-loading tests |
| Non-uniform cellular automata | #430, #445, #446 | not exhaustively revalidated | needs-local-validation | compare controlled PR files with current tree; run focused tests |
| Parser `FunctionRegistry` | #429 | stated `FunctionRegistry.*` paths absent | not-started or incorporated differently | search current parser for equivalent registry; decide whether to reapply design |
| Data analysis/distribution fitting/inference | #423, #424, #433, #442 | current tools contain legacy/new statistical pieces, not the claimed independent package | partially-implemented | inventory algorithms and tests; validate against reference software/data |
| Optimization | #435, #441 | `OptimizerDefaultImpl1` scaffold present | partially-implemented | define algorithm contract; implement candidate generation/evaluation only after tests |
| Dynamic linking | #438 | runtime still uses static connector and static plugin libraries | not-started for production architecture | inspect student design as historical input; define stable ABI/API first |
| GUI animation | #440 | not exhaustively revalidated | needs-local-validation | identify accepted files and focused GUI tests |
| Continuous ODE/diffusion | current tests and DCS comments in CMake | RK4/Dormand-Prince/diffusion tests present | partially-validated | run tests; isolate legacy `SolverDefaultImpl1`; add hybrid-time regression tests |

No student grade is evaluated by this plan.

## 8. Build and test state

### 8.1 Validation performed in this audit

Performed through repository inspection:

- confirmed root CMake and preset definitions;
- confirmed test target registration and aggregation;
- confirmed current GitHub Actions workflow structure;
- confirmed current application/plugin/tool target composition;
- confirmed selected source-level defects/incompleteness.

Not performed:

- CMake configure;
- Ninja build;
- CTest execution;
- GUI startup;
- worker runtime smoke test;
- model load/save execution;
- sanitizer/Valgrind/profiling runs.

### 8.2 Existing evidence from repository documents

`current_plans.md` records a prior full test run with 16 failures out of 1657 tests:

- three GUI GMDD failures;
- thirteen non-GUI runtime/whole-cell/metabolic failures.

This is historical repository evidence and must be revalidated on the audit branch/base before being treated as the current result.

### 8.3 Baseline commands for local/CI execution

Run from repository root on Ubuntu 24.04:

```bash
cmake --list-presets=all
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

Then:

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

And smoke tests:

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

Application presets must be validated separately.

## 9. Prioritized consolidation backlog

| Priority | Area | Problem | Evidence | Recommended action | Validation | Risk |
|---|---|---|---|---|---|---|
| P0 | Continuous/statistics | legacy solver uses uninitialized `_stepSize`; RK4 and Simpson contracts are incorrect/incomplete | `SolverDefaultImpl1.h/.cpp` | quarantine or repair with focused numerical tests; prevent statistical p-values from depending on an invalid integrator | analytical integrals/ODEs, UBSan, regression against Boost/R/Python reference values | wrong scientific results/UB |
| P0 | CI/GUI | diagnostics job runs known failing tests and can keep every PR red | workflow + `current_plans.md` | reproduce failures, fix or temporarily separate diagnostic workflow from required baseline | PR CI green with required tests; diagnostics retained as non-blocking artifact if needed | unreliable merge gate |
| P0 | WCM/runtime/metabolic | prior evidence reports 13 non-GUI failures | `current_plans.md` | reproduce and classify first failing test per subsystem | focused CTest filters and deterministic fixtures | silent scientific/runtime regressions |
| P1 | Plugin build | same component tree compiled into full and minimal static libraries | plugin CMake + diffusion test link graph | define non-overlapping target composition before dynamic migration | link-map review, clean build, all plugin tests | duplicate symbols/ODR/build bloat |
| P1 | Plugin architecture | runtime hard-wires static connector and static plugin targets | simulator CMake | define plugin ABI/API/version contract and build one pilot plugin as MODULE/SHARED | host/plugin compatibility matrix and load/unload tests | ABI/lifetime breakage |
| P1 | Security | worker access tokens use `mt19937_64` | `TokenService.cpp` | use OS CSPRNG or vetted cryptographic source; define token entropy/expiry/storage | deterministic interface tests + security review | predictable authorization tokens |
| P1 | Security | secret is embedded in shell command string | `AISecretStore.cpp` | remove shell command composition; use direct process/pipe or Secret Service API without secret in argv | process inspection test; failure-path tests | secret exposure |
| P1 | Optimization | implementation is a scaffold but exposed as executable/backend | optimizer source/header | mark capability experimental; define algorithm and acceptance tests before implementation | benchmark functions and model-control integration tests | misleading functionality |
| P1 | Ownership | optimizer owns seven raw allocations and is copyable by default | optimizer header/cpp | delete copy/move initially or migrate containers to value/RAII after callers are mapped | compile-time copy tests + ASan/LSan | double delete/leak |
| P1 | Ownership | temporary model allocated without observed release | `Simulator::_completePluginsFieldsAndTemplate()` | convert to automatic/RAII lifetime after confirming plugin-created object ownership | focused repeated-call LSan test | repeated leak |
| P1 | Tests | AI plugin test omitted from aggregate and runner | unit CMake | add target to aggregate and CTest baseline | test listed and executed in `ctest -N`/CTest log | untested integration |
| P1 | Parser | FunctionRegistry PR not represented at stated paths | PR #429/current tree | decide whether feature was intentionally dropped, renamed, or needs reintroduction | parser registry unit/error/compatibility tests | model expression incompatibility |
| P1 | Hybrid time | no formal synchronization contract between event time and integration time | hybrid docs/current plugins | define explicit time advancement contract before behavior changes | analytical hybrid examples and event-boundary tests | temporal inconsistency |
| P2 | Documentation | README/build guides contain obsolete presets, paths, targets, qmake claims | root README and stable docs vs CMake | update documentation only after baseline validation | link/path/preset verification | developer confusion |
| P2 | Tests | Qt5 fallback conflicts with Qt6 baseline | unit CMake | decide whether Qt5 compatibility is supported; otherwise remove in separate change | Qt6-only CI configure | hidden compatibility drift |
| P2 | Tools architecture | all tool domains are one static library | tools CMake | split only after dependency graph and tests exist | per-domain targets and link checks | cyclic dependencies/build coupling |
| P2 | Oldies | every historical file remains `needs-review` | oldies status matrix | review in themed batches without deleting files | status matrix updated with evidence | stale guidance persists |
| P3 | Namespaces | global symbols across kernel/parser/plugins/tools | source headers | incremental namespace migration after ABI boundaries are defined | compile/API compatibility tests | high churn if premature |

## 10. Initial top-ten lists

### 10.1 Top 10 priority corrections

1. Correct/quarantine `SolverDefaultImpl1`.
2. Restore a reliable required CI baseline.
3. Reproduce and classify WCM/runtime/metabolic failures.
4. Remove duplicate plugin source compilation from overlapping targets.
5. Execute `genesys_test_ai_plugins` in the aggregate baseline.
6. Replace worker token generation with a cryptographically secure source.
7. Remove secrets from shell command arguments/strings.
8. Close optimizer copy/ownership hazards and label it experimental.
9. Fix the temporary-model leak in plugin template completion.
10. Reconcile parser `FunctionRegistry` status and compatibility requirements.

### 10.2 Top 10 safe modernizations

These are candidates, not blanket edits:

1. Replace owning raw manager/container pointers with `std::unique_ptr` where ownership is proven.
2. Delete copy operations for ownership-heavy classes until value/RAII semantics are defined.
3. Replace manual temporary heap objects with automatic objects or scoped smart pointers.
4. Use `std::vector`/`std::array` instead of non-standard variable-length arrays.
5. Add `[[nodiscard]]` to result/status APIs where ignoring failure is unsafe.
6. Add `const` to read-only getters and numerical methods after caller review.
7. Use scoped enums only where persistence/ABI mappings are explicit.
8. Replace duplicate CMake source globs with non-overlapping explicit/domain target composition.
9. Add sanitizer presets as opt-in diagnostic presets after local validation.
10. Introduce per-domain tool/plugin interface targets before moving implementation libraries.

### 10.3 Top 10 test gaps

1. Direct tests for every `SolverDefaultImpl1` overload and precondition.
2. Statistical p-values/critical values against trusted reference datasets/software.
3. Empty, degenerate, non-finite, and small-sample statistical inputs.
4. Optimizer candidate generation, constraints, reproducibility, and best-solution ranking.
5. AI plugin test inclusion in ordinary CTest baseline.
6. Plugin dynamic load/version/ABI/lifecycle tests.
7. Repeated plugin-template completion under ASan/LSan.
8. Worker token entropy/authorization/expiry and malformed request tests.
9. Parser FunctionRegistry nominal/error/backward-compatibility tests.
10. Hybrid event-boundary/time-step synchronization tests.

### 10.4 Top 10 ownership/lifetime candidates

Status means audit priority, not proof of a leak unless explicitly stated.

1. `Simulator::_completePluginsFieldsAndTemplate()` temporary `Model` — confirmed unreleased allocation in inspected method.
2. `OptimizerDefaultImpl1` owning lists — confirmed raw ownership with unsafe default copy semantics.
3. `PluginManager` connector/plugins/system executor — explicit manual ownership; constructor injection ownership contract must be documented.
4. `PluginManager::autoInsertPlugins()` returned `List<Plugin*>*` — caller ownership unclear.
5. `PluginManager::discoverPluginFilenames()` returned `List<std::string>*` — caller ownership unclear.
6. `Model` managers/data/components — historical ownership hub requiring current source audit.
7. `ModelSimulation` controls/events/reporters — historical lifetime-sensitive area.
8. `SimulationScenario` copy/replacement/destruction semantics — historical coupling risk.
9. `TraceManager` handler/list lifetime — historical lifetime-sensitive area.
10. C++/Python wrappers and external integration generated libraries — cross-runtime lifetime and unload risk.

## 11. Phase plan

### Phase 0 — baseline, inventory, and CI

- **Status:** `in-progress`.
- **Objective:** obtain a reproducible green/known-red baseline.
- **Tasks:** run all presets; record toolchain; isolate first failures; remove required dependence on diagnostic-only jobs; verify test aggregation.
- **Acceptance:** every required job has an intentional pass/fail contract; failure list is reproducible and assigned.
- **Execution:** AI autonomous for evidence gathering; human decision for changing required status checks.

### Phase 1 — minimum build/test stabilization

- **Status:** `not-started`.
- **Objective:** correct P0 numerical/build/runtime blockers without broad refactoring.
- **Tasks:** solver correction/quarantine; WCM/runtime failure triage; AI test aggregation; smoke baseline.
- **Acceptance:** unit/kernel/smoke presets pass or have explicitly approved quarantines with issues.
- **Execution:** AI with human review for scientific/numerical changes.

### Phase 2 — plugin target and template consolidation

- **Status:** `not-started`.
- **Objective:** remove overlapping static source aggregation and classify every plugin domain.
- **Tasks:** build target graph; plugin inventory; metadata/factory/persistence matrix; pilot dynamic boundary design.
- **Acceptance:** no source compiled into conflicting plugin aggregate libraries; every plugin classified.
- **Execution:** AI inventory; human architectural approval before ABI changes.

### Phase 3 — parser and FunctionRegistry

- **Status:** `not-started`.
- **Objective:** establish the actual current parser extension contract.
- **Tasks:** locate equivalent functionality; compare #429; specify registry API; add compatibility tests for `.gen` models.
- **Acceptance:** documented supported functions, deterministic unknown-function errors, regression fixtures.
- **Execution:** AI analysis/testing; human decision on public parser semantics.

### Phase 4 — continuous, ODE/PDE, diffusion, and ModalModel

- **Status:** `not-started`.
- **Objective:** validate numerical and hybrid-time semantics.
- **Tasks:** analytical ODE tests; convergence; diffusion conservation/known solutions; event-step synchronization; modal persistence/execution fixtures.
- **Acceptance:** solver tolerances and time contract documented; deterministic tests pass.
- **Execution:** AI implementation with human/scientific review.

### Phase 5 — biochemical, biological, TinkerCell/Gro, SBML, and whole-cell

- **Status:** `not-started`.
- **Objective:** separate native model semantics from interoperability and validate optional dependencies.
- **Tasks:** plugin inventory; SBML support matrix; GLPK/fallback tests; stochastic/biochemical fixtures; TinkerCell/Gro status determination.
- **Acceptance:** supported/unsupported constructs explicit; no silent data loss; core fixtures pass.
- **Execution:** joint AI/domain-expert work; biological semantics require human review.

### Phase 6 — GUI, worker, shell, AI assistant, and distributed execution

- **Status:** `in-progress` for GUI separation; otherwise `partially-implemented`.
- **Objective:** stabilize independent application targets and security boundaries.
- **Tasks:** fix GMDD tests; validate each GUI preset; harden worker tokens/API; define process/context handoff; document MCP/distributed status.
- **Acceptance:** each executable configures/builds/starts independently; security review complete for network-facing components.
- **Execution:** AI for build/test work; human approval for public deployment/security policy.

### Phase 7 — statistics, data analysis, DOE, and optimization

- **Status:** `partially-implemented`.
- **Objective:** distinguish valid algorithms from scaffolds and GUI prototypes.
- **Tasks:** trusted numerical references; small-sample policy; distribution fitting tests; FactorialDesign specification; real optimizer algorithm selection.
- **Acceptance:** every exposed operation has documented assumptions and reference-backed tests.
- **Execution:** joint AI/statistics expert review.

### Phase 8 — user/developer documentation

- **Status:** `in-progress`.
- **Objective:** align README, stable guides, Doxygen, packaging, and application names with code.
- **Tasks:** remove stale presets/paths; fix internal links; review oldies by theme; regenerate final docs locally.
- **Acceptance:** all documented commands/paths exist; Doxygen outputs reproducibly generated; oldies matrix updated.
- **Execution:** AI autonomous for mechanical verification; human approval for deletion/obsolete status.

### Phase 9 — release readiness for 2026-2

- **Status:** `not-started`.
- **Objective:** produce a controlled release candidate.
- **Tasks:** clean build matrix; unit/smoke/application/package validation; sanitizer pass; model compatibility set; release notes and rollback plan.
- **Acceptance:** required CI green, packaging artifacts validated, known limitations documented, branch promotion approved.
- **Execution:** human release decision with AI-generated evidence.

## 12. Governance for AI assistants

Every continuation must:

1. read `docs/ai_assistants/README.md` first;
2. read this plan, the test matrix, module inventory, and current handoff;
3. work from the actual designated base branch and record its commit;
4. create one branch per bounded task;
5. use small commits with one concern per commit;
6. preserve facts, inferences, and hypotheses as separate labels;
7. run CMake/Ninja/CTest locally or through CI when execution is available;
8. attach failure logs or concise diagnostics to the relevant plan/handoff;
9. avoid deleting `oldies/` or historical files without explicit review;
10. avoid broad ownership, namespace, plugin, or directory migrations without an approved impact map;
11. create draft PRs for multi-phase or high-risk changes;
12. update `genesys_2026_consolidation_handoff.md` at the end of each round.

Use only these task/module statuses:

- `not-started`;
- `in-progress`;
- `blocked`;
- `needs-human-decision`;
- `needs-local-validation`;
- `needs-ci-validation`;
- `validated`;
- `partially-validated`;
- `implemented`;
- `partially-implemented`;
- `deferred`;
- `obsolete`.

## 13. Handoff and tracking by AI assistants

The current operational state is maintained in `genesys_2026_consolidation_handoff.md`.

Each future consolidation round must update that handoff with:

- branch and commit inspected;
- evidence gathered;
- build/test results;
- changes made;
- blocked and pending items;
- the next smallest safe action.

Raw command output belongs in local/CI logs or artifacts, not in the main plan. Decisions and status changes must cite repository files, tests, PRs, issues, or logs. Future actions must remain incremental and independently reviewable.

## 14. Recommended next prompts for a local coding agent

### Prompt 1 — reproduce baseline

Read `docs/ai_assistants/README.md`, then this plan, test matrix, module inventory, and handoff. On a clean branch from `WorkInProgress`, run the `tests-unit`, `tests-kernel-unit`, and `tests-smoke` configure/build/CTest flows. Do not change code. Record toolchain, first failure per target, failing tests, and whether the GUI diagnostics job reflects the same failures.

### Prompt 2 — isolate legacy solver correctness

Read the mandatory AI-assistant documentation. Audit only `source/tools/Continuous/SolverDefaultImpl1.*`, its interface, callers, and tests. Add focused regression tests that demonstrate current defects before proposing the minimum correction. Do not alter the newer `OdeSolverFactory` solvers unless a shared defect is proven.

### Prompt 3 — repair test aggregation

Read the mandatory documentation. Audit `source/tests/unit/CMakeLists.txt` and CTest discovery. Determine which declared test executables are omitted from aggregate targets or the default CI build. Add only the missing dependencies/registration required to make the test baseline complete, then validate with `ctest -N` and CTest.

### Prompt 4 — plugin target graph

Read the mandatory documentation. Produce an exact source-to-target map for `source/plugins`. Identify files compiled into more than one static library and executables that link overlapping plugin libraries. Do not implement dynamic loading yet. Propose a non-overlapping target graph and validate it with link maps and the existing tests.

### Prompt 5 — worker/AI secret hardening

Read the mandatory documentation. Audit `TokenService`, `AISecretStore`, provider clients, and their callers. Add tests that demonstrate the current security weaknesses. Propose the minimum platform-compatible secure implementation without adding a dependency unless justified.

## 15. Remaining risks

- No local build/test was executed in this GitHub-only audit.
- Current branch list and historical branch comparisons were constrained by connector behavior.
- Pull-request bodies may contain unverified author claims.
- The complete commit history since 2026-01-01 was not reconstructed month by month through a local Git graph.
- Ownership and leak findings outside explicitly inspected methods remain candidates, not confirmed defects.
- Biological, biochemical, statistical, and numerical correctness requires domain-expert review.
- Dynamic plugin migration requires an explicit ABI/version/toolchain policy before implementation.
- GUI behavior cannot be validated without Qt6 build and display/offscreen runtime execution.

## 16. Appendices

### 16.1 Primary build files

- `CMakeLists.txt`;
- `CMakePresets.json`;
- `source/kernel/simulator/CMakeLists.txt`;
- `source/parser/CMakeLists.txt`;
- `source/plugins/CMakeLists.txt`;
- `source/plugins/data/CMakeLists.txt`;
- `source/plugins/components/CMakeLists.txt`;
- `source/tools/CMakeLists.txt`;
- `source/tests/CMakeLists.txt`;
- `source/tests/unit/CMakeLists.txt`;
- `source/tests/smoke/CMakeLists.txt`.

### 16.2 Selected source evidence

- `source/tools/Continuous/SolverDefaultImpl1.h/.cpp`;
- `source/tools/Statistics/HypothesisTesterDefaultImpl1.cpp`;
- `source/tools/Optimization/OptimizerDefaultImpl1.h/.cpp`;
- `source/tools/AIAssistant/AISecretStore.cpp`;
- `source/applications/worker/auth/TokenService.cpp`;
- `source/kernel/simulator/Simulator.h/.cpp`;
- `source/kernel/simulator/PluginManager.cpp`.

### 16.3 Relevant PRs sampled

- CMake/bootstrap: #27–#40 sampled through GitHub search;
- DCS/CA/data/parser/optimization: #422–#453 sampled selectively;
- CI: #443 and #444;
- GUI separation: #451.

### 16.4 Associated consolidation files

- `genesys_2026_test_matrix.md`;
- `genesys_2026_module_inventory.md`;
- `genesys_2026_consolidation_handoff.md`;
- `current_plans.md`.
