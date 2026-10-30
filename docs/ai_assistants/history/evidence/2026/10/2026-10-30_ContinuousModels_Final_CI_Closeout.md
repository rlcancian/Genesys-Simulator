---
document_type: evidence-report
authority: informative
owner: continuous-models-implementation
date: 2026-10-30
status: partial-validation
tracks: 550
---

# Continuous Models — CI closeout evidence

## 1. Escopo analisado

This closeout records the additional three-domain shared-calendar regression,
its local validation, and the complete GitHub check set for the tested PR head.
It does not assert that the full Continuous Models acceptance matrix or human
review is complete.

## 2. Diagnóstico inicial

**Confirmed in code:** prior tests covered the three data-definition families
and their component lifecycles in separate tests, but there was no single
regression starting ODE, diffusion, and BioNetwork together on one live
`ModelSimulation` calendar with distinct synchronization intervals.

**Confirmed in code:** `BioNetwork` retains its species and reaction definitions
through a connected `BioSimulate` component; an initial fixture without such an
owner failed `Model::check()` because model checking removed unreferenced
biochemical definitions. The fixture was corrected to follow the existing
ownership contract; no production workaround was added.

## 3. Evidências e problemas encontrados

- **Confirmed by execution:** the new test
  `ContinuousCalendarTest.OdeDiffusionAndBioNetworkShareOneRealEventCalendar`
  passes. It starts an ODE (`y'=1`, STS 2), a diffusion field (STS 3), and a
  biochemical A→B network (STS 1) from replication-start handling, then runs
  them through the actual `ModelSimulation` calendar.
- **Confirmed by execution:** all three runs reach `Completed`; ODE state and
  diffusion local time reach 4; the biochemical A+B amount is conserved within
  `1e-12`; global simulation time ends at 4.
- **Confirmed by execution:** the complete
  `genesys_test_plugins_continuous_diffusion` target passes 40/40.
- **Confirmed by execution:** final local CTest on the same source/test tree
  enumerates 1,945 cases, of which 1,941 pass, 4 historical cases are disabled,
  and 0 fail, for each of `tests-unit` and `tests-kernel-unit`. Smoke passes 3/3.
- **Confirmed by execution:** focused ASan/UBSan passes 20/20 for continuous
  calendar and Observable tests with `ASAN_OPTIONS=detect_leaks=0`. LeakSanitizer
  was therefore not active in that focused run.
- **Confirmed by execution:** all 16 GitHub checks passed for PR #550 at
  `69a4a25b1f069f689fb944e16a34c16169996e59`, including unit, kernel, smoke,
  GUI GMDD, Qt application startup under Xvfb, Worker, Shell, Launcher,
  Debian package build and lifecycle, documentation governance, and archive /
  plugin-target inspections.
- **Strong evidence:** LSan reported 4,910 bytes/161 allocations in both a
  branch ODE fixture and a `WorkInProgress` plugin-bootstrap baseline, with
  matching normalized stacks rooted in `Buffer` templates created by
  `PluginManager::completePluginsFieldsAndTemplates()`. Fixture bodies differ,
  so this is not proof that all allocation paths are identical.

## 4. Impactos arquiteturais

The new test demonstrates concurrent logical execution of the three continuous
data-definition types on the real event calendar with different
`StepsToSynchronize`. It does not change production architecture, event ordering,
clock ownership, persistence, scientific algorithms, or the `Failure` plugin.

## 5. Solução implementada

Added a behavioral regression and retained explicit ownership of the
BioNetwork's members through a connected component. No production code change
was necessary for this coverage gap.

## 6. Patches e commits

- `d658c4884` — `test(continuous): verify three-domain calendar coexistence`;
  adds the regression, this test's focused evidence, and a ledger entry.
- `69a4a25b1` — `docs(continuous): update current validation checkpoint`;
  updates STATUS/backlog and records the test checkpoint.
- The complete automated validation above is for `69a4a25b1`.

The PR remains open and draft. `reviewDecision` is empty. No merge was made.
Unrelated pre-existing local changes in `Resource.h` and untracked `teste1.gui`
remain untouched and unstaged.

## 7. Validação

Local commands and outcomes:

```text
cmake --build --preset tests-unit --parallel 2
ctest --preset tests-unit --output-on-failure
# 1,941 passed, 4 disabled, 0 failed

cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
# 1,941 passed, 4 disabled, 0 failed

cmake --build --preset tests-smoke --parallel 2
ctest --preset tests-smoke --output-on-failure
# 3/3 passed

cmake --build --preset tests-unit --target genesys_test_plugins_continuous_diffusion --parallel 2
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion
# 40/40 passed

cmake --build build/observable-asan --target genesys_test_plugins_continuous_diffusion --parallel 2
ASAN_OPTIONS=detect_leaks=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter='ContinuousCalendarTest.OdeDiffusionAndBioNetworkShareOneRealEventCalendar:ContinuousCalendarTest.*:ContinuousObservableTest.*'
# 20/20 passed; ASan/UBSan enabled, LSan disabled

python3 scripts/validate-ai-docs.py
# passed
```

Qt6 GUI build and Developer Manual PDF generation had passed on an earlier
source-identical checkpoint; the final CI additionally passed startup checks for
the AI Assistant, Data Analyser, and Optimizer under Xvfb and GUI GMDD
validation. No interactive main-GUI model editing or simulation was performed
in this closeout.

## 8. Riscos remanescentes

- `HUM-SCI-002` records an unresolved policy for ordinary discrete events that
  mutate inputs already used by precomputed continuous work. No rollback or
  retraction model was invented.
- The broad cancellation/destruction, cross-model mutation, persistence
  round-trip, and entity lifecycle matrices are not complete for all three
  domains. Existing individual tests cover parts of them.
- The single shared-calendar regression exercises autonomous data-definition
  requests from replication-start handling; it does not by itself prove a
  single entity-driven, calendar-interrupted component flow across all three
  domains.
- The complete historical PR diff still contains a GUI crash-diagnostics fix
  and other changes beyond continuous simulation; independent review must
  inspect and justify the whole PR, not only the latest commits.
- The PR is draft and has no recorded independent review. Passing CI does not
  authorize merge under repository governance.

## 9. Próximos passos

1. Obtain independent review of the complete PR, including GUI and generated
   documentation changes.
2. Resolve the `HUM-SCI-002` input-invalidation condition or obtain an explicit
   maintainer decision excluding the unsupported interaction.
3. Complete remaining lifecycle, persistence, and cross-model tests before
   claiming `done_confirmed`.
4. Keep PR #550 draft and unmerged until the governance gates are satisfied.

**Maturity:** `PARTIALLY_VALIDATED`.
