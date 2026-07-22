---
document_type: evidence-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# GenESyS Executed Evidence Archive

## 1. Purpose

This directory contains immutable, dated indices of bounded GenESyS validation work.

Evidence supports audit and diagnosis. It does not become current policy or current status merely because it is newer than another document. Use [`../../STATUS.md`](../../STATUS.md) for current operational conclusions.

## 2. Organization

The selected convention is date-first:

```text
history/evidence/
└── YYYY/
    └── MM/
        └── VALIDATION_LEDGER.md
```

Current ledger:

- [`2026/07/VALIDATION_LEDGER.md`](2026/07/VALIDATION_LEDGER.md)

A separate document may be added inside the same month only when its technical detail cannot be represented responsibly in the ledger. It must still be linked from that month's ledger.

## 3. Required evidence metadata

New evidence entries should record, when applicable:

- scope and non-claims;
- repository, branch, head and merge commit;
- PR and issue;
- workflow or local environment;
- toolchain;
- commands or workflow steps;
- test inventory and results;
- artifact name, ID, digest and retention;
- red checkpoint;
- correction/change;
- interpretation;
- remaining risks;
- relationship to current `STATUS.md`.

## 4. Immutability rule

After evidence is finalized and merged:

- do not rewrite the result to match later repository state;
- correct material errors through an explicit addendum or successor ledger entry;
- keep historical failures when they explain a later correction;
- do not reuse an old count as current without checking `STATUS.md` and the current branch;
- preserve detailed former reports through Git history and recorded blob SHAs when consolidating them.

## 5. Scope discipline

Evidence must state what it does **not** prove.

Examples:

- unit tests do not prove unregistered behavior;
- GUI startup does not prove interaction or scientific correctness;
- a focused ASan run does not prove repository-wide leak freedom;
- a loopback request does not prove loopback-only binding;
- successful linking does not prove ABI stability or absence of latent ODR risk;
- package creation does not prove install/upgrade/uninstall lifecycle.

## 6. Current migration state

D3 consolidates the former top-level dated reports into the July 2026 ledger. Temporary redirects remain until D4/D5 update all active links and enforce the canonical root allowlist.
