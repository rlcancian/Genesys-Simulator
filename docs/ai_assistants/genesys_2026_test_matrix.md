# GenESyS 2026 Test Matrix

## 1. Purpose

Record the validation paths, current executed evidence, missing coverage, and acceptance criteria for the GenESyS 2026 consolidation.

This matrix distinguishes:

- source/build/test structure confirmed by repository inspection;
- remote execution confirmed by GitHub Actions;
- validation that still requires local or additional CI execution;
- scientific correctness that cannot be inferred from ordinary unit-test success.

Primary base originally inspected:

- branch: `WorkInProgress`;
- commit: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.

Latest executed checkpoint:

- pull request: #469;
- head: `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`;
- PR merge commit: `67d895f67307a6ba614e0c4cc5be88fbf24390ab`;
- GitHub Actions run: `29771060564`;
- result: success.

See `genesys_2026_phase0_ci_evidence_20260720.md` for the complete bounded interpretation.

## 2. Status vocabulary

Use only:

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

A status of `validated` applies only to the explicitly identified commit/environment/scope.

## 3. Baseline validation order

1. `tests-unit` configure/build/CTest.
2. `tests-kernel-unit` configure/build/CTest.
3. `tests-smoke` configure/build/CTest.
4. explicit test-target/CTest aggregation inventory.
5. application presets independently.
6. focused numerical, persistence, ownership, security, and scientific tests.
7. sanitizers and package validation after the ordinary baseline is trustworthy.

## 4. Current Phase 0 execution summary

| Validation path | Current evidence | Current status | Remaining gap |
|---|---|---|---|
| `tests-unit` configure/build/CTest | workflow run `29771060564` passed | `validated` | exact test count and aggregate completeness not captured |
| three GUI GMDD tests | diagnostic artifact ID `8472919224`; 3/3 passed | `validated` | standalone GUI startup not covered |
| `tests-kernel-unit` | preset confirmed, not executed in current workflow | `needs-local-validation` | configure/build/CTest result |
| `tests-smoke` | preset confirmed, not executed in current workflow | `needs-local-validation` | configure/build/CTest result |
| application preset matrix | presets confirmed, not executed in current workflow | `needs-local-validation` | independent build/startup paths |
| sanitizers | no current executed preset | `not-started` | ASan/LSan/UBSan/Valgrind plan |
| Debian package lifecycle | separate packaging work exists | `needs-ci-validation` | build/install/start/uninstall evidence |

## 5. Detailed validation matrix

| Module / concern | Target or preset | Current evidence | Missing or insufficient coverage | Priority | Acceptance criterion | Status |
|---|---|---|---|---|---|---|
| Root unit baseline | `tests-unit`; CTest unit registration | configure, build, and CTest passed on head `a60ca65...` | exact count; omitted target detection | P0 | all intended targets built and all registered required tests pass | `validated` for executed scope |
| Kernel-focused baseline | `tests-kernel-unit` | preset/aggregate confirmed by inspection | no current execution | P0 | target and CTest agree on included tests; all pass | `needs-local-validation` |
| Smoke baseline | `tests-smoke`; `genesys_smoke_tests` | simulator/continuous/LSODE smoke sources confirmed | no current execution; no shell/worker/GUI startup | P1 | all registered smoke tests execute and pass | `needs-local-validation` |
| Test aggregation | `genesys_kernel_unit_tests`, direct runner, CTest discovery | broad suite passes through current preset | declared executable inclusion not independently enumerated | P0/P1 | `ctest -N` and target graph show every intended test, including AI plugins | `partially-validated` |
| Utilities | `genesys_test_util` | included in broad unit structure | legacy container edge cases; sanitizers | P2 | deterministic edge-case behavior; no sanitizer findings | `partially-validated` |
| Simulator support | `genesys_test_simulator_support` | broad unit baseline green | destruction order, exception paths, ownership stress | P1 | lifecycle tests pass under ordinary and sanitizer builds | `partially-validated` |
| Simulator runtime | `genesys_test_simulator_runtime` | broad unit baseline green | repeated create/destroy, leak/UB paths | P0/P1 | repeated runtime lifecycle has no leak/UB | `partially-validated` |
| ModelManager | focused support target | focused target confirmed | replacement/copy/multi-model ownership | P1 | ownership contract documented and tested | `partially-validated` |
| Persistence | focused support target | broad baseline green | cross-plugin round trip; backward compatibility fixtures | P1 | supported fields round-trip; unsupported data diagnosed | `partially-validated` |
| PluginManager | focused runtime target | broad baseline green | connector injection ownership; duplicate insertion; future ABI mismatch | P1 | deterministic lifecycle and diagnostics; no leak/double delete | `partially-validated` |
| TraceManager | focused support target | broad baseline green | handler teardown and concurrency if supported | P1 | no dangling handlers; safe teardown | `partially-validated` |
| SimulationScenario | focused support target | broad baseline green | copy/replacement/destruction and invalid scenarios | P1 | copy/replace/destroy semantics explicit and tested | `partially-validated` |
| ExperimentManager | focused support target | broad baseline green | DOE/tool integration and failures | P2 | nominal and error paths pass | `partially-validated` |
| Parser expressions | `genesys_test_parser_expressions` | broad baseline green | unknown-function semantics and compatibility fixtures | P1 | deterministic errors and `.gen` compatibility | `partially-validated` |
| Parser FunctionRegistry | no current dedicated target confirmed | historical PR #429 only | current design/disposition unknown | P1 | explicit registry decision plus tests | `not-started` |
| Generated method inventory | generated test target | generator/target confirmed | generated stubs are not behavioral coverage | P2 | inventory reproducible; gaps tracked separately | `partially-validated` |
| Kernel statistics | `genesys_test_statistics` | broad unit baseline green | trusted numerical references; degenerate/non-finite cases | P0/P1 | results agree with declared reference/tolerances | `partially-validated` |
| Hypothesis testing | `genesys_test_tools_hypothesistester` | current registered baseline did not fail | p-values depend on suspect legacy integrator; reference vectors absent | P0 | exact methods match authoritative references and reject invalid domains | `blocked` for scientific validity |
| Legacy quadrature/derivative solver | no dedicated focused target confirmed | source inspection identified defects | direct regression tests for every overload/precondition | P0 | tests first demonstrate defects, then pass after minimum correction | `blocked` |
| ODE solver factory | focused ODE target | analytical/convergence tests exist; aggregate green | stiffness, long-time stability, NaN/Inf, event boundaries | P1 | documented solver contract and analytical/convergence pass | `partially-validated` |
| Diffusion Method of Lines | focused diffusion target | DCS tests exist; aggregate green | conservation, boundary, mesh convergence, 2D fixtures | P1 | known-solution and conservation tolerances pass | `partially-validated` |
| Continuous/plugin integration | focused integration target | current aggregate did not fail | full/minimal target overlap; hybrid time contract | P0/P1 | no ODR/link overlap and event-time contract tested | `blocked` architecturally |
| Continuous/LSODE smoke | smoke targets | targets confirmed | current smoke preset not executed | P1 | expected trajectory/tolerance and diagnostics explicit | `needs-local-validation` |
| Cellular automata neighborhood | focused target | target confirmed; broad baseline green | historical full CA test inventory divergence | P1 | current tests pass and historical files are reconciled | `partially-validated` |
| Cellular automata full/user-defined | incomplete current target inventory | historical PR evidence | external compiler, persistence, policy, failure paths | P1 | full current target/file inventory and passing behavior suite | `needs-local-validation` |
| Whole-cell plugins | `genesys_test_wcm_plugins` | no failure surfaced in current `tests-unit` run | explicit target inclusion, with/without GLPK, persistence, stochastic reproducibility | P0/P1 | deterministic supported fixtures pass in all declared dependency modes | `partially-validated` |
| Biochemical/SBML | no complete dedicated matrix confirmed | source/docs and CI dependency exist | construct support matrix; round trip; diagnostics | P1 | no silent data loss; supported/unsupported constructs enforced | `not-started` |
| ModalModel/EFSM/Petri | no complete target confirmed | domain source/docs exist | transition/token/persistence/submodel fixtures | P1 | deterministic execution and round trip | `not-started` |
| Hybrid synchronization | no dedicated contract target | integration touches event clock | fixed-step/event-boundary semantics | P0/P1 | analytical hybrid fixture matches defined time contract | `not-started` |
| AI provider clients | provider target compiled | source/target structure confirmed | fake transport, timeout, parsing, redaction | P1 | no real network required; deterministic errors/redaction | `needs-local-validation` |
| AI plugins | `genesys_test_ai_plugins` | target declared | inspected aggregate/direct runner omission | P1 | executable built, registered, and executed in ordinary CI | `blocked` |
| AI secret storage | no dedicated target confirmed | source inspection identified shell-command exposure | fake/process boundary and failure tests | P1 | secret absent from argv/logs; safe failure | `not-started` |
| Worker API router | conditional focused target | target structure confirmed | malformed input, auth, quotas, timeouts, lifecycle | P1 | route/error/auth contracts pass | `partially-validated` |
| Worker token service | no dedicated target confirmed | source inspection found `mt19937_64` | CSPRNG, entropy, expiry, rotation, storage | P1 | OS CSPRNG and validated token lifecycle | `not-started` |
| Optimizer backend | no complete focused behavior suite | scaffold source exists | candidate generation/evaluation/ranking, ownership, stochastic replication | P1 | Level 3 contract and benchmark suite pass | `partially-implemented` |
| Data Analyser backend | dataset/statistics targets | related targets exist; aggregate green | fitting references, import/export, independent package scope | P1 | reference-backed statistics and metadata round trip | `partially-implemented` |
| FactorialDesign / Do Experiments | backend source included; GUI absent | scaffold/planning evidence | design generation, aliasing, ANOVA/RSM, workflow | P1/P2 | declared designs/analyses match references; Level 3 workflow | `partially-implemented` |
| Shell application | `genesys_shell` | preset/target confirmed | scripted startup/model/exit smoke | P1 | starts, processes minimal model/command, exits cleanly | `needs-local-validation` |
| Model-specific applications | model-specific presets | historical sweeps exist | current representative execution | P1 | representative apps build/run with expected outputs | `needs-local-validation` |
| Main GUI | `gui-app`; GMDD unit executable | 3 focused GMDD tests passed in current artifact | standalone configure/build/startup/model interaction | P0/P1 | target builds/starts and focused GUI tests remain green | `partially-validated` |
| HTTP worker GUI | `gui-httpworker` | independent target confirmed | startup and process/runtime handoff | P1 | starts independently and controls worker via defined contract | `needs-local-validation` |
| Data Analyser GUI | `gui-dataanalyser` | independent target confirmed | startup/import/basic analysis | P1 | completes a small dataset workflow | `needs-local-validation` |
| Optimizer GUI | `gui-optimizer` | independent target confirmed | backend below Level 3; context handoff | P1 | startup succeeds; no unsupported maturity claim | `needs-local-validation` |
| AI Assistant GUI | `gui-ai-assistant` | independent target confirmed | fake provider, secrets, model context | P1 | offline startup/workflow; no secret exposure | `needs-local-validation` |
| Do Experiments GUI | option intentionally fails if enabled | planned only | Level 3 backend/workflow absent | P2 | target introduced only after tested specification | `not-started` |
| Required CI baseline | GitHub Actions `tests-unit` job | current run passed | missing kernel/smoke matrix and exact test inventory | P0 | required CI contract intentionally covers declared baseline | `partially-validated` |
| GUI GMDD diagnostic CI | focused three-test job | current run and artifact passed | determine whether separate job remains useful | P1 | stays green or becomes non-blocking diagnostic by explicit policy | `validated` for current head |
| Sanitizers | no current validated presets | stale README commands previously identified | presets and runtime compatibility | P1 | focused high-risk suites pass without findings | `not-started` |
| Packaging | separate workflow/scripts | packaging assets and prior artifacts exist | current package lifecycle not part of this run | P2 | build/install/start/uninstall/version checks pass | `needs-ci-validation` |

## 6. Current aggregation concerns

1. `genesys_test_ai_plugins` is declared but was not found in the inspected `genesys_kernel_unit_tests` dependencies/direct runner.
2. Several test targets repeat `genesys_kernel_simulator_runtime` in link lists.
3. `genesys_test_plugins_continuous_diffusion` can combine the full plugin component library with a runtime already linked to the minimal library built from the same recursive source tree.
4. GUI test source collection compiles substantial GUI implementation into a unit executable, increasing coupling/build cost.
5. Generated method inventory increases test count but does not establish behavioral correctness.
6. A green CTest run must not be interpreted as proof that omitted executables were run.

## 7. Numerical and scientific reference policy

For each correctness-sensitive algorithm, record:

- mathematical definition and parameterization;
- valid domain and preconditions;
- units and scaling;
- analytical invariant or closed-form oracle when available;
- authoritative bibliographic reference;
- independent comparator and version when used;
- absolute and relative tolerances with rationale;
- behavior for empty, small, degenerate, repeated, and non-finite inputs;
- deterministic seeds and stochastic replication policy;
- optional dependency mode, such as GLPK present/absent;
- distinction among exact, asymptotic, approximate, heuristic, and experimental results.

Acquisition of bibliography, PDFs, converted text, datasets, parameterizations, and expected values is deferred and tracked in `genesys_numerical_statistical_references_plan.md`.

## 8. Required evidence format for future runs

Record:

- branch and commit;
- PR merge commit when applicable;
- workflow/run/job identifiers;
- OS/compiler/CMake/Ninja/Qt versions;
- configure/build/test preset;
- `ctest -N` count and exact executed test count;
- exact failing names and first failure;
- target/CTest inclusion evidence;
- dependency modes;
- artifact/log location and digest where available;
- classification: environment, dependency, build, code, numerical tolerance, scientific reference, or test defect;
- resulting status changes in this matrix and the operational handoff.

## 9. Next execution checkpoint

The next bounded validation run must execute, without functional changes:

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure

cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

It should additionally capture:

```bash
ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Do not begin broad functional refactoring merely because the current `tests-unit` checkpoint is green.
