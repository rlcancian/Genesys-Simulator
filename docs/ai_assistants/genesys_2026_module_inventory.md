# GenESyS 2026 Module Inventory

## Purpose and evidence level

This inventory maps the current GenESyS repository structure as inspected on branch `WorkInProgress`, commit `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`, on 2026-07-20.

Evidence levels:

- **Confirmed in current code/build:** directly observed in current files.
- **Confirmed in current documentation:** recorded in stable repository documents, but may still require execution.
- **Historical evidence:** derived from PRs or `oldies/`; not treated as current implementation without revalidation.
- **Hypothesis to validate:** plausible but not proven in this audit.

The audit environment was ChatGPT Web with GitHub connector only. Build/test status marked `needs-local-validation` was not executed in this environment.

## Root and governance

| Directory/file | Purpose | Main targets/artifacts | Build status | Test status | Documentation status | Risk | Future owner |
|---|---|---|---|---|---|---|---|
| `CMakeLists.txt` | canonical repository build entry point | kernel, parser, plugins, tools, shell, worker, GUI, tests | confirmed structure; needs-local-validation | indirect | partially documented | option/preset/direct-build drift | AI + human |
| `CMakePresets.json` | reproducible Ninja/C++23 workflows | unit, kernel, smoke, shell, worker, GUI variants | confirmed structure; needs-local-validation | test presets defined | partially documented | README contains stale presets | AI |
| `README.md` | repository overview and quick start | documentation only | not applicable | not applicable | stale/inconsistent | obsolete options, paths, qmake and sanitizer claims | AI with human review |
| `.github/workflows/genesys-ci.yml` | Ubuntu 24.04 CI | `tests-unit`, GUI GMDD diagnostics | needs-ci-validation | known prior failures | documented in plans | diagnostic job may keep PR red | AI + human status-check decision |
| `docs/ai_assistants/` | operational guidance for humans/AI | stable guides and plans | not applicable | manual validation | in-progress | internal contradictions/stale paths | AI + human |
| `docs/ai_assistants/oldies/` | temporary historical documentation | historical Markdown | not applicable | not applicable | all files `needs-review` | stale claims; premature deletion | human approval + AI review |
| `docs/users/`, `docs/developers/` | Doxygen sources/final PDFs | documentation artifacts | needs-local-validation | documentation build not audited | partially-implemented | generated output/version drift | AI + human |
| `models/` | example and persisted simulation models | `.gen` and domain fixtures | needs-local-validation | selected model-specific evidence only | partially documented | parser/plugin backward compatibility | AI + domain experts |
| legacy `projects/` | temporary historical build material, not canonical | none in root graph | obsolete as entry point | not applicable | root README marks obsolete | accidental use by old docs/scripts | human decision before removal |

## Kernel

### `source/kernel/util`

- **Purpose:** common utility classes, custom containers, identifiers, filesystem/path helpers, and shared low-level support.
- **Principal target:** `genesys_kernel_util`.
- **Confirmed dependencies:** broadly consumed by kernel, parser, plugins, tools, and applications.
- **Build:** static library in root graph; `needs-local-validation`.
- **Tests:** `genesys_test_util`; `partially-validated` by repository structure.
- **Documentation:** covered generally by kernel guidance; no complete current API/ownership map.
- **Risks:** custom container semantics, raw-pointer collections, implicit ownership, global ID state, broad coupling.
- **Recommended future owner:** AI for inventory/tests; human review for public API changes.

### `source/kernel/statistics`

- **Purpose:** core statistical collectors/data structures used by simulator and tools.
- **Principal target:** `genesys_kernel_statistics`.
- **Build:** static library; `needs-local-validation`.
- **Tests:** `genesys_test_statistics`; current exact correctness not executed.
- **Documentation:** statistical correctness policy exists in `tools_and_statistics.md`.
- **Risks:** edge domains, non-finite/empty data, interaction with higher-level inference code.
- **Recommended future owner:** AI + statistics expert.

### `source/kernel/simulator`

- **Purpose:** simulator root, managers, model runtime, event scheduling, persistence, plugin registration, experiments, tracing, parser facade, and essential plugins.
- **Principal targets:**
  - `genesys_kernel_simulator_support`;
  - `genesys_kernel_simulator_runtime`;
  - aggregate `genesys_kernel`.
- **Build composition:** static support/runtime split. Runtime explicitly adds `source/plugins/PluginConnectorStaticImpl1.cpp` and `PluginConnectorDummyImpl1.cpp` and links static plugin/parser/statistics libraries.
- **Tests:** many focused unit executables for support/runtime/managers/persistence; prior plans report current failures requiring revalidation.
- **Documentation:** stable kernel ownership guidance exists.
- **Confirmed risks:**
  - manual manager ownership in `Simulator`;
  - `Simulator::_completePluginsFieldsAndTemplate()` allocates a temporary `Model` without observed release;
  - static plugin connector is part of runtime target;
  - raw-pointer return APIs and implicit ownership contracts;
  - broad exception swallowing in plugin template completion.
- **Historical high-attention classes:** `Model`, `ModelSimulation`, `SimulationScenario`, `TraceManager`, `PluginManager`.
- **Recommended future owner:** AI for tests/local fixes; human architecture review for ownership/plugin API changes.

## Parser

### `source/parser`

- **Purpose:** expression/model parser, generated scanner/parser sources, semantic parsing infrastructure.
- **Principal target:** `genesys_parser` static library.
- **Generation:** optional Bison/Flex regeneration through `GENESYS_PARSER_REGENERATE`.
- **Tests:** `genesys_test_parser_expressions`; additional historical FunctionRegistry tests are not currently confirmed.
- **Documentation:** stable parser coverage is incomplete; #429 is historical evidence.
- **Confirmed current state:** `FunctionRegistry.h/.cpp` from PR #429 are absent at their stated paths in `WorkInProgress`.
- **Risks:** divergence between generated sources and grammar, unknown-function behavior, `.gen` compatibility, registry feature loss/renaming.
- **Status:** `partially-validated` for current parser; FunctionRegistry status `needs-human-decision` after code search.
- **Recommended future owner:** AI for inventory/tests; human decision on language semantics.

## Plugins

### `source/plugins/data`

- **Purpose:** all plugin data definitions, including general, biochemical, whole-cell, AI/external, continuous/modal-related data domains.
- **Principal target:** `genesys_plugins_data` static library.
- **Build:** recursively compiles every `.cpp` under the tree into one target; optional Python link.
- **Tests:** indirect runtime tests plus WCM/AI/domain tests; no complete per-domain target matrix.
- **Risks:** domain coupling, optional dependency leakage, lack of per-plugin packaging, large ABI surface.
- **Status:** `partially-validated`.
- **Recommended future owner:** AI inventory; domain experts for biological/statistical semantics.

### `source/plugins/components`

- **Purpose:** all model component plugins across standard discrete event, modal, continuous, whole-cell, external integration, and other domains.
- **Principal targets:**
  - `genesys_plugins_components`;
  - `genesys_plugins_components_minimal` created one directory above.
- **Build:** both targets recursively compile the same component `.cpp` tree.
- **Tests:** selected component tests; continuous diffusion target links full components while runtime already links minimal components.
- **Confirmed risk:** overlapping source compilation and possible duplicate-symbol/ODR/link-order behavior.
- **Status:** `blocked` pending link/build validation and target redesign.
- **Recommended future owner:** AI for exact graph; human approval for architecture.

### Standard/general-purpose plugins

- **Purpose:** discrete-event components and model data definitions.
- **Build:** aggregated into global plugin static targets.
- **Tests:** indirect coverage; no current per-plugin classification completed.
- **Documentation:** `plugins/other_plugins.md` and historical matrices.
- **Risk:** persistence/factory/metadata regressions hidden by aggregate compilation.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI.

### Continuous/hybrid plugins

- **Observed path:** `source/plugins/components/Continuous/`.
- **Known classes:** LSODE and differential-equation components, plus diffusion integration elsewhere.
- **Build:** included in aggregate component libraries.
- **Tests:** smoke continuous/LSODE and DCS ODE/diffusion tests.
- **Risk:** no explicit synchronization contract between solver step and discrete-event time.
- **Status:** `partially-validated`, with hybrid semantics `not-started`.
- **Recommended future owner:** AI + numerical simulation expert.

### ModalModel / EFSM / Petri / submodel

- **Observed path:** `source/plugins/components/ModalModel/`.
- **Known concepts:** states, places, nodes, transitions, submodels, cellular automata.
- **Build:** aggregate static plugin targets.
- **Tests:** cellular-neighborhood test confirmed; complete modal/persistence/runtime matrix absent.
- **Risk:** state-transition semantics, persistence, internal network lifecycle, interaction with hybrid time.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI + modeling domain reviewer.

### Cellular automata

- **Observed path:** under `source/plugins/components/ModalModel/CellularAutomata/` and component wrappers.
- **Historical PRs:** #422, #427, #428, #430, #445–#448, #453.
- **Current evidence:** neighborhood target exists; multiple #453 test files are absent in current branch.
- **Risk:** branch divergence, runtime compilation/loading of user rules, temporary files, persistence, ownership.
- **Status:** `partially-implemented` / `needs-local-validation`.
- **Recommended future owner:** AI with security review for generated-code execution.

### Biochemical simulation

- **Observed path:** `source/plugins/data/BiochemicalSimulation/`.
- **Known concepts:** biochemical networks and simulator runner.
- **Build:** aggregate data library.
- **Tests:** no complete dedicated current matrix confirmed.
- **Risk:** mathematical semantics, SBML mapping, integration with whole-cell layer, optional dependencies.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI + biochemical modeling expert.

### Whole-cell modeling

- **Observed paths:**
  - `source/plugins/components/WholeCellModeling/`;
  - `source/plugins/data/WholeCellModeling/`.
- **Known concepts:** stochastic reactions, cell state/growth/division/fate, transcription/translation, compartments, metabolic projection/FBA.
- **Tests:** `genesys_test_wcm_plugins` contains substantial focused tests.
- **Prior evidence:** current plans record runtime/metabolic failures.
- **Dependency:** optional GLPK with a built-in fallback described as suitable only for small models.
- **Risk:** scientific correctness, stochastic reproducibility, fallback validity, serialization, event timing.
- **Status:** `blocked` until prior failures are reproduced/classified.
- **Recommended future owner:** AI + biological/operations-research experts.

### External integration / Python-facing plugins

- **Observed paths:**
  - `source/plugins/components/ExternalIntegration/`;
  - `source/plugins/data/ExternalIntegration/`.
- **Known concepts:** `CppForG`, `CppCompiler`, generated code, dynamic loading, Python-facing simulator facade.
- **Build:** aggregate plugin libraries; Python integration optional.
- **Tests:** complete current sandbox/path/lifetime matrix not confirmed.
- **Risk:** command execution, generated code, temporary directories, dynamic library unload, package dependencies, C++/Python lifetime.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI + security/hardening review.

### Dynamic plugin architecture

- **Current state:** host runtime is compiled with a static connector and static plugin libraries.
- **Production dynamic library boundary:** not established by current CMake.
- **Historical input:** DCS dynamic-linking evaluation PR #438.
- **Required design decisions:** minimal stable interface, factory/registration API, metadata schema, ABI version, compiler/standard library compatibility, ownership/unload policy, symbol visibility, dependency diagnostics.
- **Status:** `not-started` as a production architecture migration.
- **Recommended future owner:** human architecture decision with AI analysis/implementation.

## Tools

### `source/tools`

- **Purpose:** reusable backends for AI assistant, statistics, optimization, continuous solvers, and factorial design.
- **Principal target:** `genesys_tools` static library.
- **Build:** one target explicitly lists sources from all tool domains.
- **Risk:** domain coupling and inability to validate/package tools independently.
- **Status:** `partially-implemented`.
- **Recommended future owner:** AI for dependency map; human review before target split.

### Continuous numerical tools

- **Observed classes:** legacy `SolverDefaultImpl1`, newer RK4/Dormand-Prince/factory, diffusion/MOL support.
- **Tests:** newer solvers have analytical/convergence tests.
- **Confirmed P0:** legacy solver has uninitialized `_stepSize`, incomplete/incorrect RK4-like behavior, unimplemented overloads returning zero, non-standard VLA usage, and Simpson precondition problems.
- **Dependency impact:** hypothesis-testing CDFs integrate PDFs using this legacy solver.
- **Status:** `blocked`.
- **Recommended future owner:** AI + numerical methods reviewer.

### Statistics / inference / fitting

- **Observed classes:** `HypothesisTesterDefaultImpl1`, probability distributions, fitter dummy, simulation-results dataset.
- **Tests:** hypothesis tester, statistics, dataset targets.
- **Risks:** p-values depend on flawed numerical integration; approximation domains/small-sample behavior; empty-file division; equal-variance selection policy marked TODO.
- **Status:** `blocked` for scientific claims until reference validation.
- **Recommended future owner:** AI + statistics expert.

### Optimization

- **Observed classes:** `Optimizer_if`, `OptimizerDefaultImpl1` and GUI frontend.
- **Current behavior:** scaffold only; `step()` increments counters and does not generate/evaluate candidates; model-file loading is deferred; best-solution list stays empty.
- **Ownership:** seven heap-allocated lists manually deleted; copy operations are not disabled, creating shallow-copy/double-delete risk.
- **Status:** `partially-implemented`.
- **Recommended future owner:** AI + optimization expert; human decision on algorithm and product claims.

### Factorial design / Do Experiments

- **Observed backend:** `FactorialDesign.cpp` in `genesys_tools`.
- **Frontend:** `GENESYS_BUILD_GUI_DOEXPERIMENTS` exists but intentionally triggers `FATAL_ERROR` because the application is not implemented.
- **Tests:** no complete DOE matrix confirmed.
- **Status:** backend `partially-implemented`; GUI `not-started`.
- **Recommended future owner:** AI + DOE/statistics expert.

### AI assistant backend/provider

- **Observed targets/classes:** `genesys_ai_provider`, provider clients for OpenAI/Anthropic/local, conversation service, audit log, secret store.
- **Tests:** `genesys_test_ai_plugins` exists but is omitted from the inspected aggregate/runner.
- **Security finding:** secret is interpolated into a shell command string passed to `system()`.
- **Status:** `partially-implemented`, security `blocked` before broader deployment.
- **Recommended future owner:** AI + security review.

## Applications

### `source/applications/shell`

- **Purpose:** interactive/terminal GenESyS shell.
- **Target:** `genesys_shell`.
- **Build:** C++23 executable linked against kernel/parser/plugin runtime group.
- **Tests:** no dedicated current shell smoke test confirmed.
- **Documentation:** root/stable docs still use obsolete terminal names in places.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI.

### `source/applications/modelSpecific`

- **Purpose:** compile selected model-specific examples/smarts as dedicated executables.
- **Presets:** `terminal-smart`, `terminal-model-specific`, `genesys_modelspecific_app`, specialized variants.
- **Tests:** prior broad sweep documented but not rerun in this audit.
- **Risk:** selected source/class/header configuration and plugin branch drift.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI.

### `source/applications/worker`

- **Purpose:** HTTP/background worker, sessions, API routing, simulator session/job control.
- **Targets:** `genesys_worker_core`, `genesys_worker_app` (`genesys-worker`).
- **Tests:** conditional API router target.
- **Confirmed security risk:** access-token generation uses `std::mt19937_64`, which is not a CSPRNG.
- **Additional risks:** payload limits, timeouts, concurrency/lifecycle, token expiry/rotation, public bind/deployment posture.
- **Status:** `partially-validated`; public deployment `blocked` pending security review.
- **Recommended future owner:** AI + security/human deployment decision.

### `source/applications/gui`

- **Purpose:** umbrella for independent Qt graphical applications; it does not recursively collect all sibling application sources.
- **Options/subdirectories:**
  - `genesys`;
  - `httpworker`;
  - `dataanalyser`;
  - `optimizer`;
  - `ai_assistant`;
  - planned `doexperiments`.
- **Status:** structural separation `implemented` for first five targets; runtime/build validation incomplete.
- **Risk:** known GUI GMDD failures, main-GUI coupling tests, context handoff between processes.
- **Recommended future owner:** AI for build/tests; human UX/product review.

#### Main GenESyS GUI

- **Target/preset:** `genesys_gui` / `gui-app`.
- **Tests:** render strategy and GMDD layout tests.
- **Known state:** prior plan reports three GMDD failures; CI has a dedicated diagnostic job.
- **Status:** `blocked`.

#### HTTP Worker GUI

- **Target/preset:** `genesys_httpworker_gui_application` / `gui-httpworker`.
- **Status:** `needs-local-validation`.
- **Risk:** process/runtime control contract.

#### Data Analyser GUI

- **Target/preset:** `genesys_dataanalyser_gui_application` / `gui-dataanalyser`.
- **Status:** `needs-local-validation`.
- **Preferred extraction rationale:** file/dataset workflows can be tested without sharing live raw model pointers.

#### Optimizer GUI

- **Target/preset:** `genesys_optimizer_gui_application` / `gui-optimizer`.
- **Status:** frontend `needs-local-validation`; capability `partially-implemented` because backend is scaffold.
- **Risk:** misleading functionality and model/context lifetime.

#### AI Assistant GUI

- **Target/preset:** `genesys_ai_assistant_gui_application` / `gui-ai-assistant`.
- **Status:** `needs-local-validation`.
- **Risk:** secrets, provider configuration, model context, network calls.

#### Do Experiments GUI

- **Current state:** planned option only; enabling it produces an intentional configure error.
- **Status:** `not-started`.

## Tests

### `source/tests/unit`

- **Purpose:** GTest unit/integration-light executables discovered by CTest.
- **Aggregate targets:** `genesys_kernel_unit_tests`, `genesys_kernel_unit_tests_run`.
- **Confirmed issue:** AI plugin test is declared but omitted from aggregate dependencies/runner.
- **Other concerns:** repeated runtime link entries; GUI implementation recursively compiled into a test; generated inventory inflates count without behavioral validation.
- **Status:** `partially-validated` / aggregate `blocked` until corrected and rerun.
- **Recommended future owner:** AI.

### `source/tests/smoke`

- **Purpose:** simulator startup and selected continuous/LSODE smoke validation.
- **Target/preset:** `genesys_smoke_tests` / `tests-smoke`.
- **Status:** `needs-local-validation`.
- **Recommended future owner:** AI.

## CI, packaging, and release

### GitHub Actions

- **Unit workflow:** Ubuntu 24.04, root CMake presets, broad development dependencies.
- **Trigger:** pull requests to several branches plus manual dispatch; no push trigger.
- **Risk:** diagnostic GUI job can function as a permanent red check.
- **Status:** `needs-ci-validation` / diagnostic path `blocked`.
- **Recommended future owner:** AI implementation; human required-check policy.

### Debian/Docker/OVA/PPA-related assets

- **Purpose:** distribution and environment packaging.
- **Current evidence:** packaging/docker and historical/PR work exist; ordinary unit CI does not establish package correctness.
- **Tests:** install/startup/version/dependency/package-lint validation not audited in this round.
- **Status:** `needs-ci-validation`.
- **Recommended future owner:** AI + human release approval.

## Cross-cutting architectural status

| Concern | Current status | Evidence | Next safe action | Decision owner |
|---|---|---|---|---|
| Root CMake replaces `/projects` | implemented | root README/CMake | remove remaining stale references before eventual legacy deletion | AI + human deletion approval |
| C++23 baseline | implemented | root CMake/presets | verify actual GCC version and extension-free build | AI local validation |
| Qt6 baseline | partially-implemented | CI/GUI presets; tests still allow Qt5 fallback | decide and document Qt5 policy | human |
| Namespaces | not-started broadly | many global classes in inspected headers | map public/serialized/plugin symbols first | human architecture |
| Explicit ownership | partially-implemented | manual managers/containers and historical guidance | create subsystem ownership maps and sanitizer tests | AI + human review |
| Interface-based decoupling | partially-implemented | interfaces/factories exist, but concrete/static coupling remains | identify one bounded client/implementation seam at a time | AI + human review |
| Dynamic plugins | not-started for production build | static connector/static aggregate libraries | specify ABI/API and pilot one plugin | human architecture |
| Independent applications | partially-implemented | separate shell/worker/GUI targets | build/startup/context-handoff matrix | AI |
| Scientific correctness | blocked in legacy solver/statistics | inspected code and missing reference validation | tests first, then minimum fixes | domain experts + AI |
| Release readiness | not-started | known failures and unexecuted matrix | complete Phases 0–1 before release work | human release owner |
