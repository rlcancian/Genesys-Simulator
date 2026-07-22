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
- Validation: run `29929616027`, ordinary tests and GUI GMDD green.

### AUTO-DOC-002 — Consolidate normative governance and architecture

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #513
- Merge: `b48697e77d39b25cafc19271ce574bdead60f94d`
- Validation: run `29931594603`, ordinary tests and GUI GMDD green.

### AUTO-DOC-003 — Consolidate current state and plans

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #514
- Merge: `53b49f7518509823fe2265a3f017b5aa76f09d2f`
- Validation: run `29933330431`, ordinary tests and GUI GMDD green.

### AUTO-DOC-004 — Consolidate executed evidence

- Priority: `P1`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #515
- Merge: `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca`
- Validation: run `29934227250`, ordinary tests and GUI GMDD green.

### AUTO-DOC-005 — Consolidate technical references and clean active root

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #516
- Merge: `d375d9e68e5c1dc84e214a772fb15cb05944f0d8`
- Validation: run `29936506990`, ordinary tests and GUI GMDD green.
- Result: six technical references, canonical README routing and removal of superseded active-root guides/plans/redirects.

### AUTO-DOC-007 — Consolidate oldies governance and classify retained history

- Priority: `P0`
- Status: `done`
- Environment: `github`
- Issue/PR: #511 / #517
- Merge: `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5`
- Validation: run `29938004455`, ordinary tests and GUI GMDD green.
- Result:
  - `archive/OLDIES_REVIEW.md` is the only active tracker;
  - all 25 retained files are listed as `retained-review-pending` and not deletion-ready;
  - three superseded top-level trackers were removed;
  - no content file under `oldies/` changed.
- Source branch: removed automatically.

### AUTO-DOC-006 — Add documentation-governance CI

- Priority: `P0`
- Status: `running`
- Environment: `github` or `local`
- Branch: `WiP20260722/ai-docs-ci`
- Objective: enforce the final governed documentation structure.
- Scope:
  - add `scripts/validate-ai-docs.py`;
  - add `.github/workflows/genesys-docs-governance.yml`;
  - validate the exact six-file top-level allowlist;
  - validate active relative links and repository-bound targets;
  - require front matter and core metadata for active AI documentation;
  - enforce unique backlog IDs plus required priority/status fields;
  - prohibit workflow run IDs in normative governance/architecture;
  - restrict dated evidence placement;
  - validate the 25-file oldies tracker and deletion states;
  - reject premature oldies deletion/rename through PR-diff inspection.
- Non-goals:
  - no source/CMake/runtime/package/scientific/security changes;
  - no historical content review or deletion;
  - no third-party Python dependency.
- Acceptance:
  - focused documentation-governance workflow green;
  - ordinary CI and GUI GMDD green;
  - validator usable locally with Python 3;
  - exact final structure enforced;
  - source branch removed after merge.
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
