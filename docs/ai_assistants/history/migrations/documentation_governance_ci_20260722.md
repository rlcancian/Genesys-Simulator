---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# AI Documentation Governance CI — 2026-07-22

## 1. Scope

This record documents migration phase D5.

D5 adds automated structural validation for the governed AI-assistant documentation after D0–D4 and D6 established the final layout.

## 2. Validator

The repository-local validator is:

```text
scripts/validate-ai-docs.py
```

It uses only the Python standard library and can run locally or in GitHub Actions.

Checks include:

- exact six-file top-level Markdown allowlist;
- required canonical, runbook, reference, history and archive paths;
- YAML front matter and required metadata for active AI documentation;
- relative Markdown links, repository-bound resolution and prohibition on active dependencies on `oldies/`;
- unique autonomous/human backlog IDs and required priority/status fields;
- prohibition on 11-digit workflow run IDs in normative governance/architecture;
- dated evidence placement under `history/evidence/`;
- exact 25-file `oldies/` inventory agreement with `archive/OLDIES_REVIEW.md`;
- all retained entries remaining not deletion-ready;
- PR-diff rejection of oldies deletion/rename before the retention gate unless an explicit environment waiver is supplied.

## 3. Workflow

The focused workflow is:

```text
.github/workflows/genesys-docs-governance.yml
```

It runs for relevant pull-request changes against active development/stable branches and supports manual dispatch.

The workflow checks out full Git history so the validator can inspect oldies deletion/rename operations against the PR base commit.

## 4. Security and authority boundary

The workflow has read-only repository permissions.

The optional environment variable `GENESYS_ALLOW_OLDIES_DELETION=1` only bypasses the date-based script failure; it does not replace the required maintainer approval, detailed review, dedicated PR or branch-protection review process.

## 5. Non-changes

D5 does not change:

- C++ source, CMake targets or runtime behavior;
- packaging or deployment behavior;
- scientific, numerical or statistical semantics;
- security architecture;
- the contents or classifications of retained historical files;
- release or semester-branch promotion state.

## 6. Completion gate

The full documentation migration may be declared structurally complete only after:

1. the focused documentation-governance workflow passes;
2. ordinary CI and GUI GMDD diagnostics pass;
3. the D5 PR is merged into `WorkInProgress`;
4. its source branch is removed;
5. issue #511 is updated and closed;
6. `STATUS.md`, backlogs and changelog record completion.
