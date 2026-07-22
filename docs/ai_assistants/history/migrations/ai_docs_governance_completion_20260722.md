---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# AI-Assistant Documentation Governance Completion — 2026-07-22

## 1. Scope

This record closes the structural migration and governance program tracked by issue #511.

The program reorganized `docs/ai_assistants/` into one governed entrypoint, six canonical top-level documents, task-specific runbooks and references, immutable history, a retained historical archive tracker and automated structural enforcement.

## 2. Integrated phases

| Phase | Pull request | Merge commit | Principal result |
|---|---:|---|---|
| D0 | #512 | `958cdc6f63c02d004f1ffdf55e104b58a245bb88` | canonical documents and runbooks |
| D1 | #513 | `b48697e77d39b25cafc19271ce574bdead60f94d` | normative policy consolidation |
| D2 | #514 | `53b49f7518509823fe2265a3f017b5aa76f09d2f` | sole current status and backlogs |
| D3 | #515 | `ca910a2fbe4504ef8520ef48b8b377da7e9e02ca` | date-first executed-evidence ledger |
| D4 | #516 | `d375d9e68e5c1dc84e214a772fb15cb05944f0d8` | six technical references and root cleanup |
| D6 | #517 | `c9c76c3d62633b69a7d18d899aa764b7ebdf69a5` | one tracker for 25 retained historical files |
| D5 | #518 | `610d8ab21c87cfd11663af78370b39262cf4da81` | local and GitHub Actions governance enforcement |

All phase source branches were removed automatically after merge.

## 3. Final top-level allowlist

The only Markdown files directly under `docs/ai_assistants/` are:

```text
README.md
GOVERNANCE.md
ARCHITECTURE.md
STATUS.md
BACKLOG_AUTONOMOUS.md
BACKLOG_HUMAN.md
```

Supporting content is organized under:

- `runbooks/`;
- `reference/`;
- `history/`;
- `archive/`;
- retained non-authoritative `oldies/`.

## 4. Enforcement

The final structure is checked by:

- `scripts/validate-ai-docs.py`;
- `.github/workflows/genesys-docs-governance.yml`.

The first focused enforcement run was `29938886903` and passed. The corresponding ordinary CI run was `29938886807`; configure, build, CTest and GUI GMDD diagnostics all passed.

## 5. Historical retention

The 25 files under `oldies/` remain preserved. `archive/OLDIES_REVIEW.md` is their only active tracker.

Every file is currently:

- `retained-review-pending`;
- non-authoritative;
- not deletion-ready.

Deletion remains prohibited before 2026-11-01 and also requires individual review, explicit maintainer approval, dedicated PR and preserved provenance.

## 6. Non-claims

Completion of this documentation program does not establish:

- release readiness;
- package lifecycle readiness;
- security readiness;
- repository-wide memory safety;
- scientific validity;
- maturity of experimental applications, optimization or whole-cell functionality;
- resolution of human decisions in `BACKLOG_HUMAN.md`.

## 7. Operational transition

The migration-specific freeze ends after this record is merged and issue #511 is closed.

Technical tasks previously marked `paused` remain paused. They require explicit maintainer activation and retain their own scope, validation and stop gates.
