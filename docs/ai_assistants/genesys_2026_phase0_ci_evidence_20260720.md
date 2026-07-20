# GenESyS 2026 Phase 0 CI Evidence — 2026-07-20

## 1. Purpose

Record the complete executed evidence for the three primary GenESyS Phase 0 presets and the bounded corrections integrated during consolidation.

This evidence establishes a reproducible software baseline. It does not establish release readiness, security readiness, package lifecycle correctness, application usability, or scientific validity.

## 2. Integrated checkpoint

- Repository: `rlcancian/Genesys-Simulator`.
- Branch: `WorkInProgress`.
- Consolidation documentation merge: `9c52b61532b847668adc3be92c780966301bcf7c`.
- Phase 0 workflow merge: `802b8aec7ac129559692bd574e70fd9991aaec1d`.
- Semester branch: `20261`, renamed from historical `2026-1`.
- Future stable branch: `20262`, reserved for end-of-semester promotion.

## 3. Ordinary baseline

PR #472 ordinary CI:

- workflow run: `29780136801`, run 121;
- tested merge commit: `0d5f32b48510f6c1776ccfb1572f51fb452a6538`;
- `tests-unit` configure: passed;
- `tests-unit` build: passed;
- `tests-unit` CTest: passed;
- focused GUI GMDD job: passed.

The three focused GUI tests remain green; older failing records are historical snapshots unless reproduced on a newer identified commit.

## 4. Kernel-focused baseline

Phase 0 workflow:

- run: `29780136722`, run 1;
- job: `Validate tests-kernel-unit`;
- configure: passed;
- build preset and direct runner: passed;
- CTest inventory: 1,696 registered tests;
- CTest execution: 1,692 passed, 0 failed, 4 disabled;
- total CTest time: 26.90 seconds.

Artifact:

- name: `genesys-phase0-tests-kernel-unit`;
- ID: `8476435822`;
- digest: `sha256:bf803a8432328e0bb7555d61c4c01d72e6cafe20391b57380099a8db440bc673`;
- expiration: 2026-10-18.

Toolchain captured:

- Ubuntu 24.04;
- CMake 3.31.6;
- Ninja 1.13.2;
- G++ 13.3.0.

AI plugin tests explicitly registered and passed:

- #495 `AIConversationServiceTest.KeepsIndependentBoundedHistories`;
- #496 `AIPluginTest.BuiltInConnectorExposesSupportAndComponentMetadata`;
- #497 `AIPluginTest.PromptTemplateEvaluatesExpressionsAndEscapesLiteralBraces`.

Disabled tests:

- #225 `SimulatorRuntimeTest.SearchQueueFindsEntityInRangeSavesRankAndRoutesToFoundPort`;
- #226 `SimulatorRuntimeTest.SearchQueueNotFoundRoutesToPortZeroAndSavesZeroRank`;
- #229 `SimulatorRuntimeTest.RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly`;
- #230 `SimulatorRuntimeTest.RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval`.

## 5. Smoke baseline

Phase 0 workflow job `Validate tests-smoke`:

- configure: passed;
- build: passed;
- CTest inventory: 3;
- executed/passed: 3/3;
- failed: 0;
- total time: 0.68 seconds.

Tests:

1. `smoke_simulator_start`;
2. `test_continuous_system`;
3. `test_lsode`.

Artifact:

- name: `genesys-phase0-tests-smoke`;
- ID: `8476325130`;
- digest: `sha256:3d0f40bb68d05ec0c738c8a34d92d2f4380cf743323015220c25706157871776`;
- expiration: 2026-10-18.

## 6. Integrated bounded corrections

PR #470, merge `5b24c1e4f31f1b001cd0bd6910fb2c134108fc77`:

- ordinary CI target branch corrected to `20261`;
- Debian workflow target branch corrected to `20261`;
- Debian path filter corrected to `debian/**`.

PR #471, merge `1fca8763cb7a6449cab719950ff74fb59e149b1e`:

- AI plugin tests added to ordinary aggregate;
- dedicated AI direct-runner target added to kernel runner graph;
- ordinary and kernel executions passed.

PR #472, merge `802b8aec7ac129559692bd574e70fd9991aaec1d`:

- added reusable `genesys-phase0-validation.yml`;
- captures kernel/smoke inventories, executions, versions, diagnostics, and artifacts.

## 7. Status justified by the evidence

| Concern | Status |
|---|---|
| `tests-unit` | `validated` for recorded commits |
| focused GUI GMDD | `validated` |
| `tests-kernel-unit` configure/build/direct runner/CTest | `validated` |
| `tests-smoke` configure/build/CTest | `validated` |
| ordinary and kernel AI plugin test inclusion | `validated` |
| exact kernel inventory | `validated`: 1,696 registered |
| exact smoke inventory | `validated`: 3 registered |
| core Phase 0 baseline | `validated` |
| four disabled runtime tests | explicit coverage gap |
| application/package/sanitizer/security/scientific validation | pending |

## 8. Validation not performed

- standalone shell, worker, main GUI, and independent GUI startup/workflows;
- representative model-specific application sweep;
- Debian package build/install/start/uninstall after trigger correction;
- ASan, LSan, UBSan, Valgrind, and profiling;
- worker networking/authentication/security controls;
- Qt5 fallback removal;
- dynamic plugin ABI/lifecycle implementation;
- authoritative numerical/statistical reference validation;
- biochemical/whole-cell benchmark validation.

## 9. Next safe technical step

Open a test-only PR for `SolverDefaultImpl1` that:

1. adds direct regression tests for each overload and precondition;
2. demonstrates the uninitialized-step/RK4/Simpson/unimplemented-path defects;
3. records analytical or independently generated expected values;
4. does not modify the implementation until the failures are isolated.

The disabled Search/Remove tests, Qt6-only cleanup, plugin target mapping, worker security, and application/package validation should remain separate bounded workstreams.