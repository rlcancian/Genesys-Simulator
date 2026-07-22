---
document_type: governance
authority: normative
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: 90d
status: active
tracks: 511
---

# GenESyS AI-Assisted Development Governance

## 1. Purpose

This document is the normative source for AI-assisted development in the GenESyS repository. It defines authority, evidence, branch, review, validation, documentation, maturity, security, and escalation rules.

It does not replace source-code inspection. Current code, generated build graphs, executable tests, Git history, pull requests, issues, workflow runs, and artifacts remain the evidence used to establish implementation facts.

## 2. Mandatory reading order

Every AI assistant must read, in this order:

1. repository `/README.md`;
2. `docs/ai_assistants/README.md`;
3. this file;
4. `ARCHITECTURE.md` when changing architecture, module boundaries, ownership, plugins, applications, numerical/scientific behavior, or public interfaces;
5. `STATUS.md` for current repository state;
6. the applicable backlog and environment runbook;
7. topic-specific reference documents required by the selected task.

Historical and evidence documents are not part of the default reading set unless the current task requires them.

## 3. Document authority and precedence

When documents conflict, use this order:

1. `GOVERNANCE.md` — normative policy;
2. `ARCHITECTURE.md` — approved architecture and durable boundaries;
3. `STATUS.md` — current operational state and validated checkpoints;
4. `BACKLOG_AUTONOMOUS.md` and `BACKLOG_HUMAN.md` — approved pending work;
5. applicable file under `runbooks/`;
6. active topic-specific guides and references;
7. dated evidence and migration records under `history/`;
8. temporary historical material under `oldies/` or `archive/`.

A later dated evidence record supersedes an older status claim only for the commit, environment, and validation scope explicitly recorded by that evidence.

Historical documents never override current policy or current source code.

## 4. Evidence discipline

Every technical statement must be classified as one of:

- **confirmed in current code/build** — directly inspected in the current branch or generated build graph;
- **confirmed by executed evidence** — demonstrated by an identified test, workflow, artifact, or local command;
- **strong indication** — supported by multiple observations but not yet executed or exhaustively proven;
- **hypothesis to validate** — plausible and explicitly unconfirmed;
- **historical evidence** — true only for an older branch, commit, PR, report, or environment.

Assistants must not:

- invent files, classes, methods, targets, options, workflows, APIs, dependencies, results, or branch state;
- convert a PR description into proof that code exists in the current branch;
- claim that configuration success proves compilation, that compilation proves tests, or that tests prove unregistered paths;
- claim scientific validity based only on successful build, startup, unit tests, or plausible-looking output;
- claim local execution when operating only through GitHub or another remote connector.

## 5. Change policy

Changes must be:

- small;
- bounded to one primary concern;
- reversible;
- reviewable;
- supported by evidence;
- validated at the narrowest sufficient level and then at the required regression level.

Do not combine unrelated concerns such as:

- ownership correction and broad modernization;
- static plugin cleanup and dynamic-plugin migration;
- GUI relocation and numerical behavior changes;
- worker startup validation and security redesign;
- scientific algorithm changes and documentation-only migration.

Preserve behavior unless a defect or intentionally changed contract is demonstrated.

## 6. Branch and pull-request policy

The formal promotion flow remains:

```text
feature branches -> WorkInProgress -> YYYYs -> currentStable -> master
```

For the current semester:

```text
feature branches -> WorkInProgress -> 20262 -> currentStable -> master
```

`20262` is reserved for end-of-semester promotion and must not be populated by ordinary development PRs.

For bounded AI-assisted work, use a dated descriptive branch, currently following:

```text
WiPYYYYMMDD/<short-scope>
```

Example:

```text
WiP20260722/ai-docs-governance
```

Each pull request must:

- target `WorkInProgress` unless an explicit human instruction selects another base;
- describe scope, non-goals, impact, validation, and remaining risks;
- remain draft until required checks and artifacts are reviewed;
- avoid merge while mandatory validation is red, incomplete, or ambiguous;
- use merge commits because repository squash and rebase merge are disabled;
- delete its source branch after successful merge when repository tooling permits it.

Never promote a stable branch implicitly.

## 7. Validation policy

Select validation according to impact.

### Documentation-only changes

- inspect rendered Markdown structure and links;
- verify referenced paths and identifiers;
- run ordinary CI when repository path filters trigger it;
- do not claim source/runtime validation from a documentation-only PR.

### C++/kernel/parser/plugin/tool changes

- configure with CMake preset;
- build with Ninja;
- run focused tests;
- run ordinary `tests-unit` regression;
- run `tests-kernel-unit` when kernel/parser/plugin/tool behavior is affected;
- run `tests-smoke` when runtime, continuous, or startup behavior is affected.

### GUI changes

- preserve ordinary unit/GUI diagnostics;
- build the affected Qt6 preset;
- validate bounded startup when applicable;
- distinguish startup evidence from interaction and functional correctness.

### Ownership, lifetime, undefined behavior, or resource changes

- add a focused regression first;
- use ASan/LSan/UBSan or another validated diagnostic path when technically appropriate;
- record exact scope; a focused sanitizer does not prove repository-wide leak freedom.

### Packaging and deployment changes

Validate build, package creation, install, startup, upgrade/reinstall when relevant, uninstall, files left behind, version metadata, dependencies, and service/runtime assumptions separately from unit tests.

## 8. Autonomous execution boundary

An autonomous agent may execute a task only when all of the following are true:

- the task appears in `BACKLOG_AUTONOMOUS.md` with status `ready`;
- scope and non-goals are explicit;
- the required environment is available;
- acceptance criteria are executable and objective;
- no unresolved human decision controls the implementation;
- stop/escalation conditions are defined;
- the task does not silently alter scientific meaning, security posture, release scope, public compatibility, or product claims.

The agent must stop and move the task to `blocked` when it discovers a decision boundary not represented in the task.

## 9. Human-decision boundary

Use `BACKLOG_HUMAN.md` when progress depends on:

- architectural choice among materially different designs;
- scientific formulation, bibliography, dataset, parameterization, or claim level;
- security profile, authentication, trust boundary, secret handling, or network exposure;
- release scope, maturity acceptance, waiver, or branch promotion;
- product behavior or supported-feature definition;
- destructive historical-document deletion before its retention gate.

Assistants must present evidence, options, tradeoffs, recommendation, and the exact implementation work unlocked by the decision. They must not choose by assumption.

## 10. Ownership and C++ modernization policy

Before changing a pointer, reference, container, destructor, copy/move operation, or smart-pointer type:

1. map ownership and observing relationships;
2. inspect construction, adoption, replacement, teardown, callbacks, and exceptional exits;
3. identify ABI/API and persistence impact;
4. add focused tests or diagnostic evidence;
5. apply the smallest correction.

Modern C++ is used for technical benefit, not cosmetic replacement. Prefer incremental use of `nullptr`, `override`, RAII, Rule of Zero, deleted unsafe operations, `std::unique_ptr`, `noexcept`, `constexpr`, `[[nodiscard]]`, ranges, and newer library types only when compatible with the real toolchain and existing semantics.

## 11. Plugin governance

During baseline consolidation:

- plugins remain in the current static build graph;
- no broad dynamic migration is authorized;
- overlapping static target/source composition must be resolved before dynamic work;
- registration, factories, metadata, persistence, dependencies, ownership, and lifecycle must remain explicit.

The approved future in-process dynamic boundary is a stable C ABI with opaque handles, explicit create/destroy operations, versioned function/capability tables, structured errors, and no STL, Qt types, C++ implementation classes, or C++ exceptions crossing the boundary.

Implementation remains deferred until its prerequisite architecture tasks and human decisions are resolved.

## 12. Software maturity and scientific claims

Supported software functionality must reach at least **Level 3 — Beta** before inclusion in a semester-stable supported set. Prioritized functionality may later reach **Level 4 — Stable user feature**.

Software maturity and scientific claim level are independent.

A passing Level 3 software workflow may still be only:

- educational/demonstrative;
- a mechanistic research prototype;
- quantitatively validated for a declared domain;
- or predictive only after substantially stronger independent validation.

Numerical, statistical, biochemical, optimization, and biological claims require declared formulations, domains, references, fixtures, tolerances, reproducibility, limitations, and provenance.

## 13. Security governance

Network, secret, generated-code, external-process, AI-provider, and plugin-loading changes are security-sensitive.

Current worker direction is a controlled academic intranet, not direct public-Internet exposure. Security implementation requires explicit decisions and validation for bind addresses, authentication, TLS, credentials, quotas, isolation, audit logging, denial behavior, and resource limits.

No token or write-capable GitHub credential may be embedded in desktop GUI code.

## 14. Documentation governance

Canonical top-level AI-assistant documents are:

- `README.md`;
- `GOVERNANCE.md`;
- `ARCHITECTURE.md`;
- `STATUS.md`;
- `BACKLOG_AUTONOMOUS.md`;
- `BACKLOG_HUMAN.md`.

During migration, existing documents remain available but must not be treated as competing current-state sources.

Dated execution records belong under `history/evidence/`. Migration records belong under `history/migrations/`. Temporary historical documents remain under `oldies/` or `archive/` until reviewed.

Do not delete `oldies/` before:

1. every file has been reviewed;
2. useful content has been consolidated or explicitly rejected;
3. active links no longer depend on it;
4. the existing post-2026-11-01 retention gate is satisfied;
5. deletion is approved and performed in a dedicated PR.

## 15. Required reporting after work

Every completed task or PR must record:

- scope analyzed;
- diagnosis;
- evidence;
- impact;
- solution;
- patch/change summary;
- validation;
- remaining risks;
- next step;
- branch, PR, merge commit, workflow run, and artifact identifiers when applicable.
