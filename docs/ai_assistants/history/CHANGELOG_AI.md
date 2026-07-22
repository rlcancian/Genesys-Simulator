---
document_type: history-index
authority: informative
owner: project-maintainer
last_updated: 2026-07-22
status: active
tracks: 511
---

# GenESyS AI-Assisted Development Changelog

## 1. Purpose

This is a concise index of material AI-assisted development and governance changes. It does not replace Git history, pull-request discussions, issues, workflow runs or artifacts. Do not paste full logs here.

## 2. 2026 entries

| Date | Scope | Environment | PR/issue | Merge/checkpoint | Validation/evidence | Remaining boundary |
|---|---|---|---|---|---|---|
| 2026-07-20 | Correct CI/Debian branch and path triggers | GitHub | PR #470 | `5b24ab9277306b26a8742ac5b25cbca9d89ed6df` | ordinary CI green | package lifecycle still required |
| 2026-07-20 | Add AI plugin tests to ordinary/direct-runner baseline | GitHub | PR #471 | `1fcac9f678c3c96a40cd921c0a3d54ea03bdf64c` | ordinary and kernel paths green | provider/security breadth pending |
| 2026-07-20 | Add reusable Phase 0 kernel/smoke validation | GitHub | PR #472 | `802b8aec7ac129559692bd574e70fd9991aaec1d` | kernel/smoke artifacts green | applications/packages not covered |
| 2026-07-20 | Record consolidation decisions and initial evidence | GitHub | PRs #469/#473 | `9c52b61532b847668adc3be92c780966301bcf7c`, `a56b92cc83f1da5573def1f7dda26da7f9c56938` | documentation plus ordinary CI | later status documents supersede volatile claims |
| 2026-07-21 | Stabilize legacy solver contract | GitHub | PR #474 | `82af912d369f92d9365536ec5a6ba5ee75f3414d` | focused red/green and full baseline | authoritative broader numerical validation pending |
| 2026-07-21 | Activate Search/Remove runtime coverage | GitHub | PR #475 | `c34c6bb542149787cde7329345a460a40b73befe` | four focused tests green | four disabled historical duplicates remain cleanup debt |
| 2026-07-21 | Correct Queue/Station/Delay/Resource first-use lifecycle | GitHub | PRs #476/#478/#479/#480 | through `4f98a909d941ac31582205904c597fb345d3527f` | ordinary/kernel/smoke green | broader accounting and sanitizer breadth pending |
| 2026-07-21 | Correct plugin-completion ownership with ASan/LSan | GitHub | PR #483 | `6d6dd4edc610ec5271e12cb42a89c37f0325b6c3` | focused leak reduced to zero for exercised path | repository-wide leak freedom not established |
| 2026-07-21 | Prohibit unsafe Optimizer copy/move | GitHub | PR #485 | `6aca91a55beaf4ee8838c4843159ec4e06456318` | compile-time ownership contract green | optimizer remains scaffold |
| 2026-07-21/22 | Map duplicate plugin targets and GLPK divergence | GitHub | issues #487/#492; PRs #488–#491 | through `8520c542a80697aad16527a89cec6b4d675d5797` | CMake File API, symbols and link artifacts | implementation blocked on issue #492 |
| 2026-07-22 | Validate standalone shell | GitHub | PR #495; issue #496 | `abb992ec2775a64b381b3604d981d0bd18161dc6` | preset/build/argv/plugin count/exit green | autoload deployment decision #496 |
| 2026-07-22 | Validate standalone worker public health | GitHub | PR #499; issue #500 | `8f42f50992b0bd53759018f09e46f48434839bf7` | build/HTTP 200/exact JSON/clean exit | wildcard bind and security decisions pending |
| 2026-07-22 | Validate Data Analyser GUI startup | GitHub | PR #503 | `b3c7f462666c955d2c6d6429c1d5636290ef25f1` | Qt6/Xvfb PID-window/liveness/teardown green | functional/scientific workflows pending |
| 2026-07-22 | Validate Optimizer GUI startup and record evidence | GitHub | PRs #507/#508 | `408c62dcd428de38708df4eac11cad287fb84f13`, `fbf67b76c7d1e86e9431b680aa10de465d657b51` | Qt6/Xvfb startup artifact green | algorithms and maturity not validated |
| 2026-07-22 | Validate AI Assistant GUI startup | GitHub | PR #510; issue #509 | `ca47f7f05b0414190fedf73da947dd3d5c5e2456` | runs `29919067547`/`29919067854`, artifact `8529227458` | credentials/provider workflow/security pending |
| 2026-07-22 | Close superseded Optimizer GUI draft | GitHub | PR #506 | closed without merge | replacement #507 already integrated | none |
| 2026-07-22 | Establish canonical AI documentation governance D0 | GitHub | issue #511; PR #512 | `958cdc6f63c02d004f1ffdf55e104b58a245bb88` | run `29929616027`, ordinary tests and GUI GMDD green | effective consolidation followed |
| 2026-07-22 | Consolidate normative governance D1 | GitHub | issue #511; PR #513 | `b48697e77d39b25cafc19271ce574bdead60f94d` | run `29931594603`, ordinary tests and GUI GMDD green | state/evidence/reference consolidation followed |
| 2026-07-22 | Consolidate current status/backlogs D2 | GitHub | issue #511; PR #514 | `53b49f7518509823fe2265a3f017b5aa76f09d2f` | run `29933330431`, ordinary tests and GUI GMDD green | evidence/reference consolidation followed |
| 2026-07-22 | Consolidate executed evidence D3 | GitHub | issue #511; PR #515 | `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca` | run `29934227250`, ordinary tests and GUI GMDD green | reference/oldies/CI phases remain |

## 3. Update rule

Append or amend one row only after a material status change. After merge, use the merge commit, record the strongest relevant validation, identify the principal remaining boundary, and synchronize `STATUS.md` plus the applicable backlog.
