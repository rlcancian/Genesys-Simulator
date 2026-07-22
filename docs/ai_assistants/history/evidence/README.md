---
document_type: evidence-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: migration-placeholder
tracks: 511
---

# GenESyS Executed Evidence Archive

## Purpose

This directory is the planned destination for immutable, dated evidence produced by bounded GenESyS validation work.

Evidence records support audit and diagnosis. They do not become current policy or current status merely because they are newer than another document.

Use `../../STATUS.md` for current operational conclusions.

## Planned organization

```text
history/evidence/
├── 2026/
│   └── 07/
├── applications/
├── ownership/
├── plugins/
├── runtime/
└── scientific/
```

The final organization may prefer date-first or domain-first directories, but one convention must be selected before D3 moves existing files.

## Required evidence metadata

New evidence documents should record:

- scope and non-claims;
- repository, branch, head/merge commit;
- PR and issue;
- workflow/local environment;
- toolchain;
- exact commands or workflow steps;
- test inventory and results;
- artifact name, ID, digest, and retention when applicable;
- red checkpoint where relevant;
- correction or change;
- interpretation;
- remaining risks;
- relationship to current `STATUS.md`.

## Immutability rule

After evidence is finalized and its PR merged:

- do not rewrite the result to match later repository state;
- correct material errors through an explicit addendum or successor evidence record;
- keep historical failures when they explain a later correction;
- do not reuse an old count or result as current without checking `STATUS.md` and the current branch.

## Scope discipline

Evidence must state what it does **not** prove.

Examples:

- unit tests do not prove unregistered behavior;
- GUI startup does not prove interaction or scientific correctness;
- a focused ASan run does not prove repository-wide leak freedom;
- a loopback request does not prove loopback-only binding;
- successful linking does not prove ABI stability or absence of latent ODR risk;
- package creation does not prove install/upgrade/uninstall lifecycle.

## Migration note

During D0, existing top-level `*_evidence_YYYYMMDD.md` files remain in place. They will be moved only in D3 after the destination convention is reviewed and links can be updated atomically.
