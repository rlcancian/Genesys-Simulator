---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# Historical `oldies/` Governance Consolidation — 2026-07-22

## 1. Scope

This record documents migration phase D6.

D6 consolidates the governance and inventory of historical AI-assistant Markdown retained under `docs/ai_assistants/oldies/`.

It does not delete, rewrite, relocate, validate or endorse any of the 25 retained historical content files.

## 2. Active tracker

The only active tracker after D6 is:

```text
docs/ai_assistants/archive/OLDIES_REVIEW.md
```

It lists all 25 retained files, maps each one to current canonical destinations and classifies every entry conservatively as `retained-review-pending`.

No entry is deletion-ready.

## 3. Superseded trackers

The following top-level trackers are removed:

- `oldies_inventory.md` — former blob `cb5d81c9cd41da79c443c352f62f0d5898cca1d8`;
- `oldies_review_status.md` — former blob `591a58dd0b57c6982ce9bfd0aa06ecf2cb091e81`;
- `consolidation_map.md` — former blob `201877a22c64d6aafffc4fb2355169b4f39cec81`.

Their exact contents remain recoverable through Git history and the recorded blob SHAs.

## 4. Retention and deletion gate

Deletion remains prohibited unless all conditions are met:

1. individual detailed review;
2. useful content consolidated or explicitly rejected;
3. no active documentation dependency;
4. date after 2026-11-01;
5. explicit maintainer approval;
6. dedicated deletion PR;
7. Git-history or archival provenance recorded;
8. documentation-governance CI green.

The existence of a canonical reference does not satisfy individual review.

## 5. Structural result

After D6:

- the top-level AI-assistant directory is intended to contain only the six canonical Markdown files;
- references are under `reference/`;
- executed evidence is under `history/evidence/`;
- migration records are under `history/migrations/`;
- the historical review tracker is under `archive/`;
- the 25 retained content files remain under `oldies/`.

## 6. Non-claims

D6 does not establish:

- correctness of any historical technical claim;
- completion of individual oldies review;
- permission to delete historical content;
- current implementation of historical plans;
- software, security, release or scientific readiness.

## 7. Follow-up

D5 adds automated enforcement for the final documentation structure, links, front matter, backlog IDs, evidence placement and oldies retention gate.
