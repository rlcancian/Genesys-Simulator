---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
tracks: GUI-001, GUI-002, GUI-003, GUI-004, GUI-005
branch: WorkInProgress
source_branch: WiP20261118/gui-problems-crash-regressions
source_commit: 39abc33c77945a897bfc97cf5db7b2fad363729b
pull_request: 590
merge_commit: 09967000173c61337a0515ea7e33aa875e258d6c
status: merged-and-validated
---

# GUI property editor and model validation corrections

## Scope

PR #590 integrates the GUI corrections on `WorkInProgress`. The source branch
was created from `53696478f5beb2abcc5ce2d8be1568a419faae23`; the final feature
head was `39abc33c77945a897bfc97cf5db7b2fad363729b`.

## Findings and implementation

- **GUI-001:** corrected Property Editor classification/binding so an editable
  `Variable::InitialValue` reaches the actual kernel definition and survives
  graphical persistence round trips.
- **GUI-002:** offered `Build Expression` for generic editable String controls
  while retaining direct inline editing and the specialized source-code editor.
- **GUI-003:** made invalid parser expressions recoverable during model
  reference discovery and prevented discovery from evaluating expressions or
  registering references by default. Invalid `Decide` conditions remain
  diagnosable; the expression is not silently rewritten.
- **GUI-004:** corrected generic object-list mutation in the Property Editor so
  removing a Set member removes only the reference and leaves the Resource or
  Queue registered in the model.
- **GUI-005:** removed edit-triggered temporary `Model::save()` calls used only
  to refresh hidden SimulLang/C++ views. Those tabs remain in the UI resources
  but are hidden; property edits continue to update dirty state in memory.

The work was grouped into four source commits:

- `fb01e8afa` — recoverable parser reference discovery;
- `616d9e54c` — editable Strings and data-definition identity;
- `194fbb089` — opt-in parser syntax reference collection;
- `39abc33c7` — Set member removal and deferred text serialization.

The merge commit is `09967000173c61337a0515ea7e33aa875e258d6c`.

## Local validation

All commands ran against the final feature head before merge.

| Command / validation | Result |
|---|---|
| `cmake --preset tests-unit && cmake --build --preset tests-unit --parallel 4 && ctest --preset tests-unit --output-on-failure` | PASS; 2,189 passed, 4 disabled, 2 skipped |
| `cmake --preset tests-kernel-unit && cmake --build --preset tests-kernel-unit --parallel 4 && ctest --preset tests-kernel-unit --output-on-failure` | PASS; 2,230 passed, 4 disabled, 2 skipped |
| `cmake --preset tests-smoke && cmake --build --preset tests-smoke --parallel 4 && ctest --preset tests-smoke --output-on-failure` | PASS; 3/3 |
| Focused parser/property-editor/Set/serialization regressions | PASS; 7/7 |
| `cmake --preset gui-app` and GUI application build | PASS |
| Manual figure-spec validation and manual build | PASS; regenerated PDF, 400 pages |
| `git diff --check` and final feature worktree status | PASS; clean before merge |

The full kernel suite included the final GUI regressions for editable
`InitialValue`, automatic Model-to-Traces navigation, Resource/Queue Set member
removal, and property edits that mark the document dirty without refreshing or
saving hidden text views. The `tests-kernel-unit` preset ran 2,230 tests; four
were disabled and two were skipped by CTest.

## Hosted validation and integration

PR #590 was opened as draft against `WorkInProgress`, then marked ready after
the final-head local suites and manual artifacts were reviewed. The following
hosted checks passed: manual PDF, Debian package build and package lifecycle,
shell, worker, launcher, standalone tools, smoke, GUI model diagnostics, AI
Assistant, Data Analyser, Optimizer, plugin archive/link inspection (present
and absent), and plugin target inspection where completed.

Four final reruns (`tests-unit`, parser regeneration, plugin lifetime
sanitizer, and plugin-target introspection) were cancelled by the 30-minute
job limit while their dependency-install steps were still running; configure,
build, and test steps were skipped. The final `tests-kernel-unit` and main GUI
build reruns remained in progress during post-merge closeout. Earlier hosted
checks listed above passed. GitHub reported no configured required branch
protection checks. The corresponding local unit/kernel/smoke suites and GUI
build passed as recorded above. These runner outcomes are infrastructure
cancellations or pending results, not code-test failures.

## Portable package closeout

After integration, `bash ./generate_genesys_install.sh` was run from
`~/Repositories` against commit `78726e7a33fd5dc75d54977cc92fdb18c3593823`.
The build completed successfully and published
`~/University/Seafile/genesys-simulator/genesys-linux-x86_64-portable.tar.gz`
(72 MiB). SHA-256 is
`12437ee15d83f2d169582e47b70f1c003febcb8f50a3a50f919e36a53daf3016`, matching
the generated archive. The previous package was preserved alongside it as
`genesys-linux-x86_64-portable.tar.gz.pre-78726e7a3` (SHA-256
`3bbce1aa0d1d5aa61015364555620df8af53afd7380ffabf411e6d1882e03185`).

## Remaining limits

- Save and Save As dialogs were not manually driven through an interactive
  desktop session; explicit persistence and graphical round-trip are covered
  by automated tests.
- Model identity diagnostics do not normalize names or rewrite legacy model
  data; invalid references are diagnosed and valid identity is preserved.
- This evidence records bounded validation for GUI-001 through GUI-005; it does
  not claim general GUI crash freedom or release readiness.
