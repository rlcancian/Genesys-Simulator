---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# Normative Sources Consolidation — 2026-07-22

## 1. Scope

This record documents phase D1 of the AI-assistant documentation migration.

D1 consolidates current branch, promotion, documentation, architecture, maturity, plugin, Qt6, worker, scientific-claim, security, and AI-agent policy into the canonical files:

- `../../GOVERNANCE.md`;
- `../../ARCHITECTURE.md`;
- `../../BACKLOG_HUMAN.md` where a material choice remains open.

No source code, CMake, workflow, package, runtime, scientific algorithm, or security implementation is changed.

## 2. Sources reviewed

The following pre-canonical policy documents were compared:

| Former top-level file | Blob SHA before retirement | Principal content |
|---|---|---|
| `branch_workflow.md` | `1ab6e324592c907aaf9c9941cfda2e353a730b0e` | branch roles, promotion chain, release gate and waiver policy |
| `documentation_governance.md` | `7f633ff551732e3cf8e9c07d47ed73151b4e369c` | documentation placement, oldies retention, issue-report relay security |
| `genesys_2026_human_decisions.md` | `b64efba1392aafa40d1eef9dd575327b06d58401` | architectural alternatives, maturity framework, worker profiles and deferred decisions |
| `genesys_2026_decisions_addendum_20260720.md` | `f0f2ca9387c96c411e89df8d42007bdad7e4ab1b` | later authoritative decisions that superseded earlier alternatives |

Durable policies were also checked against:

- `kernel_development.md`;
- `plugins_development.md`;
- `applications_development.md`;
- `tools_and_statistics.md`;
- `whole_cell_and_sbml.md`;
- `modal_and_hybrid_simulation.md`;
- `python_integration.md`;
- `build_ci_tests.md`.

Those topic guides remain available until D4 reference consolidation.

## 3. Canonical decisions after D1

The following policies are canonical in `GOVERNANCE.md` and `ARCHITECTURE.md`:

- branch roles and conservative promotion chain;
- `20261`/`20262` naming and end-of-semester timing;
- dated working branches using `WiPYYYYMMDD/<scope>`;
- PR review, CI, merge-commit, and source-branch cleanup policy;
- promotion gate and explicit waiver fields;
- Qt6-only intended platform;
- current static plugin graph and deferred stable-C-ABI future boundary;
- Level 3 minimum for supported functionality and separation from scientific claim level;
- controlled-academic-intranet worker direction;
- no write-capable GitHub token in desktop GUI code;
- Doxygen source/final/intermediate placement;
- ownership, validation, evidence, and autonomous stop/escalation rules;
- neuro-symbolic-mechanistic AI virtual-cell research direction without implementation overclaims.

Material choices still open remain in `BACKLOG_HUMAN.md`, including static target consolidation, autoload deployment, worker binding/authentication, reference packages, optimizer scope, and initial AI virtual-cell research package.

## 4. Historical preservation

The four superseded top-level files are retired from the active tree after consolidation. Their exact contents remain preserved through:

- repository commit history;
- the immutable pre-retirement blob SHAs listed above;
- pull-request and issue audit history;
- current backlog entries that preserve unresolved options and decisions.

They are no longer normative and are not part of mandatory reading. Restoring an exact historical source does not require keeping a duplicate file in the active documentation tree.

## 5. Non-claims

D1 does not establish:

- implementation of deferred plugin ABI work;
- completion of Qt5 fallback removal;
- worker security readiness;
- scientific validity;
- release readiness;
- completion of the full documentation migration.

## 6. Follow-up

D2 consolidates current state and active plans into `STATUS.md` and the two backlogs. D3 relocates immutable execution evidence. D4 consolidates topic references. D5 enforces the resulting structure in CI. D6 classifies retained `oldies/` while preserving the post-2026-11-01 deletion gate.
