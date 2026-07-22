---
document_type: archive-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: active-retention
tracks: 511
---

# GenESyS AI Assistant Archive

## Purpose

This directory governs retained historical material that is no longer current but has not passed its review and deletion gate.

The 25 historical Markdown files remain physically under `../oldies/`. Their only active tracker is [`OLDIES_REVIEW.md`](OLDIES_REVIEW.md).

## Authority

Historical material:

- is not current policy, architecture, project status or executable backlog;
- must be revalidated against current source/build evidence before reuse;
- never overrides `../GOVERNANCE.md`, `../ARCHITECTURE.md`, `../STATUS.md`, the canonical backlogs or current code;
- may contain unique analysis, rejected alternatives, stale assumptions or unresolved questions.

## Review states

`OLDIES_REVIEW.md` classifies each retained file using:

- `retained-review-pending`;
- `unique-content-to-consolidate`;
- `consolidated-retained`;
- `obsolete-retained`;
- `discard-after-gate`.

A file is not deletion-ready merely because a current reference exists.

## Retention gate

Do not delete or bulk-move `../oldies/` until all conditions are satisfied:

1. every historical file has been individually inspected;
2. useful content has been consolidated or explicitly rejected;
3. current documentation no longer depends on it;
4. the date is after 2026-11-01;
5. the maintainer explicitly approves deletion;
6. a dedicated deletion PR preserves a clear Git-history reference or archival tag/branch.

D6 consolidates governance only. It does not satisfy the individual-review or deletion gate.

## Other historical destinations

- executed evidence: `../history/evidence/`;
- completed migration records: `../history/migrations/`;
- concise change index: `../history/CHANGELOG_AI.md`.

Ordinary workflow artifacts and current handoffs do not belong in this archive.
