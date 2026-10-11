---
document_type: evidence-ledger
authority: informative
owner: project-maintainer
period: 2026-10
last_updated: 2026-10-11
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
