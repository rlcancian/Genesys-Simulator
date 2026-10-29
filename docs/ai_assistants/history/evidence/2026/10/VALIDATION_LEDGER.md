---
document_type: evidence-ledger
authority: informative
owner: project-maintainer
period: 2026-10
last_updated: 2026-10-29
status: active
tracks: 511
---

# GenESyS Validation Evidence Ledger — October 2026

## 1. Purpose

This ledger indexes material local validation for October 2026. It does not
replace current status, hosted CI, Git history, or the detailed task report.

## 2. BacteriaColony handoff

| Date | Scope | Branch / head | Commands / result | Bounded conclusion | Limitation |
|---|---|---|---|---|---|
| 2026-10-11 | `BC-HANDOFF-001`; clean candidate, `tumble()`, two-signal demo, viewer coordinates and regression presets | `WiP20261009/BacteriaColony-clean-candidate`; code fix `4d281158`; base `origin/WorkInProgress` `50a340d460876c657d8a3be66554bb29cfe8519a` | `ctest --preset tests-unit --output-on-failure` 1,902 passed / 4 disabled; `tests-kernel-unit` 1,902 / 4; `tests-smoke` 3/3; `cmake --preset gui-app` and build passed; `python3 scripts/validate-ai-docs.py` passed | Local regression suite passed; centered 8×8 viewer coordinate map corrected and unit tested; candidate history excludes the contaminated commit and third-party examples | GUI channel interaction/reset/calendar contention not demonstrated this run; local generated inventory file contributes 980 nonbehavioral `SUCCEED()` tests to the test inventory; no hosted CI evidence |
| 2026-10-11 | `BC-HANDOFF-002`; user cron, clean regression, two-signal viewer | `WiP20261009/BacteriaColony-clean-candidate`; initial HEAD `9345124f647cb4a7f910b5f2ff7577a38b253db9`, fast-forwarded to remote `bafa57b80d158253e283037a0df9e642b1f54a96`; validation worktree `/tmp/Genesys-bc002-validation` | `tests-unit` clean worktree: 922 behavioral passes / 4 disabled after excluding `KernelSimulatorMethodInventory`; kernel target exit 0; smoke 3/3; Qt6 `genesys_gui` build and real viewer interaction succeeded; local crontab entry `20 * * * *` installed; manual poll and scheduled poll at 14:20:02 -03 both logged `no_new_handoff` and exited without Codex | Model loaded in the real Qt6 viewer; runtime run produced 3→6 bacteria; Signal 1/2 had distinct ranges and 3,870 differing heatmap pixels; report includes screenshot paths and hashes | GUI reset and manual-step blocking during running/paused not interactively exercised; no hosted CI |

Detailed scope, source findings, provenance, discrepancies, and remaining gates
are recorded in [`BACTERIA_COLONY_AGENT_REPORT.md`](../../../../reference/BACTERIA_COLONY_AGENT_REPORT.md)
and the historical integration plan §22.9. Build/CTest results are local only.

## 3. Interpretation

- CTest passed counts include four disabled historical Search/Remove cases,
  which were not executed.
- The locally present generated method inventory is untracked and was not
  staged. Its `SUCCEED()` cases do not establish behavioral correctness.
- The GUI build proves compilation and linking, not viewer interaction.
- No hosted CI run, scientific calibration, or release readiness is claimed.

## 4. PR #550 continuous-model local audit

| Date | Scope | Branch / head | Commands / result | Bounded conclusion | Limitation |
|---|---|---|---|---|---|
| 2026-10-16 | PR #550 continuous temporal base, ODESolver, DiffusionField, BioNetwork, event lifetime and plugin loaders | `WiP20261016/ContinuousModels`; implementation `6cb42d1c`; based on `origin/WorkInProgress` `6df79436ff9d72e302e9d2b047fc5a8f6e1f297b` | `tests-unit`: 1,918 passed / 4 disabled; `tests-kernel-unit`: 1,917 passed / 4 disabled, plus focused T0=100 integration; `tests-smoke`: 3/3; focused ASan/UBSan 3/3 with LeakSanitizer disabled; developer manual rebuilt; User Manual Doxygen-LaTeX PDF failed on existing Unicode `Δ` | Deferred evolution until calendar dispatch fixed time-run-ahead; matrix tests cover sync counts 0/1/2/3/5/20, final partial steps, nonzero global origin, event destruction and BioNetwork replication reset | Nonzero `DiffusionSimulate` component override, LeakSanitizer, User Manual PDF, final-head hosted CI and exhaustive legacy model compatibility remain unverified; PR remains draft |

Detailed diagnosis, commands, environment, test counts, sanitizer scope, manual impact, and remaining decisions are recorded in [`2026-10-16_PR550_ContinuousModels_local_handoff.md`](2026-10-16_PR550_ContinuousModels_local_handoff.md).

| 2026-10-29 | PR #550 continuous model component integration and completion lifecycle | `WiP20261016/ContinuousModels`; starting head `c00921b91db131087cff16c12488940d38801f32` | Shared continuous component base, typed execution state, asynchronous entity-retention tests, unit/kernel/smoke and GUI build results, focused ASan/UBSan and LeakSanitizer outcome are recorded in the linked handoff | Local code implements a causal calendar-driven lifecycle and routes entities only after completion; the continuous state observed between synchronization events remains at the last completed synchronization point | `HUM-SCI-002` time semantics remain a maintainer decision; LeakSanitizer reported plugin-fixture/process-lifetime allocations; final hosted CI must be checked after push; unrelated GUI crash fix already in branch history |
| 2026-10-29 | PR #550 continuous state Observable and hybrid synchronization | [`2026-10-29_ContinuousModels_Observable_Implementation_Handoff.md`](2026-10-29_ContinuousModels_Observable_Implementation_Handoff.md); starting head `ad2c261ec7878638a8812090599397cee78eed69` | Per-step typed state observers, confirmed/work state separation, real-kernel intermediate event test, unit/kernel/smoke and GUI builds, docs/manual validation, sanitizer outcome and hosted checks | `HUM-SCI-002` is implemented for deterministic models; observer can schedule an event at an intermediate step while ordinary discrete readers retain the last confirmed state | Decision still needed for invalidating/replaying precomputed work and retracting observer-derived events after an ordinary event changes a continuous input; Doxygen PDF fails on a pre-existing Unicode delta; final sanitizer and post-push CI results are recorded before publication |

Detailed diagnosis, architectural decisions, exact commands, test counts, sanitizer evidence, changed files, and follow-up gates are recorded in [`2026-10-29_ContinuousModels_Component_Integration_Handoff.md`](2026-10-29_ContinuousModels_Component_Integration_Handoff.md).
