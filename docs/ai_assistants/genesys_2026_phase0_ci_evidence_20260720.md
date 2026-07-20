# GenESyS 2026 Phase 0 CI Evidence — 2026-07-20

## 1. Purpose

Record the first executed validation evidence for the GenESyS 2026 consolidation plan. This checkpoint supersedes earlier documentation statements that treated the three GUI GMDD tests as currently failing or treated the ordinary pull-request CI state as unknown.

This document does not establish release readiness. It records only the scope actually executed by the referenced GitHub Actions run.

## 2. Repository and branch identity

- Repository: `rlcancian/Genesys-Simulator`.
- Pull request: #469, `docs: add GenESyS 2026 consolidation plan`.
- Pull-request base: `WorkInProgress`.
- Pull-request head branch: `audit/genesys-2026-consolidation-plan-20260720`.
- Validated head commit: `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`.
- Pull-request merge commit validated by CI: `67d895f67307a6ba614e0c4cc5be88fbf24390ab`.
- Workflow run ID: `29771060564`.
- Workflow run number: `107`.
- Workflow conclusion: `success`.

## 3. Semester branch correction verified

The repository ref `20261` resolves successfully. It is the current name of the branch previously called `2026-1`.

At this checkpoint:

- `20261` base commit: `188f1ec7c3fe1f54c7ed4637395403ac8ee1ff5f`;
- `WorkInProgress` is 29 commits ahead of `20261`;
- `WorkInProgress` is 0 commits behind `20261`.

This comparison is a point-in-time ancestry result. It does not authorize promotion back to `20261` and does not change the end-of-semester policy for future `20262` promotion.

## 4. Execution environment confirmed from CI

- GitHub-hosted runner image: `ubuntu-24.04`.
- Operating system reported by the runner: Ubuntu 24.04.4 LTS.
- Qt platform mode: `QT_QPA_PLATFORM=offscreen`.
- CMake configure preset: `tests-unit`.
- CMake build preset: `tests-unit`.
- CTest preset: `tests-unit`.
- Failure output policy: `CTEST_OUTPUT_ON_FAILURE=1`.

The workflow installed its declared build dependencies before configuration. Exact compiler, CMake, Ninja, Qt, and dependency version strings were not extracted into this checkpoint because the GitHub connector exposed the complete job as a successful log but did not provide a convenient bounded tail/query interface for all version lines.

## 5. Ordinary unit baseline result

Job: `Configure, build and test tests-unit`.

Result:

- checkout: success;
- dependency installation: success;
- CMake configure with `tests-unit`: success;
- build with `tests-unit`: success;
- CTest with `tests-unit`: success;
- failure diagnostics step: skipped because no failure occurred;
- job conclusion: `success`.

Interpretation:

- every test actually registered and executed by the current `tests-unit` CTest preset passed on the validated PR merge commit;
- the previously documented historical result of 16 failures out of 1657 tests was not reproduced by this run;
- the exact current CTest count was not captured through the connector and must not be inferred from the historical 1657 count;
- a green `tests-unit` run does not prove that every declared test executable is included in the aggregate. The known `genesys_test_ai_plugins` aggregation concern remains open until target/CTest inclusion is explicitly verified.

## 6. GUI GMDD diagnostic result

Job: `Diagnose GUI GMDD tests`.

Artifact:

- artifact name: `genesys-gui-gmdd-diagnostics`;
- artifact ID: `8472919224`;
- artifact digest: `sha256:3b7b967f78901f8fbb9fff1dbdb00281cca02cc150c578afcdadb46f0c6f9389`;
- artifact expiration: 2026-10-18.

The artifact reports:

- `CONFIGURE_RC=0`;
- `BUILD_RC=0`;
- `GTEST_RC=0`;
- executable: `build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout`.

The following three tests passed:

1. `GuiGmddLayout.SeizeEditableReferencesStayAboveAndLowerDefinitionsUseTwoRows`;
2. `GuiGmddLayout.SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition`;
3. `GuiGmddLayout.SerializerRoundTripRestoresViewStateGeometriesAndGroups`.

Execution summary from the artifact:

- 3 tests from 1 test suite;
- all 3 passed;
- total reported Google Test execution time: 542 ms.

Interpretation:

- these three tests are not current blockers on the validated PR head;
- older documents describing them as failing are historical snapshots and must not be used as current status without a newer failing run;
- this focused result does not replace a standalone `gui-app` configure/build/startup validation.

## 7. Status changes justified by this evidence

| Concern | Previous planning status | Status after this checkpoint | Rationale |
|---|---|---|---|
| Pull-request `tests-unit` CI | `needs-ci-validation` / unknown | `validated` for head `a60ca65...` | configure, build, and CTest job passed |
| Three GUI GMDD tests | historical `blocked` | `validated` for head `a60ca65...` | focused executable built and all three tests passed |
| Phase 0 ordinary CI evidence | `in-progress` with no execution | `partially-validated` | one of three baseline presets executed successfully |
| Historical 16/1657 failure list | treated as unresolved historical evidence | `not-reproduced-in-current-tests-unit` | current `tests-unit` run passed; exact old environment/commit differed |
| Full baseline | incomplete | `needs-local-validation` or additional CI | `tests-kernel-unit` and `tests-smoke` were not executed by this workflow |

## 8. Validation not performed by this run

The following remain unvalidated by workflow run `29771060564`:

- configure/build/CTest for `tests-kernel-unit`;
- configure/build/CTest for `tests-smoke`;
- standalone build/startup for `genesys_shell`;
- standalone worker build/startup and network behavior;
- standalone main GUI `gui-app` build/startup beyond the GMDD unit-test executable;
- `gui-httpworker`, `gui-dataanalyser`, `gui-optimizer`, and `gui-ai-assistant` presets;
- model-specific application sweep;
- sanitizer execution;
- Valgrind execution;
- Debian package build/install/start/uninstall validation;
- Qt5 removal;
- secure worker authentication;
- numerical/statistical scientific correctness;
- whole-cell/biochemical benchmark validity;
- future dynamic plugin ABI implementation.

## 9. Remaining Phase 0 actions

1. Execute `tests-kernel-unit` on the same supported Ubuntu/Qt6 baseline.
2. Execute `tests-smoke` on the same supported Ubuntu/Qt6 baseline.
3. Record exact test counts and target inclusion using `ctest -N`, build target inspection, and CTest logs.
4. Verify whether `genesys_test_ai_plugins` is built and executed by the ordinary baseline.
5. Confirm that historical WCM/runtime/metabolic failures are absent, renamed, no longer registered, or fixed; do not infer the cause solely from the green aggregate result.
6. Build representative application presets independently.
7. Preserve this successful run as the current CI checkpoint until superseded by newer evidence.

## 10. Next safe technical step

After the two remaining baseline presets are executed, the next bounded code task should be one of:

- repair test aggregation so every intended unit target executes;
- add failing regression tests for `SolverDefaultImpl1` before modifying it;
- generate the static plugin source-to-target/link map.

No broad plugin, namespace, ownership, optimizer, or scientific-model refactoring is authorized by this CI result.
