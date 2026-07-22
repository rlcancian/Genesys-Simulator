---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# Current-State Consolidation — 2026-07-22

## 1. Scope

This record documents phase D2 of the AI-assistant documentation migration.

D2 establishes:

- `../../STATUS.md` as the only current operational state;
- `../../BACKLOG_AUTONOMOUS.md` as the only source of autonomously executable tasks;
- `../../BACKLOG_HUMAN.md` as the only source of material decisions and human-controlled plans.

No code, build, workflow, runtime, package, security, scientific algorithm, or release decision is changed.

## 2. Former competing sources

| Former top-level file | Blob SHA before retirement | Former role | Reason for retirement |
|---|---|---|---|
| `current_plans.md` | `ae6125e8006f16a42daa93fe7be6983f0d010292` | mixed plans, old branches, partially completed work and stale failures | mixed current and historical state; contained superseded GUI/CI claims |
| `genesys_2026_test_matrix.md` | `c96e9a1cdf2dda184e6ad71d478ed8b6238cb7c5` | cumulative validation matrix and backlog | duplicated current status and immutable evidence |
| `genesys_2026_module_inventory.md` | `c39dfcbd83d5606e76c8261029329dafb3d2eb36` | audit snapshot at an older commit | useful historical inventory, not continuously current architecture |
| `genesys_2026_consolidation_handoff.md` | `7d2beb944029c2cc2943d34e4b265e33e6eea37a` | operational handoff | duplicated STATUS and accumulated historical narrative |
| `genesys_2026_consolidation_plan.md` | `1c56a3d6feb62ed006db085ff51b43060a1b923b` | initial audit and macroplan | planning snapshot superseded by canonical backlogs and completed evidence |

Their former exact contents remain available through Git history and the blob SHAs above.

## 3. Current-state rules after D2

- current branch/commit/checks/blockers belong only in `STATUS.md`;
- actionable autonomous work belongs only in `BACKLOG_AUTONOMOUS.md`;
- unresolved architecture, security, science, release and product decisions belong only in `BACKLOG_HUMAN.md`;
- immutable run IDs, artifacts, commit-specific results and detailed diagnostics belong under `history/evidence/`;
- completed phase narratives belong under `history/migrations/` or `history/CHANGELOG_AI.md`;
- topic architecture and procedures belong under `reference/` after D4.

## 4. Preserved current facts

D2 preserves the current known baseline and boundaries, including:

- 1,721 tests registered, 1,717 passed, four historical duplicates disabled, zero failures in the latest exact Phase 0 inventory;
- shell, worker, Data Analyser, Optimizer and AI Assistant bounded startup validations;
- plugin target decision blocked by issue #492;
- shell autoload contract blocked by issue #496;
- worker bind contract blocked by issue #500;
- release and scientific validity not established;
- oldies deletion prohibited before the retention gate and explicit approval.

## 5. Migration behavior

The five former files are reduced to short migration notices during D2 so historical links do not break. D4/D5 may remove these notices after all active references use the canonical documents and the root allowlist can be enforced.

## 6. Non-claims

D2 does not assert that every old plan item is still required. Items not supported by current code, issues, evidence or maintainer decisions are historical until revalidated.

## 7. Follow-up

D3 relocates dated evidence and completed integration reports. D4 consolidates topic references and removes migration notices after link updates. D5 adds structural CI. D6 consolidates the oldies tracker while preserving retained historical files until the deletion gate.
