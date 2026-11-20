---
document_type: integration-evidence
authority: local-execution
owner: ai-assistant
date: 2026-11-20
status: candidate-for-review
integration_branch: WorkInProgress
implementation_branch: codex/integrate-branches-20261120
base_commit: 82f57c203454173fcffaf11f19b99a49b939859e
---

# WorkInProgress branch consolidation checkpoint

This record covers a local integration worktree created from
`origin/WorkInProgress` at `82f57c203454173fcffaf11f19b99a49b939859e`.
The original checkout and unrelated worktrees were left untouched.

## Integrated candidates

- PR #591, `WiP20261119/modal-network-editor-gui10-14`: merged cleanly after
  the current WorkInProgress base was refreshed. The local branch added two
  focused fixture corrections from the already validated modal worktree:
  `bf905532a` and `f060792b2`. The EFSM reload ownership fix, GUI relations,
  CPN/Markov/Graph projections and persistence are covered by the tests listed
  below.
- PR #588, `WiP202611/gui-extensions-f1-dependencies`: merged with one
  localized `source/tests/unit/CMakeLists.txt` resolution that retained both
  existing and new Qt test sources. Remote checks were green before merge.
- `WiP202611/gui-extensions-f2-animation-tool`: is a direct descendant of F1
  and merged cleanly after F1. It adds the registered animation-tool slice and
  its test; no separate PR was found for the current branch head.
- PR #587, `WiP202611/gui-formal-architecture-review`: merged as documentation
  with two conflicts resolved by retaining the current `STATUS.md` and the
  current active backlog wording. The formal contract review and reconciled
  architecture/plan documents were retained.

## Validation executed on the consolidated branch

Toolchain: GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1, Qt 6.4.2, Google Test
1.14.0, C++23.

- Focused EFSM/Network/Graph/Markov/CPN and GUI-extension selection: 73/73
  passed.
- `cmake --build --preset tests-unit --parallel 3`: PASS.
- `ctest --preset tests-unit --output-on-failure -j3`: 2329/2329 executed
  tests passed; four historical tests were disabled and two local-reference
  tests were skipped.
- `cmake --build --preset tests-smoke --parallel 3`: PASS;
  `ctest --preset tests-smoke --output-on-failure -j2`: 3/3 passed.
- `cmake --build --preset gui-app --parallel 3`: PASS; `genesys-gui` linked.
- The first pre-consolidation full suite had one Markov persistence failure in
  parallel execution. The test passed 20 isolated repetitions, all later full
  unit/kernel runs, and the final consolidated unit suite. This remains an
  intermittent-risk observation, not a claim of a root-cause fix.
- Xvfb could not start a child command within a bounded 15-second probe in this
  environment. Qt tests used the configured offscreen platform; visual desktop
  interaction is UNKNOWN.

## Candidates not integrated

- `WiP20261113/read-component` / PR #580: NOT INTEGRATED — CONFLICT. Conflicts
  include `GuiExtensionContracts.h` and `SimulationControlAndResponse.h`; this
  needs semantic reconciliation rather than an automatic merge.
- `WiP20261016/ContinuousModels` / PR #550: NOT INTEGRATED — CONFLICT. It
  conflicts in current source and generated/manual documentation; the old
  branch needs a separate continuous-model reconciliation.
- `WiP202608/unified-debian-package` / PR #524: NOT INTEGRATED — CONFLICT.
  Debian install files and manual/package state conflict with the current
  packaging contract.
- `WiP202611/gui-extensions-architecture-implementation` / PR #575: NOT
  INTEGRATED — OBSOLETE/SUPERSEDED by the approved formal architecture and
  F1/F2 slices.
- `Diagnostics20261115/expression-builder-menu-without-hint` / PR #584: NOT
  INTEGRATED — TEST DIAGNOSTIC intentionally contains a failing test.
- `WiP20260722/optimizer-gui-validation` / closed PR #506: JÁ INCORPORADO /
  REDUNDANTE; its only differing workflow line is already represented in the
  current tree.
- `WiP202610/pr545-closeout`: NOT INTEGRATED — HISTORICAL documentation
  conflicts with current status/changelog and adds no source contribution.
- `WiP202609/modal-network-status-reconcile`: NOT INTEGRATED — HISTORICAL
  status reconciliation conflicts with current documents.
- `WiP202608/modal-network-architecture` / closed PR #526: NOT INTEGRATED —
  OBSOLETE documentation; current architecture and formal review are newer.

No remote WorkInProgress update, PR or merge was performed by this checkpoint.
