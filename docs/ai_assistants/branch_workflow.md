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

## Promotion gate terminology

A promotion gate is the documented go/no-go checklist that must be satisfied before moving code to a more stable branch. It is not a single test. It combines build/test evidence, unresolved-risk review, application/package readiness, documentation, and explicit approval.

A mandatory gate failure blocks promotion unless an authorized waiver records:

- the failed criterion;
- evidence and impact;
- containment or disabled scope;
- responsible owner;
- expiration/review condition;
- explicit human approval.

Possible gate levels:

### Minimal gate

- canonical configure/build succeeds;
- primary unit tests pass;
- no known P0 blocker;
- known limitations are documented.

Use only for an internal snapshot, not a strong stable-release claim.

### Standard gate

- clean build on the supported Ubuntu/Qt6 baseline;
- required unit and smoke tests pass;
- supported applications build and have startup/smoke checks;
- no open P0;
- P1 issues are resolved or explicitly waived with rationale and containment;
- representative model compatibility fixtures pass;
- required packages build and pass basic install/run checks;
- network-facing behavior matches the approved security profile;
- documentation, release notes, known limitations, and rollback procedure are current;
- branch, commit, toolchain, and CI evidence are recorded;
- promotion is explicitly approved by the professor/maintainer.

This is the proposed default gate for promotion from `WorkInProgress` to `2026-2`.

### Strict gate

Includes the standard gate plus:

- sanitizer/Valgrind or equivalent diagnostic matrix;
- broader application/platform matrix;
- numerical/scientific benchmark suites;
- performance/regression thresholds;
- release-candidate observation period;
- all P1 issues resolved rather than waived.

A strict gate is a candidate for later promotion toward `currentStable` or `master`, depending on the release objective and available infrastructure.

Status:

- promotion-gate concept: `decided`;
- standard gate as the default for `2026-2`: `proposed`, pending explicit approval;
- exact required checks and waiver authority: `needs-human-decision`.

See `genesys_2026_human_decisions.md` for the proposed `2026-2` checklist.

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
- Do not promote a branch merely because it is mergeable; record the applicable promotion-gate evidence.
- Never silently waive a P0/P1 gate criterion.
