# GenESyS Branch Workflow Policy

This document records the repository branch promotion policy used by GenESyS.
It is the reference point for assistant workflows, packaging notes, and release
promotion decisions.

## Branch roles

- `master`: public, stable, release-ready branch. Users should consume only this
  branch.
- `currentStable`: second-most stable branch. It is normally identical to
  `master`, except during the short period when a stable semester snapshot is
  being evaluated for merge.
- `2026-1`, `2026-2`, `2027-1`, ...: semester-stable branches. Each one is a
  frozen stable result for that semester.
- `WorkInProgress`: the active development line. New developer branches should
  start here.
- `WiP<semester>`: maintainer-specific working branch, such as `WiP20261`.
  This is a convenience branch, not a replacement for the formal repository
  policy.

## Promotion flow

1. Developer feature branches merge into `WorkInProgress`.
2. At the end of a semester, the stabilized `WorkInProgress` contents are
   promoted into that semester branch, for example `2026-1`.
3. Semester branches can feed `currentStable` when they are judged stable.
4. `master` receives pull requests only from `currentStable`.

That means the promotion chain is intentionally conservative:

```text
feature branches -> WorkInProgress -> semester stable branch -> currentStable -> master
```

## Docker and preset implications

The Docker packaging in this repository intentionally uses `master` as the
default runtime branch for end users. That keeps the user flow aligned with the
public release branch.

The current `WiP20261` worktree contains the packaging refactor and the latest
Docker-aware CMake preset set under validation. Those presets are expected to be
merged to `master` through the normal PR flow, after which the runtime Docker
defaults remain valid without special handling.

When a document or script refers to `WiP20261`, it means the active working
branch that currently carries these Docker and preset changes. It does not
change the formal release policy above.

## Notes for assistants

- Do not rewrite the branch policy casually in other documents; link here
  instead.
- When documenting runtime Docker behavior, distinguish the public default
  branch (`master`) from the active implementation branch (`WiP20261`).
- If a release or packaging workflow needs a temporary override, document the
  override and the reason in the relevant task note.
