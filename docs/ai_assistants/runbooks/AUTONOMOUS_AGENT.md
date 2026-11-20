---
document_type: runbook
authority: operational
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# Runbook: Autonomous GenESyS Agent

## 1. Purpose

This runbook defines the common execution loop for an AI agent that may select and complete pre-approved GenESyS tasks without interactive human guidance.

It applies only to tasks in `../BACKLOG_AUTONOMOUS.md` with status `ready`. It does not authorize the agent to make scientific, architectural, security, release, or product decisions.

## 2. Required startup sequence

Before selecting work:

1. read repository `/README.md`;
2. read `docs/ai_assistants/README.md`;
3. read `../GOVERNANCE.md`;
4. read `../STATUS.md`;
5. read `../BACKLOG_AUTONOMOUS.md`;
6. read this runbook;
7. read either `LOCAL_AGENT.md` or `GITHUB_AGENT.md` according to the execution environment;
8. read `../ARCHITECTURE.md` and topic-specific references required by the task.

Do not reuse remembered repository state without revalidation.

## 3. Task selection

Select the first highest-priority task that satisfies all of:

- status is exactly `ready`;
- environment matches available capabilities;
- no dependency or blocker is unresolved;
- no other active branch/PR owns the task;
- no maintainer freeze excludes the task;
- scope and acceptance criteria are objective;
- the task has an explicit stop/escalation boundary.

Do not start a `paused`, `blocked-review`, `blocked-dependency`, or human-backlog task.

## 4. Claiming a task

Before changing files:

1. verify the tracking issue or create one if the task explicitly permits it;
2. update the backlog entry to `running` with branch and issue;
3. create a dated bounded branch using the approved naming convention;
4. confirm the branch starts from current `WorkInProgress`;
5. inspect open PRs and recent merged changes for overlap;
6. record the current base commit.

Only one branch/PR may own one backlog ID at a time.

## 5. Diagnostic sequence

For technical work:

1. inspect the real source/build/test files;
2. map immediate callers, dependencies, ownership, and persistence impact;
3. reproduce the defect or missing evidence;
4. add a failing focused regression or evidence-only checkpoint where appropriate;
5. distinguish confirmed fact, strong indication, hypothesis, and historical evidence;
6. choose the smallest correction that satisfies the task.

For documentation work:

1. inspect every source document being consolidated;
2. separate policy, architecture, status, backlog, evidence, and history;
3. preserve unique technical content;
4. do not convert historical claims into current facts without current-code validation;
5. update links atomically with moves or replacements.

## 6. Change constraints

The agent must:

- preserve semantics unless the task demonstrates a defect or approved contract change;
- avoid unrelated formatting and modernization;
- use small commits with one clear concern;
- avoid broad file replacement when a narrow edit is technically safer;
- preserve uncommitted local user changes in local environments;
- not modify stable branches directly;
- not silently expand acceptance criteria.

## 7. Validation selection

Use the task's required validation plus the relevant policy from `GOVERNANCE.md`.

Minimum expectations:

- documentation: path/link/content review and ordinary CI when triggered;
- C++: configure, build, focused tests, ordinary tests;
- kernel/parser/plugin/tool: kernel-focused and smoke paths when applicable;
- GUI: affected preset and bounded startup/interaction scope;
- ownership: focused sanitizer where appropriate;
- packaging: lifecycle validation separate from unit tests;
- scientific methods: declared oracle/reference and reproducibility.

If required validation cannot be executed in the current environment, do not substitute inspection and claim success. Use an approved CI workflow or stop/escalate.

## 8. Pull-request lifecycle

Open a draft PR containing:

- backlog ID and tracking issue;
- exact scope;
- diagnosis/evidence;
- files changed;
- non-goals;
- validation performed and pending;
- impact and compatibility;
- remaining risks;
- rollback/reversal path.

Keep the PR draft until:

- final-head required checks are green;
- artifacts are reviewed where required;
- no unresolved review thread remains;
- the diff matches the backlog scope;
- `STATUS.md`, backlogs, and changelog updates are prepared as appropriate.

Merge only through the repository-supported merge method and only into the approved base.

## 9. Completion

After successful merge:

1. verify the merge commit exists in `WorkInProgress`;
2. delete the source branch when tooling permits;
3. close the tracking issue with evidence;
4. update the task to `done`;
5. update `STATUS.md` if current state changed;
6. append one concise row to `history/CHANGELOG_AI.md`;
7. link immutable evidence rather than duplicating full logs;
8. identify the next eligible backlog item, but do not start it if a freeze or review gate applies.

## 10. Mandatory stop conditions

Stop and move the task to the appropriate blocked/human state when:

- source behavior differs materially from task assumptions;
- implementation requires choosing among architectures;
- scientific formulation, reference, data, tolerance, or claim level is missing;
- security posture, network exposure, authentication, or secret behavior would change;
- public API/ABI/persistence compatibility cannot be preserved without a decision;
- the supported feature set or release gate would change;
- tests expose a broader defect outside the bounded scope;
- required validation remains red;
- another branch changes the same files during the task;
- the task would require deletion before an approved retention gate.

When stopping, report:

- exact blocker;
- evidence;
- affected files/components;
- available options;
- recommendation;
- work already completed;
- what decision or prerequisite would resume execution.
