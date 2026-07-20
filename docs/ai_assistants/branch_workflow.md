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
- `20261`, `20262`, `20271`, ...: semester-stable branches using the `YYYYs`
  convention, where the final digit identifies semester 1 or 2.
- `20261`: the first-semester 2026 stable branch. It is the renamed continuation
  of the historical branch formerly called `2026-1`; it was not deleted.
- `20262`: the second-semester 2026 stable branch to be populated only at the end
  of the second semester of 2026.
- `WorkInProgress`: the active development line. New developer branches should
  start here.
- `WiP<semester>`: maintainer-specific working branch, such as `WiP20261`.
  This is a convenience branch, not a replacement for the formal repository
  policy.

Historical PRs, workflows, and documents may still contain names such as
`2026-1` or `2026-2`. Interpret those references according to their date, but use
`20261` and `20262` in current governance documents and new automation.

## Promotion flow

1. Developer feature branches merge into `WorkInProgress`.
2. At the end of a semester, the stabilized supported contents of
   `WorkInProgress` are promoted into the corresponding semester branch.
3. At the end of the second semester of 2026, the target semester branch is
   `20262`.
4. Semester branches can feed `currentStable` when they are judged stable.
5. `master` receives pull requests only from `currentStable`.

The promotion chain is intentionally conservative:

```text
feature branches -> WorkInProgress -> YYYYs -> currentStable -> master
```

For the current second-semester cycle:

```text
feature branches -> WorkInProgress -> 20262 -> currentStable -> master
```

Promotion to `20262` is not an immediate action associated with ordinary
consolidation PRs. It is planned for the end of the second semester of 2026.

## Promotion gate terminology

A promotion gate is the documented go/no-go checklist used near a planned
promotion window. It is not a single test. It combines build/test evidence,
unresolved-risk review, application/package readiness, documentation, maturity,
and explicit approval.

A mandatory gate failure blocks promotion unless an authorized waiver records:

- the failed criterion;
- evidence and impact;
- containment or disabled scope;
- responsible owner;
- expiration/review condition;
- explicit human approval.

### Timing for `20262`

During the semester, the project should accumulate evidence, fix blockers, and
raise the supported feature set to the required maturity. The final gate for
`20262` will be defined and executed near the end of the second semester, based
on the actual repository state at that time.

Do not treat every PR as a release gate, and do not attempt premature promotion
to `20262`.

### Maturity requirement

The project policy is to bring all supported functionality to at least
**Level 3 — Beta** before it is included as supported functionality in the
semester-stable result. Functionality that cannot meet Level 3 must be fixed,
disabled, removed from the supported set, or explicitly deferred. After the
supported set reaches Level 3, prioritized functionality may advance to
**Level 4 — Stable user feature**.

Software maturity and scientific claim level are separate. A software feature
may be Level 3 while its scientific outputs remain limited to an educational or
mechanistic-prototype claim.

### Candidate end-of-semester gate

The exact checklist will be approved near the promotion window. Candidate
evidence includes:

- clean build on the supported Ubuntu/Qt6 baseline;
- required unit and smoke tests;
- supported applications building and starting;
- no unresolved P0 blocker;
- P1 issues resolved or explicitly waived with containment;
- supported features at least Level 3;
- representative model compatibility fixtures;
- required package build/install/run checks;
- worker/network behavior matching the controlled academic intranet profile;
- numerical/scientific claims backed by their declared validation packages;
- documentation, release notes, known limitations, and rollback procedure;
- branch, commit, toolchain, and CI evidence;
- explicit professor approval.

Status:

- semester naming `20261`/`20262`: `decided`;
- promotion timing for `20262`: end of second semester 2026, `decided`;
- promotion-gate concept: `decided`;
- exact final `20262` gate and waiver authority: `deferred` until the end-of-semester preparation round.

See:

- `genesys_2026_decisions_addendum_20260720.md`;
- `genesys_2026_human_decisions.md` for earlier terminology and options.

## Docker and preset implications

The Docker packaging in this repository intentionally uses `master` as the
default runtime branch for end users. That keeps the user flow aligned with the
public release branch.

The current `WiP20261` worktree contains packaging/preset history associated
with first-semester work. References to that branch do not change the formal
promotion policy above.

New scripts and documentation must not assume the obsolete hyphenated semester
branch names.

## Notes for assistants

- Do not rewrite the branch policy casually in other documents; link here
  instead.
- When documenting runtime Docker behavior, distinguish the public default
  branch (`master`) from the active implementation branch.
- If a release or packaging workflow needs a temporary override, document the
  override and the reason in the relevant task note.
- Do not promote a branch merely because it is mergeable; record the applicable
  promotion evidence.
- Never silently waive a P0/P1 gate criterion.
- Do not create or promote `20262` prematurely; the target window is the end of
  the second semester of 2026.
