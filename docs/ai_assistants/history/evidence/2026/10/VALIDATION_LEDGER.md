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

## 3. EFSM editor post-merge closeout

| Date | Scope | Branch / head | Commands / result | Bounded conclusion | Limitation |
|---|---|---|---|---|---|
| 2026-10-30 | EFSM User Manual integration wording and official PDF regeneration | Base `WorkInProgress` `1d31cf7e6021c246f55713144b1014c5b841ec6c`; PR #555 | `doxygen docs/users/DoxyfileUser` passed; `latexmk -pdf -interaction=nonstopmode -halt-on-error refman.tex` passed; PDF 3,981 pages; guide page 177 visually inspected | The manual now identifies the EFSM editor as integrated and the official user PDF contains the corrected wording | Doxygen emitted source-comment reference warnings; LaTeX reported 14 duplicate labels but no compilation failure; details at [`2026-10-30_modal_efsm_user_manual_final.md`](2026-10-30_modal_efsm_user_manual_final.md) |
| 2026-10-30 | Final active-document audit of EFSM status and historical Modal/Network checkpoints | `WorkInProgress` `e74e477f2eeeddd089df52c41547442171a513fd` | Focused CTest selection for EFSM persistence/editor/undo and ModalModel association: 45/45 passed; `python3 scripts/validate-ai-docs.py` passed | Current EFSM implementation remains integrated; outdated active statements were reconciled to historical checkpoints | Documentation-only correction; full baselines and hosted checks are linked from the closeout report |
| 2026-10-30 | Integrated GUI save/reopen followed by kernel EFSM activation; status/evidence reconciliation | Base `WorkInProgress` `17338f8e566f87fc3b06eb422a5dd354e6d57b02`; PR #552 merged as `51851415fb8abe33f08a67d1d571ad233ab081a5` | Focused EFSM GUI 37/37; ModalModel/EFSM 26/26; unit and kernel CTest each 1,963 passed / 4 disabled; smoke 3/3; GUI build passed; all eight hosted checks passed | The MainWindow-created, edited, saved and reloaded EFSM activated through the kernel API with expected output and final state; no kernel production code changed | No separate manual mouse/keyboard session; sanitizer evidence is from PR #551 and LSan was disabled; see [`2026-10-30_modal_efsm_postmerge_closeout.md`](2026-10-30_modal_efsm_postmerge_closeout.md) |

## 4. Interpretation

- CTest passed counts include four disabled historical Search/Remove cases,
  which were not executed.
- The locally present generated method inventory is untracked and was not
  staged. Its `SUCCEED()` cases do not establish behavioral correctness.
- The GUI build proves compilation and linking, not viewer interaction.
- No hosted CI run, scientific calibration, or release readiness is claimed.
