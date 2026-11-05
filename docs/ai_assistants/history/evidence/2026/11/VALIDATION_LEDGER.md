---
document_type: validation-evidence
authority: informative
owner: project-maintainer
last_updated: 2026-11-05
status: active
scope: model-save-decide-source-editor-plugin-catalog
---

# Model Editor Fixes — Validation Ledger

## Integrated scope

Repository: `rlcancian/Genesys-Simulator`

Integration branch: `WorkInProgress`

Starting inspected integration HEAD: `185b03c70d932f010beed2f30ba6b17ef328087f`

Current integration HEAD after PR #565: `957c66503c8966de95b4d34b047ff03dc256efcb`
Feature merge sequence: PR #559 `3ae85774949ebc87e521a2409eecf8f086bcc43b`; PR #563 `185b03c70d932f010beed2f30ba6b17ef328087f`; PR #565 `957c66503c8966de95b4d34b047ff03dc256efcb`.

PR #559 integrated the five requested corrections and the corresponding manual update. PR #563 additionally applied the same source editor contract to OctaveRunner. PR #565 completed the explicit Decide list-edit undo/redo requirement. All three PRs were merged with merge commits to `WorkInProgress`; no stable branch was changed.

## Findings and implementation records

### Save preparation — PASS

`Model::save()` prepares the model's internal DataDefinitions before delegating to persistence. It does not call `Model::check()` and does not call `_clearOrphanedDataDefinitions()`. Independent DataDefinitions remain in the model. Repeated preparation/save is covered for idempotence; the persistence implementation uses the existing atomic replacement path so a failed serialization does not replace the prior valid file. The implementation does not execute simulation or external source code.

Regression coverage exercises incomplete/no-check saves, independent and internal definitions, repeated save, invalid/incomplete state, `.gen` round trip and preservation on failed persistence. Existing `.gui` graphical serializer round-trip tests remain green. This evidence does not claim every conceivable invalid reference in every plugin is diagnosed identically.

### Decide conditions and output ports — PASS

The contract is `output ports = ordered conditions + 1`, with Else last, including one output when the condition list is empty. Positional insert/replace/remove supports duplicate text. Kernel connections for unaffected conditions and Else are shifted/preserved; only the removed output connection is removed. Graphical ports resize immediately. Output statistics are synchronized with the condition count. PR #565 adds QUndoStack commands that restore condition text, ports and the removed kernel/graphical connection.

### Source-code property matrix

| Plugin | Property | Language | Editor before | Editor after | Persistence / test |
|---|---|---|---|---|---|
| CppForG | `OnDispatchEventCode` | C++ | generic string control | shared CodeEditor control | `_saveInstance`/`_loadInstance` round trip, escaping and multiline tests |
| CppForG | `InitBetweenReplicationCode` | C++ | generic string control | shared CodeEditor control | save/load round trip |
| CppForG | `IncludesCode` | C++ | generic string control | shared CodeEditor control | quotes, backslashes, comments and `#include` round trip |
| PythonForG | `InitBetweenReplicationCode`, `OnDispatchEventCode` | Python | existing source-code control | existing shared CodeEditor | apply/cancel and multiline/UTF-8 round trip regression |
| RSimulator | `PreludeScript`, `CommandList` elements | R | generic/list controls | shared multiline source editor; list shape retained | round-trip tests; no R execution for editor validation |
| RSimulatorRunner | `PreludeScript`, `Command` | R | generic string controls | shared CodeEditor controls | persistence/restore tests; no Rscript execution for editing tests |
| OctaveRunner | `PreludeScript`, `Command` | Octave | generic string controls | shared CodeEditor controls | PR #563 multiline UTF-8 round trip; no Octave execution |
| GroProgram | `SourceCode` | Gro | existing source-code control | existing shared CodeEditor | regression round trip |

Paths, executables, working directories, output streams, status values and payload/result fields were not reclassified as source code. Structured command lists retain their list semantics. The matrix is limited to source-bearing properties identified in the inspected plugin set; it is not a guarantee that no other script-bearing plugin exists outside that set.

### Plugin identity and palette — PASS

PickUp metadata/factories now identify PickUp as its own plugin type while preserving its Remove-derived behavior and persisted plugin identity. Tests cover catalog uniqueness, repeat insertion/reload and multiple Remove instances. The main plugin palette filters presentation to ModelComponents; PluginManager registration is unchanged, and ModelDataDefinitions remain available to kernel, persistence and specialized contexts.

## Local validation on the supplemental Decide branch

Commands and results:

- `cmake --build --preset tests-unit --parallel 4 && ctest --preset tests-unit --output-on-failure` — 1,981 passed, 4 disabled, 0 failed (1,985 registered).
- `cmake --preset tests-kernel-unit && cmake --build --preset tests-kernel-unit --parallel 4 && ctest --preset tests-kernel-unit --output-on-failure` — 1,981 passed, 4 disabled, 0 failed (1,985 registered).
- `cmake --preset tests-smoke && cmake --build --preset tests-smoke --parallel 4 && ctest --preset tests-smoke --output-on-failure` — 3/3 passed.
- `cmake --preset gui-app && cmake --build --preset gui-app --parallel 4` — passed.
- Focused Decide/editor/undo tests — 6/6 passed after the final connection-notification correction.
- ASan/UBSan build `build/tests-decide-sanitize`, with `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1`, focused Decide tests — 5/5 passed. LeakSanitizer was disabled; this is not repository-wide leak-freedom evidence.
- `git diff --check` — passed.

## Hosted validation for PR #565

PR: [#565](https://github.com/rlcancian/Genesys-Simulator/pull/565), source commit `01f08694e348e519977f035d3df144391e024974`, merged as `957c66503c8966de95b4d34b047ff03dc256efcb`.

All PR checks completed successfully before merge: unit tests, kernel tests, smoke tests, GUI GMDD diagnostics, AI Assistant/Data Analyser/Optimizer Xvfb startup, Debian build and lifecycle, launcher, shell, worker, generated plugin target graph, and archive/link checks.

The first GUI GMDD and Optimizer workflow attempts were cancelled during dependency installation and consequently lacked artifacts. Failed workflows were rerun selectively. The passing GMDD artifact recorded configure/build/test return codes 0 and 3/3 tests passed. The passing Optimizer artifact recorded successful configure/build, a detected `Genesys Optimizer` window, and controlled process termination after the bounded startup check. Debian artifacts were inspected: package build, install/reinstall/purge lifecycle, AppStream validation and Lintian checks passed. The only Lintian informational item was `developer-info-missing`; AppStream validation reported success.

Hosted runs used GitHub's pull-request merge candidate (`164dca372a5c2a53acf5db6685850dc895415504`) for their generated evidence files; the PR head remained `01f08694e348e519977f035d3df144391e024974`. Checks attached to PR #559 and #563 were separately green before their merges; this ledger does not reuse their historical workflow counts as a substitute for the PR #565 checks.

## Boundaries and remaining limitations

- No interactive human GUI session was performed; GUI evidence is automated Qt tests and bounded Xvfb startup diagnostics.
- The focused sanitizer run disables LSan and does not establish repository-wide leak freedom.
- `.gui` round-trip is covered by existing graphical serializer regressions; a dedicated GUI-only incomplete-model save test was not added.
- Generic Property Editor changes outside Decide Conditions do not gain QUndoStack support from PR #565.
- The previously reported local hook message `Hook failed — hook returned invalid stop hook JSON output` was not reproduced or traced to a safe local configuration fix during this cycle. No hooks or required checks were disabled. The memory observer independently reported exhausted Claude weekly allowance; it was not restarted or modified.

The five backlog entries `AUTO-GUIFIX-001` through `AUTO-GUIFIX-005` were marked `done_confirmed` after source, test and integration evidence was recorded. Current repository state is maintained in [`../../../../STATUS.md`](../../../../STATUS.md).
