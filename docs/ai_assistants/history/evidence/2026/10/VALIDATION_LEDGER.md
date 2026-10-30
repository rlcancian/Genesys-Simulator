---
document_type: evidence-ledger
authority: informative
owner: project-maintainer
period: 2026-10
last_updated: 2026-10-30
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
| 2026-10-29 | PR #550 continuous state Observable and hybrid synchronization | [`2026-10-29_ContinuousModels_Observable_Implementation_Handoff.md`](2026-10-29_ContinuousModels_Observable_Implementation_Handoff.md); starting head `ad2c261ec7878638a8812090599397cee78eed69`; validated implementation/docs SHA `d3462b987c2a492cd45031ce6d0b9e354a2f2666` | Per-step typed state observers, confirmed/work state separation, real-kernel intermediate event test, unit/kernel/smoke and GUI builds, docs/manual validation, ASan/UBSan/LSan focused tests; all hosted checks passed on validated SHA | `HUM-SCI-002` is implemented for deterministic models; observer can schedule an event at an intermediate step while ordinary discrete readers retain the last confirmed state | Decision still needed for invalidating/replaying precomputed work and retracting observer-derived events after an ordinary event changes a continuous input; Doxygen PDF fails on a pre-existing Unicode delta; sanitizer baseline comparison against WorkInProgress was not run |

| 2026-10-30 | PR #550 calendar-bounded continuous execution and entity continuation | [`2026-10-29_ContinuousModels_CalendarBoundedExecution_Handoff.md`](2026-10-29_ContinuousModels_CalendarBoundedExecution_Handoff.md); starting head `efc3efcbef31ac259d141582fb9d2906b7c8f2ac` | Calendar horizon limits, per-step recheck, conventional entity return at next grid point, auto-start deferral, invalid-window atomicity; unit 1,933 pass / 4 disabled, kernel 1,933 / 4, smoke 3/3, GUI Qt6 build, focused ASan/UBSan/LSan 32/32 | Real-kernel tests demonstrate event 105.5 at origin 100 observing confirmed local time 5, same entity return at 106, completion at 110, and initial bounds at 0.5/5.5/9.9/10; the kernel comparator remains unchanged | Post-push GitHub checks pending; `StepsToSynchronize=0` remains synchronous; non-grid discontinuities are integrated on the existing step grid; scan optimization not applied |

Detailed diagnosis, architectural decisions, exact commands, test counts, sanitizer evidence, changed files, and follow-up gates are recorded in [`2026-10-29_ContinuousModels_Component_Integration_Handoff.md`](2026-10-29_ContinuousModels_Component_Integration_Handoff.md).

| 2026-10-30 | PR #550 ODE component async lifecycle, ODE equation dependencies, BioNetwork resumption, and review regressions | [`2026-10-30_ContinuousModels_ODE_Bio_Integration_Handoff.md`](2026-10-30_ContinuousModels_ODE_Bio_Integration_Handoff.md); branch `WiP20261016/ContinuousModels`; starting SHA `0bb9cbb66bef8650ec68d1a1dbbff292e64f9b6e`; code fix `0e6ac0cd8`; documentation `e962e37d4` | `ContinuousSystemComponent` uses shared async lifecycle; ODE retains equation dependencies; 36 focused continuous tests pass; unit 1,939 / 4 disabled; kernel 1,939 / 4; smoke 3/3; GUI Qt6 builds; 4 ASan/UBSan regressions pass with LSan disabled. LSan-enabled ODE and Bio component tests report the same 4,910 bytes/161 plugin-bootstrap allocations; diffusion and synchronous Bio cases are leak-clean; baseline comparison pending. Checks on prior SHA `764ac0954` passed; CI for `e962e37d4` was pending at evidence capture | ODE sees rate change 1→2 at event 5.5, resumes from confirmed state 5 and ends at 15; ODE respects `MaxSteps=2` across sync batches; Bio component forwards both entities at completion and exports terminal `Completed`; diffusion rejects nonzero local window start | LSan comparison against identical WorkInProgress fixture, interactive GUI, complete cross-domain/cancellation/lifetime matrix, historical reactive ODE compatibility, and final-head CI remain outstanding; PR remains draft and unmerged |

| 2026-10-30 | PR #550 post-step calendar check and independent autonomous ODE coexistence | [`2026-10-30_ContinuousModels_Final_Validation.md`](2026-10-30_ContinuousModels_Final_Validation.md); starting SHA `2f14420eace7f36f530e5f6b19153772659b86e6`; source/test commit `419d8287f` | Two independent ODE definitions with different synchronization intervals complete in a real `ModelSimulation` calendar; 37 focused continuous tests, unit 1,940/4 disabled, kernel 1,940/4, smoke 3/3, focused ASan/UBSan 19/19, GUI build and Developer Manual PDF build pass | Post-step event validation ignores queued events at or behind current global time, preventing same-timestamp events from being treated as past while preserving the calendar comparator | LSan reports 4,910 bytes/161 allocations without an equivalent `WorkInProgress` baseline; final-head CI, human PR review, broader lifecycle/persistence matrix and `HUM-SCI-002` invalidation condition remain outstanding; status `PARTIALLY_VALIDATED` |
