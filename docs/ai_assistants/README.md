---
document_type: entrypoint
authority: routing
owner: project-maintainer
last_reviewed: 2026-07-22
status: active-during-migration
tracks: 511
---

# GenESyS AI Assistant Documentation

This directory is the mandatory entrypoint for AI-assisted technical work in the GenESyS Simulator.

Before changing code, CMake, tests, CI, packaging, documentation, plugins, applications, numerical/statistical algorithms, or scientific behavior, read the required documents below.

## 1. Minimum mandatory reading

Every assistant must read:

1. repository [`/README.md`](../../README.md);
2. this file;
3. [`GOVERNANCE.md`](GOVERNANCE.md);
4. [`STATUS.md`](STATUS.md).

Read [`ARCHITECTURE.md`](ARCHITECTURE.md) whenever the task affects module boundaries, ownership, public APIs, plugins, applications, numerical/scientific behavior, persistence, security, or maturity claims.

## 2. Select work from the correct source

### Autonomous executable work

Read:

- [`BACKLOG_AUTONOMOUS.md`](BACKLOG_AUTONOMOUS.md);
- [`runbooks/AUTONOMOUS_AGENT.md`](runbooks/AUTONOMOUS_AGENT.md);
- the runbook for the available environment.

An agent may execute only a task with status `ready` whose blockers, environment, scope, acceptance criteria, and stop gates are satisfied.

### Human decisions

Read:

- [`BACKLOG_HUMAN.md`](BACKLOG_HUMAN.md).

Do not infer or choose architectural, scientific, security, release, or product decisions that are recorded there.

## 3. Environment runbooks

- [`runbooks/LOCAL_AGENT.md`](runbooks/LOCAL_AGENT.md): local checkout, Git/gh, CMake, Ninja, CTest, applications, sanitizers, packages, and large-file edits.
- [`runbooks/GITHUB_AGENT.md`](runbooks/GITHUB_AGENT.md): GitHub connector/API, PRs, issues, Actions, artifacts, and remote-only evidence limits.

Use the local runbook only when a real local checkout and executable tools are available. A GitHub-only agent must not claim local build/test execution.

## 4. Document authority

Use this precedence when documents conflict:

1. [`GOVERNANCE.md`](GOVERNANCE.md);
2. [`ARCHITECTURE.md`](ARCHITECTURE.md);
3. [`STATUS.md`](STATUS.md);
4. the two backlogs;
5. the applicable runbook;
6. active topic-specific references/guides;
7. dated evidence and migration history;
8. temporary historical material under `oldies/` or `archive/`.

Later evidence supersedes an older status claim only for the exact commit, environment, and scope it records.

## 5. Current canonical documents

| Document | Purpose | Expected volatility |
|---|---|---|
| [`GOVERNANCE.md`](GOVERNANCE.md) | normative development, evidence, branch, validation, maturity, security, and documentation rules | low |
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | durable repository architecture and technical boundaries | low/medium |
| [`STATUS.md`](STATUS.md) | single current operational state, validated baseline, blockers, and next eligible work | high |
| [`BACKLOG_AUTONOMOUS.md`](BACKLOG_AUTONOMOUS.md) | tasks agents may execute without a new material decision | high |
| [`BACKLOG_HUMAN.md`](BACKLOG_HUMAN.md) | decisions agents must not make by assumption | medium |

## 6. Supporting structure

- [`runbooks/`](runbooks/): execution procedures by agent/environment.
- [`reference/`](reference/README.md): planned topic-specific technical references.
- [`history/CHANGELOG_AI.md`](history/CHANGELOG_AI.md): concise index of material AI-assisted changes.
- [`history/evidence/`](history/evidence/README.md): planned immutable evidence archive.
- [`history/migrations/`](history/migrations/ai_docs_governance_migration_20260722.md): completed/in-progress migration records.
- [`archive/`](archive/README.md): temporary non-current retained material.

## 7. Documentation migration status

Issue #511 is executing migration phase D0.

D0 is additive except for this routing update:

- canonical documents and runbooks are being introduced;
- all pre-existing Markdown files remain in their current locations;
- no legacy file is moved, renamed, rewritten, or deleted in D0;
- effective consolidation occurs only after maintainer review;
- the existing `oldies/` deletion gate after 2026-11-01 remains in force.

The full classification, destination map, phases, validation, and rollback are recorded in:

- [`history/migrations/ai_docs_governance_migration_20260722.md`](history/migrations/ai_docs_governance_migration_20260722.md).

## 8. Legacy documents during migration

The following current locations remain available as source material until later phases reconcile them:

- build/CI/test guidance: `build_ci_tests.md`;
- kernel guidance: `kernel_development.md`;
- plugin guidance: `plugins_development.md` and `plugins/`;
- application guidance: `applications_development.md`;
- tools/statistics guidance: `tools_and_statistics.md`;
- modal/hybrid guidance: `modal_and_hybrid_simulation.md`;
- whole-cell/SBML guidance: `whole_cell_and_sbml.md`;
- Python guidance: `python_integration.md`;
- model-generation guidance: `models_and_modelspecific_generation.md`;
- Docker guidance: `docker_packaging.md`;
- current/historical plans, handoffs, inventories, decision documents, and dated evidence;
- temporary historical material under `oldies/`.

During D0, use these documents for topic detail, but route policy through `GOVERNANCE.md`, durable boundaries through `ARCHITECTURE.md`, and current state through `STATUS.md`.

## 9. Core rules

1. Inspect current repository files before technical claims.
2. Distinguish confirmed facts, executed evidence, strong indication, hypothesis, and history.
3. Prefer small, reversible, reviewable changes.
4. Do not invent files, targets, classes, methods, APIs, commands, workflows, results, or ownership.
5. Use the real CMake/Ninja/CTest/Qt6 project baseline.
6. Do not merge while mandatory validation is red or incomplete.
7. Do not claim scientific validity from build/startup/test success alone.
8. Stop and escalate when a human decision controls the implementation.
9. Do not delete historical material before consolidation and its retention gate.
10. Update `STATUS.md`, the applicable backlog, and the AI changelog after material merged work.
