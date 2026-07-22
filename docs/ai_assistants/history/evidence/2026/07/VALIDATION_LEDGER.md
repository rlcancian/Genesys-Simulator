---
document_type: evidence-ledger
authority: informative
owner: project-maintainer
period: 2026-07
last_updated: 2026-07-22
status: active
tracks: 511
---

# GenESyS Validation Evidence Ledger — July 2026

## 1. Purpose

This ledger indexes material executed evidence produced during the July 2026 consolidation cycle.

It does not replace GitHub Actions artifacts, PR discussions, issues, Git history or source inspection. Each entry records the strongest bounded conclusion and the main limitation. Former detailed reports remain recoverable from Git history through their pre-retirement blob SHAs.

## 2. Core and runtime evidence

| Date | Scope | PRs / merge | Runs / artifacts | Bounded conclusion | Main limitation | Former report blob |
|---|---|---|---|---|---|---|
| 2026-07-20 | Phase 0 ordinary, kernel and smoke baseline | PRs #469–#473; Phase 0 merge `802b8aec7ac129559692bd574e70fd9991aaec1d` | Phase 0 run `29780136722` | ordinary CI, kernel-focused graph and three smoke tests passed for recorded head | old inventory was later expanded; no release/scientific claim | `e69f70ffc3c37c33e62e0c1442f81397bd71d374` |
| 2026-07-21 | Legacy solver and Search/Remove/runtime lifecycle | PRs #474–#480; through merge `4f98a909d941ac31582205904c597fb345d3527f` | focused red/green plus ordinary/kernel/smoke runs | solver contract stabilized; Search/Remove active; Queue/Station/Delay/Resource safe on first public use | broader numerical/accounting combinations remain | `9cee198726c18399af0e47846b32902ef4418c49` |
| 2026-07-21 | Plugin-completion ownership and optimizer copy safety | PR #483 merge `6d6dd4edc610ec5271e12cb42a89c37f0325b6c3`; PR #485 merge `6aca91a55beaf4ee8838c4843159ec4e06456318` | focused ASan/LSan final run `29831405860`, artifact `8495543015`; Phase 0 run `29833758797`, artifact `8496613101` | exercised leak reduced from 27,533 bytes/470 allocations to zero; optimizer shallow copy/move prohibited | repository-wide leak freedom and optimizer functionality not established | `73958defee978b368787aea57b430a6f4c56d34b` |

## 3. Static plugin target evidence

| Date | Scope | PR / issue | Runs / artifacts | Bounded conclusion | Main limitation | Former report blob |
|---|---|---|---|---|---|---|
| 2026-07-21 | Source/consumer overlap inventory | issue #487; PR #488 | repository/CMake inspection | full and minimal target definitions select the same component source domain | generated graph required for exhaustive consumer proof | `596356992f0cc2aed75e4f97c0481f2d1f984a4b` |
| 2026-07-21 | Generated CMake codemodel | PR #489 merge `b3e027483066b903a0389285082163b86c2df362` | ordinary run `29856007581`; introspection run `29856007683`; artifact `8505306513` | both archives compile identical 84-source sets; runtime principally uses minimal; GLPK only on full | behavioral neutrality of consolidation not proven | `bda6a36b9717d20c08f9d4ecb511545766cccfbb` |
| 2026-07-22 | GLPK-present/absent links and symbols | PR #491 merge `8520c542a80697aad16527a89cec6b4d675d5797`; decision issue #492 | ordinary run `29873203393`; matrix run `29873203342`; artifacts `8512049198`, `8512054888` | focused test links both archives; symbol sets identical without GLPK and diverge in FBA implementation with GLPK | no runtime crash or duplicate-symbol failure demonstrated; architecture choice remains human | `cf05a32f8e7ca1b548abdca2b04bcdfe992942e8` |

## 4. Standalone application evidence

| Date | Application | PR / merge | Run / artifact | Bounded conclusion | Main limitation | Former report blob |
|---|---|---|---|---|---|---|
| 2026-07-22 | Shell | PR #495, merge `abb992ec2775a64b381b3604d981d0bd18161dc6`; issue #496 | run `29885199488`; artifact `8516303352` | preset/build/scripted commands/plugin count/exit passed | model workflow and file-based autoload deployment unresolved | `073b8c1b11618827e25df57b75ad8102ce65b72d` |
| 2026-07-22 | Worker | PR #499, merge `8f42f50992b0bd53759018f09e46f48434839bf7`; issue #500 | run `29896225187`; artifact `8520123900` | preset/build/loopback health/exact JSON/request-limited exit passed | listener bound wildcard; authentication/TLS/quotas/isolation not validated | `60c2c3be4f810f5ccfe113923e0736e56e424e77` |
| 2026-07-22 | Data Analyser GUI | PR #503, merge `b3c7f462666c955d2c6d6429c1d5636290ef25f1` | run `29911036076`; artifact `8525948784` | Qt6/XCB process created PID-associated X11 window and survived bounded startup | no interaction, data analysis or scientific correctness | `4decc598360c7532a0466e6111ee0d2bc02880a3` |
| 2026-07-22 | Optimizer GUI | PR #507 merge `408c62dcd428de38708df4eac11cad287fb84f13`; evidence PR #508 merge `fbf67b76c7d1e86e9431b680aa10de465d657b51` | focused run `29912799769`; artifact `8526654357` | Qt6/XCB startup/window/liveness/teardown passed | no implemented optimizer algorithm or Level 3 claim | `189b04dd6215d05ae201b977182984e08c1d6b6f` |
| 2026-07-22 | AI Assistant GUI | PR #510 merge `ca47f7f05b0414190fedf73da947dd3d5c5e2456`; issue #509 | focused run `29919067547`; ordinary run `29919067854`; artifact `8529227458` | no-credential Qt6/XCB startup/window/liveness/teardown passed without external provider call | provider workflow, credentials, redaction and failures unvalidated | no former standalone report; artifact/PR are primary evidence |

## 5. Current exact baseline

The latest retained exact kernel inventory is:

- registered: 1,721;
- executed/passed: 1,717;
- failed: 0;
- disabled: 4 historical duplicate Search/Remove blocks.

Current status and future gaps belong in `../../../STATUS.md`, not in this immutable ledger.

## 6. Interpretation rules

- evidence is valid only for the recorded commit, environment and exercised scope;
- startup evidence does not prove feature workflows;
- unit/smoke evidence does not prove scientific correctness;
- focused sanitizer evidence does not prove repository-wide resource safety;
- an artifact is stronger than a PR description but does not exceed its captured scope;
- later evidence may supersede current-state interpretation without altering this historical ledger.

## 7. Source-retirement map

The former detailed reports are reduced to migration notices during D3 and removed after active links are updated in D4/D5. Their exact contents remain in Git history under the blob SHAs recorded in this ledger.
