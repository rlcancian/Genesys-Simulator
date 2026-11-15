---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
tracks: AUTO-APP-004
branch: WiP20261115/minor_changes
base_commit: 9ce86ecc27f57e04f6056c047011b69e54bda701
status: local-validation-passed-pending-hosted-checks
---

# Minor application and plugin naming changes

## Scope

Implements the 2026-11-15 maintainer instruction tracked by `AUTO-APP-004`.
The worktree is isolated at `/home/rafaelcancian/Repositories/Genesys-MinorChanges`;
the pre-existing local `WorkInProgress` checkout and its commits were preserved.

## Implemented

- Renamed `DummyComponent` and `DummyElement` to `TemplateComponent` and
  `TemplateDataDefinition`, with Doxygen documentation and current plugin
  registration names.
- Renamed the process adapter class to `ModalModel`, preserving the
  `DefaultNetwork` association/bindings, adapter execution, and stable internal
  association key.
- Added load-time aliases from persisted `DummyComponent`, `DummyElement`, and
  `ModalModelDefault` type names to current plugin IDs. The template component
  also resolves the old `.DummyElement` child suffix.
- Added a save/load/resave compatibility test covering all three old IDs,
  `TemplateComponent` internal data, and a `ModalModel`/`EFSMNetwork` reference.
- Added per-application version identity (`inaiera`, `261115`) to production
  application controllers and Qt application metadata. The launcher's existing
  SemVer command-line contract remains unchanged.
- Changed the simulation configuration label to `Report Base Time Unit`; its
  existing `getReplicationBaseTimeUnit()`/`setReplicationReportBaseTimeUnit()`
  binding and temporal implementation are unchanged.
- Updated affected user/developer manual sources and regenerated the PDF.

## Validation so far

| Validation | Result |
|---|---|
| `tests-unit` build (588/588 build steps) | PASS |
| Legacy plugin-ID round-trip focused test | PASS, 1/1 |
| ModalModel network suite | PASS, 14/14 |
| Simulator runtime suite | PASS, 373/373 (+4 disabled) |
| Corrected static-plugin registration test | PASS, 1/1 |
| Combined production application build (GUI, standalone GUIs, shell, worker, launcher/dispatcher) | PASS |
| Launcher/dispatcher `--version` contract | PASS; both report existing SemVer `0.0.0` in local build |
| Launcher tests | PASS, 34/34 |
| Smoke suite | PASS, 3/3 |
| `tests-kernel-unit` full CTest | PASS, 2,158 passed (+4 disabled) |
| Final `tests-unit` full CTest | PASS, 2,147 passed (+4 disabled) |
| Manual rebuild (`docs/developers/ManualGenESyS_subrepo/./make.sh`) | PASS; updated `docs/ManualGenESyS.pdf` |
| `python3 scripts/validate-ai-docs.py` | PASS |
| Hosted PR checks and merge | PENDING |

The first full `tests-unit` CTest run failed one stale assertion expecting
`dummyelement.so`. The expectation was updated to current template plugin names;
the focused test passed and `ctest --rerun-failed` passed. The final complete
`tests-unit` run passed 2,147 tests; four pre-existing disabled tests were
reported and did not run.

## Remaining

Complete final unit/manual/documentation validation, review the final diff,
commit and push, open a PR to `WorkInProgress`, wait for required checks, and
merge only when the governance conditions are met. Keep the original local
`WorkInProgress` history intact while synchronizing after integration.
