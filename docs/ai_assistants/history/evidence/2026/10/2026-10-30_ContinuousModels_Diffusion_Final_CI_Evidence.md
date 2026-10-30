---
document_type: validation-evidence
authority: informative
owner: continuous-models-implementation
created: 2026-10-30
status: active
tracks: 550
---

# PR #550 — final hosted checks for diffusion component regression

## Scope and checkpoint

This records the GitHub Actions checks associated with PR #550 head
`9c0f40da98b39e8a0a56fa3d4a5d420d224b81e7` on 2026-10-30. The source/test
change under validation is `c9881a20b`, adding a real-calendar interruption
and entity-retention regression for `DiffusionSimulate`. The head also includes
the evidence and status documentation updates current at that checkpoint.

## Hosted result

**Confirmed by execution:** `gh pr checks 550` reported all 16 checks as
passed. The CI workflows for the pull request ran against the generated PR
merge ref (synthetic merge SHA `14ce4daf1dd8f1153962d733534950c165b7bb77`)
with head `9c0f40da98b39e8a0a56fa3d4a5d420d224b81e7`.

| Check | Result | Run |
|---|---|---|
| Unit tests | PASS | [GenESyS CI](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009939) |
| Kernel tests | PASS | [GenESyS Phase 0](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059010045) |
| Smoke tests | PASS | [GenESyS Phase 0](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059010045) |
| GUI GMDD diagnostics | PASS | [GenESyS CI](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009939) |
| AI Assistant GUI startup under Xvfb | PASS | [AI Assistant GUI](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009948) |
| Data Analyser GUI startup under Xvfb | PASS | [Data Analyser GUI](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009899) |
| Optimizer GUI startup under Xvfb | PASS | [Optimizer GUI](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009900) |
| Worker build/exercise | PASS | [Worker](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009961) |
| Shell build/exercise | PASS | [Shell](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009944) |
| Launcher | PASS | [Launcher](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009982) |
| Debian package build | PASS | [Debian package](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009945) |
| Debian package lifecycle | PASS | [Debian package](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009945) |
| Plugin link evidence, present GLPK | PASS | [Plugin link evidence](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009893) |
| Plugin link evidence, absent GLPK | PASS | [Plugin link evidence](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009893) |
| Plugin target introspection | PASS | [Plugin target introspection](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009887) |
| AI documentation governance | PASS | [Documentation governance](https://github.com/rlcancian/Genesys-Simulator/actions/runs/41059009902) |

## Interpretation and limits

**Confirmed by execution:** all 16 automated checks associated with the
checkpoint passed. Local validation for `c9881a20b` is recorded in
[`2026-10-30_ContinuousModels_DiffusionComponentCalendar_Evidence.md`](2026-10-30_ContinuousModels_DiffusionComponentCalendar_Evidence.md).

These checks do not establish interactive behavior of the main GUI, scientific
calibration, or complete lifecycle/persistence coverage. PR #550 remains open
and draft without an independent review decision. The input-invalidation
condition recorded under `HUM-SCI-002` and the complete historical PR scope
review remain open; maturity stays `PARTIALLY_VALIDATED`.
