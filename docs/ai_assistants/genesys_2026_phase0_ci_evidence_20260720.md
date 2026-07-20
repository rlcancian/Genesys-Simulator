# GenESyS 2026 Phase 0 CI Evidence — 2026-07-20

## 1. Purpose

Record the executed Phase 0 evidence available through GitHub Actions and the first two bounded corrections integrated into `WorkInProgress`.

This document supersedes older statements that treated the three GUI GMDD tests as currently failing, the ordinary pull-request CI as unknown, or the AI plugin test omission as unresolved.

It does not establish release readiness or scientific correctness.

## 2. Repository and branch identity

- Repository: `rlcancian/Genesys-Simulator`.
- Consolidation PR: #469.
- Consolidation branch: `audit/genesys-2026-consolidation-plan-20260720`.
- Base branch: `WorkInProgress`.
- Original inspected base: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.
- Semester branch: `20261`, renamed from historical `2026-1`.
- Future stable branch: `20262`, intended for end-of-semester promotion only.

## 3. Executed checkpoints

### 3.1 Consolidation documentation

- PR: #469.
- Workflow run: `29772903692`, run 115.
- Tested merge commit: `bf8bfd93a2b002c2098811cc339900eef516942d`.
- Configure preset `tests-unit`: passed.
- Build preset `tests-unit`: passed.
- CTest preset `tests-unit`: passed.
- Focused GUI GMDD job: passed.

### 3.2 Workflow trigger correction

- PR: #470.
- Head: `4dfeca8723ce2f2170c0d304768f0c95aa00857e`.
- Workflow run: `29772492281`, run 113.
- Tested merge commit: `c30139e8ff06111f91d95b7f69a863aaf203d219`.
- Configure/build/CTest `tests-unit`: passed.
- Focused GUI GMDD job: passed.
- Integrated into `WorkInProgress` as `5b24c1e4f31f1b001cd0bd6910fb2c134108fc77`.

Integrated corrections:

- ordinary CI PR target `2026-1` → `20261`;
- Debian workflow PR target `2026-1` → `20261`;
- Debian path filter `../../packaging/debian/**` → `debian/**`.

The Debian package job itself was not executed by this PR because the PR base was `WorkInProgress`, which is not an eligible Debian workflow PR target.

### 3.3 AI plugin test aggregation

- PR: #471.
- Final head: `5783c7166f9623aa3c42a92a6ef2bdcc0bdce436`.
- Workflow run: `29773299985`, run 117.
- Tested merge commit: `7ee8eeb73490da5fb1aae3cb256a9339f2da7f59`.
- Configure/build/CTest `tests-unit`: passed.
- Focused GUI GMDD job: passed.
- Integrated into `WorkInProgress` as `1fca8763cb7a6449cab719950ff74fb59e149b1e`.

The correction:

- adds `genesys_test_ai_plugins` to the ordinary aggregate dependencies;
- creates `genesys_test_ai_plugins_run` for the direct runner path;
- adds that run target to `genesys_kernel_unit_tests_run` dependencies.

The test source uses a local fake provider and dummy connector; it performs no network call and uses no credential.

## 4. GUI GMDD status

The current focused job executes:

1. `GuiGmddLayout.SeizeEditableReferencesStayAboveAndLowerDefinitionsUseTwoRows`;
2. `GuiGmddLayout.SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition`;
3. `GuiGmddLayout.SerializerRoundTripRestoresViewStateGeometriesAndGroups`.

All three passed in the current successful checkpoints. Older failure reports are historical snapshots unless reproduced on a specified newer commit.

## 5. Status changes justified

| Concern | Current status | Evidence |
|---|---|---|
| Ordinary `tests-unit` | `validated` for identified runs | configure/build/CTest passed |
| Focused GUI GMDD | `validated` for identified runs | 3/3 tests passed |
| Active branch trigger `20261` | `implemented-and-validated` | PR #470 merged |
| Debian root path filter | `implemented`; package execution pending | PR #470 merged |
| AI plugin inclusion in ordinary aggregate | `implemented-and-validated` | PR #471 merged and CTest passed |
| AI plugin direct kernel runner | `implemented`; explicit preset execution pending | target graph corrected |
| Full Phase 0 | `partially-validated` | kernel/smoke/inventory/application/package paths missing |
| Historical 16/1657 failures | `not-reproduced-in-current-tests-unit` | current ordinary runs passed |

## 6. Validation not yet performed

- `tests-kernel-unit` configure/build/CTest;
- `tests-smoke` configure/build/CTest;
- exact `ctest -N` counts and complete target inventory;
- shell/worker/GUI application startup;
- independent GUI application presets;
- model-specific application sweep;
- ASan/LSan/UBSan/Valgrind;
- Debian package build/install/start/uninstall after trigger correction;
- Qt5 fallback removal;
- worker authentication, CSPRNG, resource controls, and secret handling;
- numerical/statistical authoritative-reference validation;
- whole-cell/biochemical benchmark validity;
- future dynamic plugin ABI implementation.

## 7. Remaining Phase 0 actions

1. Execute `tests-kernel-unit` on Ubuntu 24.04/Qt6.
2. Execute `tests-smoke` on the same baseline.
3. Capture `ctest -N` and exact execution counts for all three presets.
4. Confirm `genesys_test_ai_plugins_run` through the explicit kernel preset.
5. Execute the Debian packaging workflow and package lifecycle.
6. Build representative application presets independently.
7. Preserve all branch, commit, run, job, dependency-mode, and artifact identifiers.

## 8. Next safe technical steps

After the missing baseline presets:

- open a focused regression-test PR for `SolverDefaultImpl1` before changing it;
- create the exact static plugin source-to-target/link map;
- remove Qt5 fallback in a dedicated PR;
- harden worker tokens and secret handling in separate security PRs.

No broad plugin, namespace, ownership, optimizer, or scientific-model refactoring is authorized by these green ordinary CI results.