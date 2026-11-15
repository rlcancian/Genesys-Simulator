---
document_type: validation-evidence
authority: informative
owner: project-maintainer
last_updated: 2026-11-07
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

## November 2026 integration closeout

The later Report Issue and Standalone Tools integrations (PRs #560 and #562) are recorded separately in [`2026-11-07_november_consolidation.md`](2026-11-07_november_consolidation.md), including their merge commits, final-head checks, PDF evidence and remaining validation boundaries.

The active ModelComponent/DataDefinition Property Editor audit for PR #571 is recorded in [`2026-11-08_datadef_component_property_audit.md`](2026-11-08_datadef_component_property_audit.md). It includes focused load/reconnection regressions and explicitly retains the Wait/Signal dependency and Match output-port migration as unresolved scope.

## 2026-11-09 PR #571 integration

PR #571 head `44e36ee5a6117101b4bf96f429da015349aae197` passed its required checks and was merged into `WorkInProgress` with merge commit `3eafcc14544c4b91c6b17d35b3827581d9dbcb76`. The canonical local checkout was fast-forwarded to the same `origin/WorkInProgress` SHA and was clean. The post-merge Debian package workflow (`42545696837`) was still in progress at this observation. This integration does not close `AUTO-PROPERTY-001`: the Wait/Signal work remains in PR #570, and Match multi-output migration remains undefined for legacy connections.

## 2026-11-13 PR #576 CppCompiler / CppForG validation (integrated)

Repository: `rlcancian/Genesys-Simulator`; feature PR [#576](https://github.com/rlcancian/Genesys-Simulator/pull/576), base `WorkInProgress`. Its post-merge documentation/evidence reconciliation is in draft [PR #582](https://github.com/rlcancian/Genesys-Simulator/pull/582). Validation checkout: `/tmp/Genesys-cppcompiler-pr576`, branch `WiP20261111/cppcompiler-cppforg-runtime`. Starting PR head: `f63e69c45c22896828747c3859125328a59255cf`; final PR head: `4f3d2705ece5993ab5f65b7702c3598aaf2d3c80`; merge commit: [`ad62e3d05c18c0022f42ca266ddaad5eb6a6810d`](https://github.com/rlcancian/Genesys-Simulator/commit/ad62e3d05c18c0022f42ca266ddaad5eb6a6810d). The integration base recorded during initial work was `2ad98f96311e9c13d27078f9dd83bbffafdabe5d`. Corrective commits: [`da13e7c`](https://github.com/rlcancian/Genesys-Simulator/commit/da13e7c83), [`64df90d`](https://github.com/rlcancian/Genesys-Simulator/commit/64df90d33), [`3b5cd17`](https://github.com/rlcancian/Genesys-Simulator/commit/3b5cd172a), [`9db4323`](https://github.com/rlcancian/Genesys-Simulator/commit/9db4323ec), [`e980c04`](https://github.com/rlcancian/Genesys-Simulator/commit/e980c0476), [`4f3d270`](https://github.com/rlcancian/Genesys-Simulator/commit/4f3d2705e). PR state is MERGED; every final-head workflow is successful. The maintainer confirmed review in conversation (“Revisado e ok”); GitHub has no review object and reports no review decision, and the branch was not protected.

### Runtime evidence so far

- Focused CppForG/CppCompiler and persistence/observability CTest selection: 14/14 passed on the final CppForG executable-relative include lookup. It covers `Create → CppForG → Dispose`, real generated callback execution at entity arrival, `Entity::setAttributeValue/getAttributeValue`, recompilation and a second simulation, independent component instances, malformed C++, missing compiler, stale artifact rejection, missing symbols, and `DiffEquations` recompilation.
- The API test asserts the generated code's marker/value and `Dispose.CountNumberIn == 1`; compilation or `dlopen()` alone is not the pass condition.
- Installed-layout test: installed headers under `share/genesys/source` (4.0 MiB), placed the freshly built test host at `libexec/genesys/system/bin`, changed cwd to `/tmp`, then passed `CppForGDynamicTest.GeneratedCodeUsesEntityAttributeApiOnArrival`. `nm -D | c++filt` showed exported `Entity::setAttributeValue/getAttributeValue`; `readelf -d` showed expected Python/C++ runtime dependencies and no RPATH/RUNPATH.
- The official smoke preset configured and built; `ctest --preset tests-smoke --output-on-failure`: 3/3 passed.
- Final full CTest run after the executable-ancestor lookup change: 2,127/2,127 enabled tests passed, 4 pre-disabled, 0 failures. Command: `ctest --preset tests-kernel-unit --output-on-failure`; both dynamic test targets were explicitly built in that preset before the final run. The official `tests-unit` configure/build also passed; its earlier full CTest run was 2,127 passed, 4 disabled, 0 failed, before the last narrow include-lookup patch. The final full CTest inventory plus final focused 14-test run cover that source change.
- The `genesys_kernel_unit_tests_run` target passed 1,956 tests in 43 groups. The initial CTest attempt had reported two dynamic targets as `NOT_BUILT`; after building them, the final complete CTest passed as above.
- Official GUI, shell, worker, and model-specific Smart_CppForG hosts all built. `nm -D | c++filt` confirmed exported `Entity::setAttributeValue` in all four executable hosts. The Smart_CppForG executable was built but not run because its startup writes a model file.
- Rebuilt `build/cppforg-asan` with `-fsanitize=address,undefined`: CppForG (6/6) and CppCompiler (5/5) passed ASan/UBSan. With LeakSanitizer enabled, CppCompiler (5/5) and CppForG (6/6) passed using `/tmp/genesys-cppforg-lsan.supp`; the sole suppression was the pre-existing process-global `Simulator::_completePluginsFieldsAndTemplate` template allocation (29,940 bytes across 966 allocations in the final run). No CppForG/CppCompiler-owned leak remained.

### Confirmed corrections

The runtime library now finds installed kernel headers by walking upward from `/proc/self/exe`, covering the packaged `libexec/genesys/system/bin` layout. CMake installs source headers needed to compile generated code using real kernel types. Host executable targets export kernel symbols for `dlopen` resolution. LSan exposed and the patch corrected ownership leaks in `DiffEquations` and repeated `ModelSimulation` initialization. The CppForG repeated-run test verifies event and between-replication callback counts. User-supplied native code remains in-process and is not sandboxed.

### Validation and integration closeout

The first hosted run on `e980c0476b4106542f1cd187d27f44b076adf427` exposed a preset/build integration defect: kernel CTest registered `genesys_test_cppforg_dynamic` and `genesys_test_cppcompiler_dynamic`, but the `tests-kernel-unit` build preset only built `genesys_kernel_unit_tests_run`. Hosted CTest therefore ended with 2 `NOT_BUILT` failures (the remaining 2,141 enabled tests passed; four were pre-disabled). The preset now builds both dynamic targets as well. On the corrected local tree, `cmake --build --preset tests-kernel-unit --parallel 4 && ctest --preset tests-kernel-unit --output-on-failure` passed: 2,127 enabled tests passed, four disabled, zero failures. The hosted kernel workflow is pending rerun on the corrected head.

The corrected final head `4f3d2705ece5993ab5f65b7702c3598aaf2d3c80` passed every hosted check. This includes ordinary unit CTest (2,141 passed, four disabled), kernel CTest (2,152 passed, four disabled), smoke (3/3), GUI/shell/worker/model-specific builds and diagnostics, Debian package build and lifecycle, plugin target/link inspection, launcher, standalone-tools integration, and documentation governance. The earlier `NOT_BUILT` kernel-target failure on `e980c04` was fixed by updating the preset target list and is superseded by the final green run. PR #576 was merged with merge commit `ad62e3d05c18c0022f42ca266ddaad5eb6a6810d`; `origin/WorkInProgress` was confirmed at that commit immediately after merge. The remaining limitations are that Smart_CppForG was built but not launched because startup writes a model file, and native user code executes in-process without a sandbox. No mandatory validation or integration gate remains open for PR #576.
