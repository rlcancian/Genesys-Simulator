# GenESyS 2026 Test Matrix

## Purpose

This matrix records the tests and validation paths confirmed in the `WorkInProgress` repository at commit `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`. It distinguishes repository inspection from executed validation.

Audit environment: ChatGPT Web with GitHub connector only. No local CMake/Ninja/CTest execution was available.

Status vocabulary follows `genesys_2026_consolidation_plan.md`.

## Baseline validation order

1. `tests-unit` configure/build/CTest.
2. `tests-kernel-unit` configure/build/CTest.
3. `tests-smoke` configure/build/CTest.
4. application presets independently.
5. focused numerical, persistence, ownership, and security tests.
6. sanitizer builds after a trustworthy ordinary baseline exists.

## Matrix

| Module / concern | Target or preset | Existing test evidence | Missing or insufficient coverage | Priority | Dependencies | Recommended command | Acceptance criterion | Status |
|---|---|---|---|---|---|---|---|---|
| Root unit baseline | configure/build/test preset `tests-unit`; target `genesys_kernel_unit_tests`; CTest label `unit` | many GTest executables discovered with `gtest_discover_tests` | local re-execution; exact current count; aggregate completeness | P0 | GCC, CMake 3.24+, Ninja, Python dev, Qt6, plugin runtime deps | `cmake --preset tests-unit && cmake --build --preset tests-unit --parallel "$(nproc)" && ctest --preset tests-unit --output-on-failure` | configure, build, and all required unit tests pass or failures are explicitly isolated/approved | needs-local-validation |
| Kernel-focused baseline | preset/target/test `tests-kernel-unit` | aggregate runner executes kernel/tool/plugin binaries | direct runner differs from ordinary CTest path; omissions possible | P0 | same as unit baseline | `cmake --preset tests-kernel-unit && cmake --build --preset tests-kernel-unit --parallel "$(nproc)" && ctest --preset tests-kernel-unit --output-on-failure` | target and CTest agree on included tests; all pass | needs-local-validation |
| Smoke baseline | preset/target/test `tests-smoke`; target `genesys_smoke_tests`; label `smoke` | simulator startup, continuous-system, LSODE smoke executables | smoke tests excluded from `tests-unit`; no shell/worker/GUI startup in this target | P1 | runtime/plugin libraries | `cmake --preset tests-smoke && cmake --build --preset tests-smoke --parallel "$(nproc)" && ctest --preset tests-smoke --output-on-failure` | all registered smoke tests execute and pass | needs-local-validation |
| Utilities | `genesys_test_util` | unit executable registered | edge cases for legacy containers, ownership, invalid indices | P2 | `genesys_kernel_util` | CTest filter for util tests | deterministic behavior and no sanitizer findings | partially-validated |
| Simulator support | `genesys_test_simulator_support` | support unit executable | destruction-order, exception-path, and ownership stress tests | P1 | simulator support/util | CTest filter for simulator support | lifecycle tests pass under ASan/LSan | partially-validated |
| Simulator runtime | `genesys_test_simulator_runtime` | runtime unit executable | repeated create/destroy, plugin completion leak, runtime failure paths | P0 | runtime, tools, plugin minimal | CTest filter plus ASan/LSan | repeated construction/destruction has no leak/UB | partially-validated |
| ModelManager | `genesys_test_support_modelmanager` | focused unit executable | copy/replacement/destruction and multi-model ownership | P1 | simulator support | focused CTest filter | ownership contract tests pass | partially-validated |
| Persistence | `genesys_test_support_persistence` | focused unit executable | cross-plugin round trip; backward compatibility fixtures | P1 | simulator support | focused CTest filter and fixture models | save/load round trip preserves supported fields and reports unsupported data | partially-validated |
| PluginManager | `genesys_test_runtime_pluginmanager` | focused unit executable | connector ownership injection, duplicate insertion, load/unload, ABI/version mismatch | P1 | runtime, plugin libraries | focused CTest filter | deterministic lifecycle and diagnostics; no leak/double delete | partially-validated |
| TraceManager | `genesys_test_support_tracemanager` | focused unit executable | handler lifetime, teardown order, concurrent use if supported | P1 | simulator support | focused CTest filter + sanitizer | no dangling handlers; teardown safe | partially-validated |
| SimulationScenario | `genesys_test_support_simulationscenario` | focused unit executable | copy/replacement/destruction semantics and invalid scenarios | P1 | simulator support | focused CTest filter | copy/replace/destroy contract documented and tested | partially-validated |
| ExperimentManager | `genesys_test_support_experimentmanager` | focused unit executable | DOE/tool integration and failure handling | P2 | simulator support | focused CTest filter | nominal/error paths pass | partially-validated |
| Parser expressions | `genesys_test_parser_expressions` | expression unit executable | syntax diagnostics, unknown functions, backward-compatible `.gen` fixtures | P1 | parser/runtime/plugin minimal | focused CTest filter | nominal/error/model compatibility suite passes | partially-validated |
| Parser FunctionRegistry | no current target confirmed | historical PR #429 described registry/resolver tests | registry source/test files absent at stated paths in current branch | P1 | parser, plugin metadata/runtime | after design decision, add dedicated target and compatibility fixtures | supported functions resolve; unknown functions fail deterministically; old models remain compatible | not-started |
| Generated method inventory | `genesys_test_kernel_simulator_method_inventory` | generated GTest inventory from headers | generated stubs do not establish behavioral correctness | P2 | Python generator | build target and inspect generated tests | inventory generation reproducible; behavioral gaps tracked separately | partially-validated |
| Kernel statistics | `genesys_test_statistics` | statistics unit executable | trusted reference comparisons, degenerate/non-finite samples | P0/P1 | kernel statistics | focused CTest filter | results agree with defined reference tolerances | partially-validated |
| Hypothesis testing | `genesys_test_tools_hypothesistester` | direct target using statistical sources and legacy solver | p-values depend on flawed integrator; small-sample/empty-data policies incomplete | P0 | tools/statistics/legacy solver | focused test binary; add R/Python/Boost reference vectors | confidence intervals/tests match references and reject invalid domains | blocked |
| Legacy quadrature/derivative solver | no dedicated target confirmed | source indirectly linked into tools/hypothesis tests | all overloads, zero steps, even Simpson intervals, `_stepSize`, RK4 stages, VLA removal | P0 | none beyond standard library | create focused GTest target before patch | tests demonstrate old defects, then pass after minimum correction | blocked |
| ODE solver factory | `genesys_test_tools_ode_solver_factory` | RK4 and Dormand-Prince analytical, convergence, tolerance, and invalid-input tests | long-time stability, stiffness, event-boundary integration, NaN/Inf policy | P1 | `genesys_tools` | focused CTest filter | analytical/convergence tests pass; contract documented | partially-validated |
| Diffusion Method of Lines | `genesys_test_tools_diffusion_mol` | DCS-labeled core tests | conservation/boundary/mesh convergence and 2D fixtures need confirmation | P1 | tools | focused CTest filter | known-solution/conservation tolerances pass | partially-validated |
| Continuous diffusion integration | `genesys_test_plugins_continuous_diffusion` | real kernel/plugin/event-clock integration target | links full plugin library while runtime links minimal; hybrid time contract unresolved | P0/P1 | full+minimal plugin graphs, runtime, tools | focused build/CTest plus link-map inspection | no duplicate-symbol/ODR problem; event-time semantics tested | blocked |
| Continuous/LSODE smoke | `genesys_test_continuous_system`, `genesys_test_lsode` | smoke targets | deterministic numerical oracle and failure diagnostics | P1 | continuous plugins/runtime | `ctest --preset tests-smoke -R 'continuous|lsode'` | expected trajectory/tolerance and errors are explicit | needs-local-validation |
| Cellular automata neighborhood | `genesys_test_cellular_automata_neighborhood` | target present | other historical #453 test files absent from current branch | P1 | plugin minimal | focused CTest filter | neighborhood cases pass; full CA test inventory reconciled | partially-validated |
| Cellular automata full/user-defined/persistence | no complete current target set confirmed | historical PR #453 claimed 50 passing tests | exact files/targets missing or divergent; external compiler/temp-dir behavior | P1 | C++ compiler, `dlopen`, writable temp dir | restore/reconcile targets only after file inventory | boundaries, policies, rules, persistence, failure paths pass | needs-local-validation |
| Whole-cell plugins | `genesys_test_wcm_plugins` | large focused test source with propensity/state/component tests | prior plan reports runtime/metabolic failures; GLPK/fallback, persistence, stochastic reproducibility | P0/P1 | runtime, plugin minimal, optional GLPK | focused CTest filters; run with and without GLPK | deterministic fixtures pass in both supported dependency modes | blocked |
| Biochemical/SBML | no dedicated complete matrix confirmed | plugin/docs and libSBML CI dependency | import/export round trip, unsupported constructs, diagnostics, native-model preservation | P1 | libSBML, biochemical plugins | add small SBML fixture suite | no silent data loss; support matrix enforced | not-started |
| ModalModel/EFSM/Petri | no complete dedicated target confirmed | domain guide and source tree | state transitions, token semantics, persistence, submodel lifecycle | P1 | modal plugin/runtime | add minimal model fixtures | deterministic execution and round trip pass | not-started |
| Hybrid synchronization | no dedicated contract target | current diffusion integration touches event clock | fixed solver step vs simulated elapsed time at event boundaries | P0/P1 | runtime event calendar + continuous plugins | create analytical hybrid fixture | state at each event boundary matches defined contract | not-started |
| AI provider clients | provider library compiled | provider source exists | transport parsing, timeout, redaction, no-network fakes | P1 | HTTP/process layer | fake provider unit targets | no real external calls; errors/redaction deterministic | needs-local-validation |
| AI plugins | `genesys_test_ai_plugins` declared | GTest target exists | omitted from `genesys_kernel_unit_tests` dependencies and runner | P1 | provider/runtime/plugins | add to aggregate, then CTest filter | built and executed in default unit CI | blocked |
| AI secret storage | no dedicated target confirmed | `AISecretStore` source | secret exposure in shell command, escaping/failure/keyring availability | P1 | `secret-tool` or replacement API | add process/fake boundary tests | secret never appears in argv/logs; failure safe | not-started |
| Worker API router | `genesys_test_worker_api_router` when worker enabled | included conditionally in aggregate | malformed payload, concurrency, auth, limits, timeouts, lifecycle | P1 | worker core/runtime | focused CTest filter | route/error/auth contracts pass | partially-validated |
| Worker token service | no dedicated target confirmed | token generator source | CSPRNG requirement, collision, entropy, expiry/rotation/storage | P1 | OS random source | add dedicated unit target | cryptographic source used; format/length validated | not-started |
| Optimizer backend | no focused target confirmed in inspected aggregate | scaffold source exists | no candidate generation/evaluation/ranking; copy/double-delete risk | P1 | model controls/responses/parser | first add contract/ownership tests, then benchmark functions | real algorithm meets deterministic acceptance criteria | partially-implemented |
| Data Analyser backend | `genesys_test_tools_simulation_results_dataset`; hypothesis/statistics tests | dataset target present | independent data-analysis package status, fitting references, import/export | P1 | tools/statistics | focused tests + trusted datasets | statistics/fits and metadata round trip validated | partially-implemented |
| FactorialDesign / Do Experiments | no GUI executable; root tools compiles `FactorialDesign.cpp` | backend source included | design generation, alias structure, replication, ANOVA/RSM; GUI workflow absent | P1/P2 | tools/statistics | add pure backend tests before GUI | known 2^k/fractional designs and analyses match references | partially-implemented |
| Shell application | preset `genesys_shell` | executable target | startup, command parsing, minimal model run, clean exit | P1 | kernel/parser/plugins | `cmake --preset genesys_shell && cmake --build --preset genesys_shell` plus scripted smoke | starts, processes minimal command/model, exits 0 | needs-local-validation |
| Model-specific applications | presets `terminal-smart`, `terminal-model-specific`, `genesys_modelspecific_app` | prior plans record broad sweeps | current branch revalidation, remaining runtime failures | P1 | selected source/plugin graph | build representative examples and run deterministic models | representative set builds/runs with expected outputs | needs-local-validation |
| Main GUI | `gui-app` target `genesys_gui` | render strategy and GMDD tests | three known GMDD failures; startup/model interaction | P0 | Qt6, worker/tools/runtime | `cmake --preset gui-app && cmake --build --preset gui-app`; offscreen tests | configure/build/startup pass; GMDD tests green | blocked |
| HTTP worker GUI | `gui-httpworker` | independent target | startup/process handoff/runtime control | P1 | Qt6 + worker | build preset and offscreen smoke | starts independently and controls worker through defined contract | needs-local-validation |
| Data Analyser GUI | `gui-dataanalyser` | independent target | startup/import/basic analysis; no live pointer sharing | P1 | Qt6 + tools | build preset and offscreen smoke | starts and completes small dataset workflow | needs-local-validation |
| Optimizer GUI | `gui-optimizer` | independent target | backend is scaffold; model/context handoff | P1 | Qt6 + incomplete optimizer | build only; capability must remain experimental | startup succeeds and UI does not claim unsupported optimization | needs-local-validation |
| AI Assistant GUI | `gui-ai-assistant` | independent target | provider configuration, secrets, model context, offline fake tests | P1 | Qt6 + provider/tools | build preset and fake-provider smoke | no real API required; secrets not exposed | needs-local-validation |
| Do Experiments GUI | enabling option triggers `FATAL_ERROR` | explicit planned option | complete implementation/workflow absent | P2 | FactorialDesign backend + Qt6 | not applicable until specification approved | option/target introduced only with backend tests and smoke path | not-started |
| CI required baseline | workflow `unit-tests` | Ubuntu 24.04 `tests-unit` job | current execution on audit PR needed | P0 | GitHub Actions dependencies | open PR and inspect run | required unit job green or failures reproducibly documented | needs-ci-validation |
| GUI GMDD diagnostic CI | workflow `gui-gmdd-diagnostics` | explicitly executes three filters | intentionally failing diagnostic can block PR | P0 | Qt6/offscreen | run on PR; decide required vs non-blocking | diagnostic preserves artifact without making required baseline permanently red, or tests fixed | blocked |
| Sanitizers | no current presets confirmed | README contains stale commands | ASan/LSan/UBSan CMake presets and runtime dependency compatibility | P1 | GCC/Clang, test baseline | add opt-in presets only after ordinary baseline | focused ownership/numerical suites pass; no sanitizer findings | not-started |
| Packaging | separate workflows/scripts | packaging docs/assets exist | package install/runtime tests not in ordinary CI | P2 | Debian/Docker dependencies | use packaging-specific workflow | generated artifacts install, start, and report versions correctly | needs-ci-validation |

## Known aggregation defects to verify locally

1. `genesys_test_ai_plugins` is declared but not listed in the inspected `genesys_kernel_unit_tests` dependencies or direct runner.
2. Several targets link `genesys_kernel_simulator_runtime` more than once.
3. `genesys_test_plugins_continuous_diffusion` links `genesys_plugins_components`, while the runtime already links `genesys_plugins_components_minimal` built from the same recursive source tree.
4. GUI test source collection recursively compiles the main GUI implementation into a test executable, increasing coupling and build cost.
5. The generated method inventory increases test count but must not be interpreted as behavioral coverage.

## Numerical reference policy

For correctness-sensitive algorithms, tests must define:

- mathematical assumptions and input domain;
- expected value/reference implementation;
- absolute and relative tolerances;
- deterministic seeds where randomness is involved;
- behavior for empty, small, degenerate, repeated, and non-finite data;
- dependency mode, such as GLPK present/absent;
- distinction between exact, asymptotic, approximate, and heuristic results.

Recommended external references are for test-vector generation/verification, not necessarily runtime dependencies. Suitable references include analytical solutions, R, Python/SciPy, Boost.Math, published examples, and independently calculated fixtures.

## Required evidence format for future runs

Record:

- branch and commit;
- OS/compiler/CMake/Ninja/Qt versions;
- configure/build/test preset;
- test count and exact failing names;
- first compiler/link/runtime failure;
- whether failure is environment, dependency, code, numerical tolerance, or test defect;
- artifact/log location;
- status change in this matrix and the handoff.
