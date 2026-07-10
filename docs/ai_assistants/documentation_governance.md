# Documentation Governance Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS documentation governance, historical retention, and safe documentation updates.

Historical notes from `oldies/` must be checked before being treated as current policy.

## Scope

Primary documentation areas:

- repository `README.md`;
- `docs/ManualGenESyS.pdf`;
- `docs/users/`;
- `docs/developers/`;
- `docs/ai_assistants/`;
- `docs/ai_assistants/branch_workflow.md`;
- temporary historical material under `docs/ai_assistants/oldies/`.

Historical source documents:

- `old_report-issue-relay-contract.md`
- `old_post-refactor-validation-report.md`
- relevant sections of `old_genesys_wiki_consolidated.md`

## Current documentation policy

- Keep top-level documentation under `docs/`.
- Keep AI-assistant operational guidance under `docs/ai_assistants/`.
- Keep repository branch/versioning policy in `docs/ai_assistants/branch_workflow.md`.
- Keep user-facing Doxygen entrypoints and final PDFs under `docs/users/`.
- Keep developer-facing Doxygen entrypoints and final PDFs under `docs/developers/`.
- Do not version Doxygen intermediate output trees.
- Preserve historical material in `oldies/` until consolidation is complete.

## Oldies retention policy

`docs/ai_assistants/oldies/` is temporary.

Do not delete it until:

1. every historical file has been reviewed;
2. useful content has been moved to stable documents or explicitly marked obsolete;
3. stable documents avoid stale branch/date assumptions unless clearly identified as historical;
4. no active README or plan points to `oldies/` as the primary source of current guidance.

The planned deletion gate remains after 2026-11-01, subject to the review conditions above.

## Update rules for AI assistants

When updating documentation:

- inspect real repository files before making technical claims;
- cite or reference the source files used for current facts;
- separate current facts from historical notes;
- avoid broad rewrites when a small update is enough;
- do not remove historical material without a clear consolidation record;
- keep generated artifacts out of source documentation unless explicitly approved.

## Issue-report relay note

Historical documentation records a GUI issue-report relay contract. The important governance point is that desktop GUI code must not embed a GitHub token with write access. Any issue-report workflow should go through a maintainer-operated relay or another safe server-side mechanism.

## Open follow-up tasks

- Revalidate whether the issue-report relay contract is implemented, planned, or obsolete.
- Mark each `oldies/` file as consolidated, still pending, or discarded.
- Update `oldies_inventory.md` as consolidation progresses.
- After the deletion date and review gate, remove obsolete historical files in a dedicated commit.
