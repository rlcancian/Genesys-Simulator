---
document_type: backlog
authority: executable-task-source
owner: project-maintainer
last_updated: 2026-07-22
review_cadence: on-status-change
status: active
tracks: 511
---

# GenESyS Autonomous Agent Backlog

## 1. Purpose

This file is the only approved source for tasks that an AI agent may execute without a new material human decision.

A task may run only when its status is `ready`, its environment is available, dependencies are resolved, and no maintainer freeze excludes it. During the documentation migration freeze, only issue #511 documentation work may run.

## 2. Status values

- `ready` — fully specified and eligible;
- `running` — owned by one active branch/PR;
- `blocked-review` — prepared but waiting for review;
- `blocked-dependency` — waits for another task/decision;
- `paused` — executable but disabled by maintainer instruction;
- `done` — accepted, validated and merged;
- `cancelled` — intentionally removed.

## 3. Documentation migration

### AUTO-DOC-001 — Establish canonical governance layer

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #512
- Merge: `958cdc6f63c02d004f1ffdf55e104b58a245bb88`
- Result: six canonical documents, three environment runbooks, migration/history/reference/archive indices and a short routing README.
- Validation: run `29929616027`, ordinary tests and GUI GMDD green.
- Source branch: removed automatically.

### AUTO-DOC-002 — Consolidate normative governance and architecture

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #513
- Merge: `b48697e77d39b25cafc19271ce574bdead60f94d`
- Result: branch/promotion, documentation, security, maturity, plugin and evidence policy consolidated into canonical governance/architecture; four competing policy files retired.
- Validation: run `29931594603`, ordinary tests and GUI GMDD green.
- Source branch: removed automatically.

### AUTO-DOC-003 — Consolidate current state and plans

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #514
- Merge: `53b49f7518509823fe2265a3f017b5aa76f09d2f`
- Result: `STATUS.md` and the two backlogs became the only operational state/task sources; five competing plan/matrix/inventory/handoff files retired.
- Validation: run `29933330431`, ordinary tests and GUI GMDD green.
- Source branch: removed automatically.

### AUTO-DOC-004 — Consolidate executed evidence

- Priority: `P1`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #515
- Merge: `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca`
- Result: date-first July 2026 validation ledger, active evidence index, ten detailed reports retired with original blob SHAs preserved.
- Validation: run `29934227250`, ordinary tests and GUI GMDD green.
- Source branch: removed automatically.

### AUTO-DOC-005 — Consolidate technical references and clean active root

- Priority: `P0`
- Status: `running`
- Environment: `github`
- Branch: `WiP20260722/ai-docs-reference`
- Objective:
  - replace scattered topic guides/plans with six references;
  - update root README to canonical routing;
  - remove temporary D1–D3 redirects and superseded topic files;
  - leave only six canonical Markdown files at the AI-assistant top level plus the three temporary oldies trackers until D6.
- Active references:
  - `reference/BUILD_TEST_PACKAGING.md`;
  - `reference/KERNEL_PARSER_OWNERSHIP.md`;
  - `reference/PLUGINS.md`;
  - `reference/SCIENTIFIC_DOMAINS.md`;
  - `reference/APPLICATIONS_TOOLS_MODELS.md`;
  - `reference/API_INTEGRATIONS.md`.
- Non-goals: no source/CMake/runtime/scientific/security implementation; no deletion of files inside `oldies/`.
- Acceptance:
  - no active guide/plan/evidence remains in the top-level root;
  - all technical content is routed to canonical references/backlogs/history;
  - root README links only to canonical paths;
  - ordinary CI green;
  - source branch removed after merge.
- Tracking issue: #511.

### AUTO-DOC-007 — Consolidate oldies governance and classify retained history

- Priority: `P0`
- Status: `ready`
- Environment: `github`
- Objective:
  - merge `oldies_inventory.md`, `oldies_review_status.md` and `consolidation_map.md` into `archive/OLDIES_REVIEW.md`;
  - classify every retained historical file;
  - remove the three old top-level trackers;
  - preserve every `oldies/` content file until the retention gate and explicit approval.
- Dependencies: AUTO-DOC-005 merged.
- Acceptance before gate:
  - one active tracker;
  - all historical files listed/classified;
  - no canonical document depends on oldies for current policy/state;
  - no `oldies/` file deleted.
- Stop/escalation: unique or uncertain historical content must be marked for human review, not discarded.
- Tracking issue: #511.

### AUTO-DOC-006 — Add documentation-governance CI

- Priority: `P0`
- Status: `blocked-dependency`
- Environment: `github` or `local`
- Objective: enforce the final structure after AUTO-DOC-005 and AUTO-DOC-007.
- Required checks:
  - exact six-file top-level Markdown allowlist;
  - relative-link validation for active documentation;
  - required front matter for canonical/runbook/reference/history indices;
  - unique backlog IDs and required task fields;
  - no run IDs in normative governance/architecture;
  - evidence ledgers restricted to `history/evidence/`;
  - oldies retention gate and tracker presence;
  - no legacy migration notices or `plugins/` guide directory.
- Dependencies: AUTO-DOC-005 and AUTO-DOC-007 merged.
- Acceptance: workflow green on its own PR and ordinary CI preserved.
- Tracking issue: #511.

## 4. Paused technical tasks

These tasks remain paused until the maintainer explicitly resumes non-documentation autonomous work.

### AUTO-APP-001 — Validate standalone HTTP Worker GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: preset/build, PID-associated window, bounded liveness, controlled teardown, ordinary CI green.
- Non-goal: no worker security redesign.

### AUTO-APP-002 — Validate standalone main GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Acceptance: `gui-app` preset/build/Xvfb startup evidence before minimal interaction work.

### AUTO-TEST-001 — Remove four historical duplicate Search/Remove test blocks

- Priority: `P2`
- Status: `paused`
- Environment: `local`
- Acceptance: active focused tests remain green; exact inventory updated; ordinary/kernel/smoke paths green.
- Stop: connector-only environments must not replace the large source file blindly.

### AUTO-QT-001 — Remove active Qt5 fallback

- Priority: `P1`
- Status: `paused`
- Environment: `local` preferred
- Acceptance: current active references mapped; Qt6 presets/tests green; no GUI redesign.

### AUTO-PKG-001 — Execute Debian package lifecycle validation

- Priority: `P1`
- Status: `paused`
- Environment: `github` or `local`
- Acceptance: package build, metadata, install, startup, reinstall/upgrade when relevant, uninstall/purge and residual-file checks.
- Non-goal: no PPA publication or package redesign.

## 5. Completed technical baseline

Do not reopen without new evidence:

- CI trigger corrections;
- AI test aggregation;
- Phase 0 kernel/smoke workflow;
- solver contract stabilization;
- active Search/Remove coverage;
- Queue/Station/Delay/Resource lifecycle corrections;
- focused plugin-completion ownership sanitizer;
- optimizer copy/move barrier;
- plugin target/codemodel/link evidence;
- shell, worker, Data Analyser, Optimizer and AI Assistant startup validations.

## 6. Completion rule

A task moves to `done` only after required validation is green, evidence is reviewed, the PR is merged, source branch deletion is confirmed, issue/status/backlog/changelog are updated, and remaining boundaries are explicit.
