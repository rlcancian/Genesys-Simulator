---
document_type: evidence
authority: scoped
owner: ai-assistant
date: 2026-11-06
task: AUTO-PERSIST-001
status: provisional
---

# Model save failure diagnostics — Phase 1

## 1. Provenance

- Baseline: WorkInProgress, SHA 62828c325e8f0a93567401bf311e98aaf5a95891 (direct GitHub branch query, 2026-11-06).
- Branch: WiP202611/save-failure-diagnostics.
- Draft PR: #568, https://github.com/rlcancian/Genesys-Simulator/pull/568.
- Related merged PR: #559, merge commit 3ae85774949ebc87e521a2409eecf8f086bcc43b. Previous save-preparation tests were green but did not resolve the recurrent field failures.
- Environment: ChatGPT Web + GitHub connector, without local Ubuntu/Qt, filesystem, tracing, strace or interactive GUI access.
- **No merge is authorized in Phase 1.**

## 2. Pipeline — confirmed by source inspection

- Shell/commands and GUI ultimately delegate to ModelManager::saveModel(filename), which calls Model::save(filename).
- Model::save() calls _createInternalDataDefinitions() and then PersistenceDefaultImpl2::save(). ModelCheck and orphan pruning are not invoked by save; incomplete models remain eligible.
- PersistenceDefaultImpl2::save() chooses format from filename extension, gathers simulator/model/simulation records, then DataDefinitions and ModelComponents through Serializer::put(), then opens filename+".tmp", calls dump(), flushes/closes and renames the temporary to destination on success.
- GenSerializer::dump() writes metadata, loops over DataDefinitions, then ModelComponents, using PluginManager::find(type) to identify which records belong in each pass.
- GraphicalModelSerializer::saveGraphicalModel("X.gui") asks ModelManager::saveModel("X.gui.tmp"); kernel writes "X.gui.tmp.tmp" then renames to "X.gui.tmp"; GUI appends graphical sections, flushes/closes and renames "X.gui.tmp" to "X.gui".
- Therefore .tmp.tmp is an expected consequence of nested transactional saves, not proof of the defect.

## 3. Failure matrix

| Operation | Method | Precondition | Failure | Historical observability | New signal |
|---|---|---|---|---|---|
| Record collection | PersistenceDefaultImpl2::save | valid objects and fields | put=false, exception, collision | ignored put return; collision traced only | SERIALIZER_DUPLICATE_NAME (GEN only); others still gaps |
| Open temporary | PersistenceDefaultImpl2::save | parent exists, destination accessible | open fails | generic temporary-file error | FILE_OPEN_FAILED; errno |
| GEN classification | GenSerializer::dump | registered plugin/valid type | missing plugin/record | false without typename context | SERIALIZER_PLUGIN_NOT_FOUND / SERIALIZER_RECORD_MISSING |
| GEN stream writes | GenSerializer::dump | valid stream | bad/fail bit | logical false or success depending pass | SERIALIZER_STREAM_WRITE_FAILED |
| Serializer result | PersistenceDefaultImpl2::save | serializer can dump | dump=false but stream healthy | misclassified I/O | SERIALIZER_DUMP_FAILED |
| Write during dump | PersistenceDefaultImpl2::save | stream open | bad stream state | generic error | FILE_WRITE_FAILED |
| Flush / close | PersistenceDefaultImpl2::save | successful dump | buffered write/close failure | generic error | FILE_FLUSH_FAILED / FILE_CLOSE_FAILED |
| Rename destination | PersistenceDefaultImpl2::save | temp complete | replace failed | generic replace error | FILE_RENAME_FAILED; errno |
| GUI kernel delegation | GraphicalModelSerializer | current model | kernel save failed | generic popup | GUI_SAVE_KERNEL_FAILED and preceding kernel cause |
| GUI intermediate append | GraphicalModelSerializer | kernel temp available | open failed | QFile errorString popup | GUI_SAVE_OVERLAY_OPEN_FAILED |
| GUI overlay | GraphicalModelSerializer | valid scene items | missing drawing/group entry, bad stream | heterogeneous generic text | GUI_SAVE_OVERLAY_SERIALIZATION_FAILED / WRITE_FAILED |
| GUI flush/close/rename | GraphicalModelSerializer | overlay complete | filesystem/Qt failure | generic UI message | GUI_SAVE_OVERLAY_FLUSH_FAILED / CLOSE_FAILED / GUI_SAVE_RENAME_FAILED |

Current blind spots: prepare-time exceptions; swallowed SaveInstance errors without fail propagation; non-GEN put failures are now logged but their return does not block save; unhandled GUI precondition failure; overlapping calls to save; controlled close-only failure; coverage of all graphical object categories.

## 4. Confirmed code defects versus field symptoms

1. **Confirmed, observability:** old kernel success flag conflated open, dump, write, flush and close failures. The maintainer's generic message does **not** establish disk I/O failure.
2. **Confirmed, code correctness:** GenSerializer::dump() overwrote the result of the DataDefinition pass with the ModelComponent pass. This could hide an earlier error if the second pass succeeds. A missing plugin often causes both passes to fail, so the bug's responsibility for observed incidents is unproven.
3. **Confirmed, data-loss risk:** GenSerializer::put() stores records in a name-keyed map. A duplicate name emits traceError but replaces the first record. Previously covered in test_save_name_collision.cpp; occurrence in private failing models unknown.
4. **Confirmed, design risk:** both kernel and GUI use deterministic filename+".tmp" and have no visible per-destination serialization lock. Concurrent or reentrant collision is plausible, but no real overlapping save has been observed.
5. **Confirmed, silent record-save exception:** ModelComponent::SaveInstance and ModelDataDefinition::SaveInstance catch std::exception thrown from _saveInstance, trace an error, but cannot return failure to PersistenceDefaultImpl2. The latter may put a partial record and still return success. The new stage trace SERIALIZER_RECORD_SAVE_REPORTED_ERROR tracks the per-object error counter but does not change this historical semantic contract; direct causal link to maintainer incidents remains unknown.
6. **Confirmed, save preparation:** _createInternalDataDefinitions() runs for each save; its iteration restarts when DataDefinition classnames grow. Idempotence was previously tested in limited configurations, not all remove/reinsert/recreate paths.

No actual instance of the maintainer's recurrent save error has yet been independently reproduced.

## 5. Changes submitted in Draft PR #568

- Kernel stages produce SAVE_DIAGNOSTIC with code, stage, format, destination, temp path, dump result, stream state and errno when attributable to open/rename/close. The code differentiates healthy-stream serializer false from an I/O failure.
- GenSerializer emits category, typename, ID and lookup stage for unknown plugins. The first pass no longer loses its result; stream errors and duplicate-name collisions receive stable identifiers.
- GUI reports GUI_SAVE_KERNEL_FAILED without obscuring earlier kernel/serializer errors, and stages GUI overlay failures separately.
- During data/component collection, a before/after count of TraceManager error messages identifies SaveInstance-reported failures by category, typename, ID and stage. Rejected Serializer::put results are now labelled SERIALIZER_RECORD_REGISTRATION_FAILED; to avoid changing production semantics in this observational phase, these logs do not force a failed save. This remains a data-integrity limitation.
- On failed opening of an existing temporary *directory*, the kernel does not attempt to remove a path it did not successfully open. No new logging library or persisted format.
- No general persistence redesign, ModelCheck gate or DataDefinition pruning.

## 6. Tests added (execution status: pending)

Kernel target genesys_test_persistence_atomic_save now includes source/tests/unit/test_model_save_diagnostics.cpp, with:
- ExistingTemporaryDirectoryIsReportedAsOpenFailure: prior destination intact and unrelated temp contents retained;
- MissingParentDirectoryIsOpenFailure: open diagnostic;
- RenameFailurePreservesExistingDestinationDirectory: failure code, existing contents unchanged;
- MissingPluginIsLogicalFailureNotFilesystemFailure: dump=false with healthy stream, existing .gen unchanged;
- UnknownTypenameIsReportedByGenSerializer: injected unknown typename;
- RejectedOutputStreamIsReportedAsWriteFailure: direct streambuf failure;
- RepeatedIncompleteSaveAndGuiKernelIntermediateWork: no ModelCheck, repeated .gui.tmp kernel path with nested temp;
- DeviceFullInducesStreamFailureWithoutFillingDisk: private test symlink to Linux /dev/full, no real disk filling.
- TransferGraphDeletionAndRecreationRemainsSavableWithoutModelCheck: LabelDefinition ownership, Goto detachment, deleted receiver, recreated receiver and consecutive saves.
- ConsolePluginInsertRemoveReinsertAcrossMultipleSaves: Create→Dispose connected flow, isolated Wait, Signal and CppForG insertion/deletion/recreation, saving between every step without executing model.

Qt target genesys_test_gui_gmdd_layout now includes source/tests/unit/test_gui_model_save_diagnostics.cpp, with:
- RepeatedIncompleteGuiSaveUsesTwoCleanTemporaryLevels: real .gui save, both temp levels disappear, model remains incomplete;
- OverlaySerializationFailurePreservesPreviousGuiFile: controlled invalid drawing; GUI overlay error, prior .gui unchanged.

Existing tests test_persistence_atomic_save.cpp, test_gen_serializer_text.cpp, test_save_name_collision.cpp and test_gui_efsm_diagram_persistence.cpp also exercise related paths. These do not replace the additional fault matrix.

## 7. Hypothesis ledger

| Hypothesis | Favorable evidence | Contrary evidence | Test discriminant | Executed result | Status |
|---|---|---|---|---|---|
| Open/I/O | original error text consistent with failed stream | occurs in both /tmp and project paths | missing dir, blocked temp, /dev/full | pending CI | HIPÓTESE NÃO RESOLVIDA |
| dump=false | previous code mislabels logic failures | exact dump result not captured for user incidents | inject unknown plugin, observe stream state | pending CI | HIPÓTESE NÃO RESOLVIDA |
| Plugin not registered | GEN lookup fails hard | user components commonly registered | unknown type, compare PluginManager state | pending CI | HIPÓTESE NÃO RESOLVIDA |
| Duplicate names | name-keyed map overwrites | naming code attempts uniqueness | duplicate-name test plus before-save inventory | mechanism previously confirmed; relevance unknown | HIPÓTESE NÃO RESOLVIDA |
| Stale internal data | repeated preparation modifies data | bounded #559 regression was green | remove/recreate and save between operations | not implemented here | HIPÓTESE NÃO RESOLVIDA |
| Fixed temp collision | two levels and stable names | no observed parallel saves | reentrant save in controlled harness | not executed | HIPÓTESE NÃO RESOLVIDA |
| GUI overlay | append/rename separate path | reported .gui.tmp.tmp error points to kernel stage | overlay null drawing and kernel failure | pending CI | HIPÓTESE NÃO RESOLVIDA |

## 8. CI, build, and exact-head requirements

Canonical commands verified in README.md and CMakePresets.json:

    cmake --preset tests-unit
    cmake --build --preset tests-unit --parallel "$(nproc)"
    ctest --preset tests-unit --output-on-failure

    cmake --preset tests-kernel-unit
    cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
    ctest --preset tests-kernel-unit --output-on-failure

    cmake --preset tests-smoke
    cmake --build --preset tests-smoke --parallel "$(nproc)"
    ctest --preset tests-smoke --output-on-failure

    cmake --preset gui-app
    cmake --build --preset gui-app --parallel "$(nproc)"

Focused filter after configuration: ctest --test-dir build/tests-unit -R 'ModelSaveDiagnostics|ModelSaveGuiDiagnostics' --output-on-failure.

Initial PR SHA f178a43dc8cba1b9414529e5fd61d8360b04f786 triggered GitHub Actions, with example run 42304334992 (unit + GUI) and run 42304334896 (kernel/smoke). At document creation runs were queued. Do not report PASS before completion and log/artifact inspection; PR merge ref may have a different SHA from the branch head.

## 9. Limits, preservation, and future fixes

- Controlled GUI kernel failure requiring QMessageBox dismissal, remove/recreate sequences involving all named plugins, undo/redo, reference expiration, long pathname limits, close-only injections and actual simultaneous same-path saves remain incomplete.
- A previous destination file is only replaced after the kernel writes and closes a temporary. Nested GUI persistence similarly replaces final .gui after overlay completion. This does not prove crash-durable fsync or concurrency safety.
- The preexisting verbose trace of linearized records may expose model source/text at high trace levels; avoid uploading these traces without redaction.
- A failed open with deterministic temp already occupied as a file, and concurrent cleanups, may still interfere with another save. Phase 2 should evaluate O_EXCL / unique same-directory temp and save serialization only after reproducing the conflict.
- Do not change persisted representation or plugin ABI before confirming a defect. If collisions are causal, reject/uniquify with migration testing; if dump=false, investigate plugin typename registration; if filesystem failure, obtain syscall errno and directory stat.
- Developer/User Manual changes and PDF regeneration should be reviewed in Phase 2 once diagnostic codes and corrective contract stabilize; Phase 1's detailed diagnostic guide is this evidence record.

## 10. HANDOFF — MODEL SAVE ROOT-CAUSE INVESTIGATION

Branch: WiP202611/save-failure-diagnostics; Draft PR #568; reference SHA 62828c325e8f0a93567401bf311e98aaf5a95891. Final branch HEAD must be re-queried before local use.

Local Codex should first read docs/ai_assistants/README.md, then execute the verified CMake/CTest presets above against a clean worktree and record exact SHA, toolchain, Qt version, results and artifact paths. Reproduce in disposable files with full trace classification from kernel/GUI. Include model-operation sequences (insert/connect/remove/reinsert) for Create, Dispose, Wait, Signal, CppForG, Goto, Label, LabelDefinition, with save between steps. Record PluginManager's registered typename, IDs, DataDefinitions and dangling reference inventory before and after _createInternalDataDefinitions().

For local-only failures capture: stat/type/perms of each temp and target; namei -l parent paths; df -h and df -i for relevant mount; errno from the actual failed operation; dump return; iostream flags at dump/flush/close; whether a plugin was not found; GUI overlay return and QFile error. Use strace with selective openat/write/close/rename/unlink only on disposable fixtures; gdb conditional breakpoints on failed save stages; AddressSanitizer/UBSan for graph remove/recreate lifecycle when justified. Do not infer causality from a stack trace alone.

Phase 2 correction acceptance: reproduce real failure before fix on fixed fixture; identify stage and cause; patch the smallest justified behavior; check both return and diagnostic; prove old destination unchanged under injected failures; .gen/.gui round-trip; repeated and relevant edit sequences; full tests-unit, tests-kernel-unit, tests-smoke, gui-app and GUI functional smoke; record a clean exact-head CI and independent reviewer verdict. **Do not merge Phase 1.**

## 11. Phase 1 criterion status

C1 pipeline mapping: code inspected; C2 diagnostics: code added / build pending; C3 controlled failures: tests authored / execution pending; C4 classification: implemented / execution pending; C5 preservation: assertions authored / execution pending; C6 .gen/.gui: tests authored / execution pending; C7 SHA and CI: pending; C8 root-cause hypotheses: unresolved; C9 handoff: authored.

**Conclusion: source-level observability defects are confirmed, but no cause of the reported real-world failures is yet confirmed.**
