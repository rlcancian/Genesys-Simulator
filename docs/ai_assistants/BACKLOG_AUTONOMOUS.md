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

A task may be executed only when its status is `ready`, its required environment is available, all dependencies are resolved, and the repository is not under a maintainer-declared freeze that excludes the task.

During the AI-documentation migration freeze, only tasks whose scope belongs to issue #511 may run.

## 2. Status values

- `ready` — fully specified and eligible for autonomous execution;
- `running` — one active branch/PR owns the task;
- `blocked-review` — implementation prepared but requires maintainer review before consolidation;
- `blocked-dependency` — waits for another task, PR, or issue;
- `paused` — technically executable but temporarily disabled by maintainer instruction;
- `done` — acceptance criteria satisfied and merged;
- `cancelled` — intentionally removed from the plan.

## 3. Required task schema

Every task must define:

- stable ID;
- priority;
- status;
- permitted environment: `local`, `github`, or `either`;
- objective;
- evidence/background;
- exact scope;
- non-goals;
- dependencies and blockers;
- files/areas likely affected;
- validation requirements;
- acceptance criteria;
- stop/escalation conditions;
- tracking issue;
- branch/PR when active.

Agents must not expand scope because adjacent cleanup appears convenient.

## 4. Active documentation migration

### AUTO-DOC-001 — Establish canonical AI documentation governance

- Priority: `P0`
- Status: `running`
- Environment: `github`
- Objective: create the approved canonical documents, runbooks, migration record, and routing entrypoint without moving or deleting existing files.
- Evidence/background: issue #511 and the completed inventory/classification of Markdown under `docs/ai_assistants/`.
- Scope:
  - create `GOVERNANCE.md`, `ARCHITECTURE.md`, `STATUS.md`, both backlogs, and three runbooks;
  - create migration/history index documents;
  - update `README.md` to route future agents;
  - retain every existing document in place.
- Non-goals:
  - no source, CMake, CI workflow, package, runtime, scientific, security, or architecture implementation changes;
  - no file moves, renames, deletions, or broad rewriting of legacy documents;
  - no `oldies/` consolidation or deletion.
- Dependencies: maintainer approval received; repository-changing scheduled tasks paused.
- Validation:
  - inspect all new Markdown links and headings;
  - compare branch against `WorkInProgress`;
  - ordinary CI must be green if triggered;
  - review PR diff before ready-for-review state.
- Acceptance:
  - canonical reading order and authority are explicit;
  - local/GitHub/autonomous runbooks exist;
  - current status has one source;
  - human decisions are separated from executable work;
  - migration matrix covers current document categories;
  - all prior files remain present.
- Stop/escalation:
  - stop if implementation requires deleting/moving content;
  - stop if a proposed policy contradicts an unresolved human decision;
  - stop if another branch changes the same documentation during the freeze.
- Tracking issue: #511
- Branch: `WiP20260722/ai-docs-governance`
- PR: pending

### AUTO-DOC-002 — Consolidate normative governance and architecture sources

- Priority: `P0`
- Status: `blocked-review`
- Environment: `github`
- Objective: after review of AUTO-DOC-001, merge the useful normative content from existing stable guides into the canonical governance/architecture/reference structure.
- Scope:
  - reconcile `branch_workflow.md`, `documentation_governance.md`, decision documents, and durable portions of domain guides;
  - preserve source history through Git;
  - replace old documents with redirects only after content comparison.
- Non-goals:
  - no execution evidence movement in the same PR;
  - no `oldies/` deletion;
  - no technical implementation changes.
- Dependencies: AUTO-DOC-001 merged and explicitly reviewed.
- Validation: link checks, document comparison, no lost active decision, ordinary CI if triggered.
- Acceptance: one normative source per policy and one durable source per architecture boundary.
- Stop/escalation: any unresolved conflict between human decisions must be moved to `BACKLOG_HUMAN.md`.
- Tracking issue: #511 or a child issue created after D0 review.

### AUTO-DOC-003 — Consolidate current status and backlogs

- Priority: `P0`
- Status: `blocked-dependency`
- Environment: `github`
- Objective: retire competing current-state claims after AUTO-DOC-002.
- Scope:
  - reconcile `current_plans.md`, `genesys_2026_test_matrix.md`, and `genesys_2026_consolidation_handoff.md` into `STATUS.md` and the two backlogs;
  - preserve dated snapshots under history when needed;
  - replace retired current-state documents with short migration notices before deletion.
- Dependencies: AUTO-DOC-001 and AUTO-DOC-002.
- Acceptance: no active README points to more than one current-state source.
- Stop/escalation: stop when a pending task lacks enough evidence to classify as autonomous or human-controlled.

### AUTO-DOC-004 — Move immutable evidence and migration records

- Priority: `P1`
- Status: `blocked-dependency`
- Environment: `github`
- Objective: relocate dated evidence and completed migration/integration notes into `history/` without altering their substantive content.
- Scope:
  - move `*_evidence_YYYYMMDD.md` to dated evidence directories;
  - move completed integration/handoff snapshots to migrations/history;
  - update internal links atomically.
- Non-goals: no evidence rewriting; no source/runtime change.
- Dependencies: AUTO-DOC-003 and approved destination map.
- Acceptance: immutable evidence remains reachable; canonical docs no longer enumerate every evidence file.

### AUTO-DOC-005 — Consolidate plugin and scientific-domain references

- Priority: `P1`
- Status: `blocked-dependency`
- Environment: `github`
- Objective: reduce `plugins/*.md` to a smaller reference set without losing domain boundaries.
- Planned destinations:
  - `reference/PLUGINS.md`;
  - `reference/SCIENTIFIC_DOMAINS.md`.
- Dependencies: AUTO-DOC-002 and detailed source comparison.
- Stop/escalation: scientific or architectural conflicts move to the human backlog.

### AUTO-DOC-006 — Add documentation-governance CI

- Priority: `P1`
- Status: `blocked-dependency`
- Environment: `github` or `local`
- Objective: enforce the canonical documentation structure after consolidation.
- Proposed checks:
  - allowlist top-level canonical Markdown;
  - internal link validation;
  - required front matter by document type;
  - no run IDs in governance/architecture documents;
  - dated evidence restricted to `history/evidence/`;
  - unique backlog IDs and required task fields;
  - current `STATUS.md` timestamp;
  - no `oldies/` deletion before gate/waiver.
- Dependencies: AUTO-DOC-001 through AUTO-DOC-005.
- Non-goals: do not enforce rules against legacy files before they are migrated.

### AUTO-DOC-007 — Review and retire `oldies/`

- Priority: `P2`
- Status: `blocked-dependency`
- Environment: `github` or `local`
- Objective: review every retained historical file and remove obsolete files only after the retention gate.
- Dependencies:
  - all historical files individually reviewed;
  - useful content consolidated or explicitly rejected;
  - no active links depend on `oldies/`;
  - date is after 2026-11-01;
  - explicit maintainer approval.
- Acceptance: dedicated deletion PR, preserved Git history/tag, no current-state dependency on oldies.
- Stop/escalation: any uncertain historical decision or unique technical content.

## 5. Paused technical tasks

The following tasks are technically bounded candidates but are paused until the documentation migration is complete and the maintainer resumes autonomous work.

### AUTO-APP-001 — Validate standalone HTTP Worker GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Objective: add bounded Qt6/Xvfb startup evidence for the existing `gui-httpworker` preset.
- Non-goals: no worker API/security redesign; no network exposure change.
- Acceptance: preset/build, PID-associated window, bounded liveness, controlled teardown, ordinary CI green.

### AUTO-APP-002 — Validate standalone main GUI startup

- Priority: `P1`
- Status: `paused`
- Environment: `github`
- Objective: validate `gui-app` configure/build/startup under bounded Xvfb before a later minimal model-interaction workflow.
- Stop/escalation: do not reinterpret GUI GMDD tests as full main-GUI startup coverage.

### AUTO-TEST-001 — Remove historical duplicate Search/Remove test blocks

- Priority: `P2`
- Status: `paused`
- Environment: `local`
- Objective: remove only the four disabled duplicate blocks from the large historical runtime test file.
- Dependencies: active focused Search/Remove executable remains mandatory and green.
- Validation: exact inventory changes from 1,721/1,717/4 to the expected no-duplicate count; ordinary, kernel, and smoke paths green.
- Stop/escalation: connector-only environments must not replace the very large file blindly.

### AUTO-QT-001 — Inventory and remove active Qt5 fallback

- Priority: `P1`
- Status: `paused`
- Environment: `local` preferred
- Objective: implement the already decided Qt6-only policy through a bounded inventory and cleanup sequence.
- Dependencies: documentation migration complete; exact active Qt5 references mapped.
- Non-goals: no GUI redesign or unrelated CMake modernization.
- Acceptance: no active Qt5 fallback outside retained historical files; all Qt6 GUI/test presets green.

### AUTO-PKG-001 — Execute Debian package lifecycle validation

- Priority: `P1`
- Status: `paused`
- Environment: `github` or `local`
- Objective: validate package build, metadata, install, executable startup, reinstall/upgrade behavior where applicable, uninstall, and residual files.
- Non-goals: no PPA publication or package redesign in the validation PR.

## 6. Completed baseline tasks

The following are `done` and must not be reopened without new evidence:

- CI branch/path trigger corrections;
- AI plugin test aggregation;
- Phase 0 kernel/smoke workflow;
- legacy solver contract stabilization;
- active Search/Remove coverage;
- Queue, Station, Delay, and Resource lifecycle corrections;
- focused plugin-completion ASan/LSan correction;
- optimizer copy/move barrier;
- plugin target overlap/introspection/link evidence;
- shell startup validation;
- worker public-health validation;
- Data Analyser GUI startup validation;
- Optimizer GUI startup validation;
- AI Assistant GUI startup validation.

## 7. Task completion rule

A task moves to `done` only after:

1. required validation is green;
2. artifact/evidence is reviewed where required;
3. PR is merged into `WorkInProgress`;
4. source branch is deleted when tooling permits;
5. `STATUS.md`, this backlog, and `history/CHANGELOG_AI.md` are updated consistently.
