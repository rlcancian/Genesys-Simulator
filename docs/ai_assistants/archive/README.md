---
document_type: archive-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: migration-placeholder
tracks: 511
---

# GenESyS AI Assistant Archive

## Purpose

This directory is reserved for temporary retained material that is no longer current but has not yet passed its consolidation and deletion gate.

During migration phase D0, no existing file is moved here. The current `../oldies/` directory remains in place and retains its existing protection rules.

## Archive authority

Archived material:

- is not current policy;
- is not current architecture;
- is not current project status;
- is not an executable backlog;
- must be revalidated against current code before reuse;
- never overrides `GOVERNANCE.md`, `ARCHITECTURE.md`, `STATUS.md`, the active backlogs, or current source/build evidence.

## Planned historical-review flow

Each historical file must be classified as:

- `pending-review`;
- `unique-content-to-consolidate`;
- `consolidated`;
- `obsolete`;
- `discard-after-gate`.

The review record must identify:

- source file;
- original date/branch/commit when known;
- topic;
- current replacement/reference;
- unique content retained;
- stale or rejected claims;
- reviewer/date;
- deletion readiness.

## `oldies/` retention gate

Do not delete or bulk move `../oldies/` until:

1. every historical file has been inspected;
2. useful content has been consolidated or explicitly rejected;
3. current documents no longer depend on it;
4. the date is after 2026-11-01;
5. the maintainer explicitly approves deletion;
6. a dedicated deletion PR preserves a clear Git-history reference or archival tag/branch.

## Evidence is not archive clutter

Executed evidence with durable audit value belongs under `../history/evidence/`, not in this archive. Completed branch/document migrations belong under `../history/migrations/`.

The archive is for unresolved historical retention, not for ordinary workflow artifacts or current handoffs.
