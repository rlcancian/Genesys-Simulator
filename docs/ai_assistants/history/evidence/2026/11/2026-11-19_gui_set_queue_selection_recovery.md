---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
tracks: GUI Set Queue Property Editor selection recovery
branch: WorkInProgress
base_commit: 2710572f6b4b7070ce18e0344a1cdc96688e2338
status: validated-local
---

# Property Editor recovery after removing a Queue from a Set

The maintainer reported that, after removing one of two Queue members from a
Set, selecting any other component left the Property Editor empty. The previous
`PropertyEditorLists.RemovesSetResourceAndQueueReferencesWithoutDeletingMembers`
test exercised an isolated browser and proved list ownership, but did not
exercise the MainWindow selection and deferred-refresh chain.

## Reproduction and root cause

`PropertyEditorLists.RemovingQueueFromSetKeepsMainWindowSelectionEditable`
creates a Set with two Queues, references it from Seize, selects it through the
Data Definitions inspector, removes one Queue using the actual list action,
then selects and edits Create. Before the correction, the list changed but the
test failed: Create properties stayed absent. Logs showed the scene
synchronization clearing the old Set binding, a deferred rebuild being canceled
for invalid binding context, and repeated model-change callbacks waiting on
`_pendingRebuild=true`. The PropertyEditorController therefore considered its
post-commit pipeline perpetually busy and deferred later selection changes.

`ObjectPropertyBrowser` now clears `_pendingRebuild` when a deferred rebuild
cannot run because the old binding was detached. It also delivers the already
committed model-change notification after selection detachment, so dirty state
and the deferred global refresh are not lost. `ModelInspectorController` keeps
the selected DataDefinition row by pointer identity when it still belongs to
the current model; rebuilding the row no longer emits a transient empty
selection. The Queue removed from the Set remains in the model's DataManager.

## Validation

- Focused Qt regression: red before the fix, green afterward. It confirms that
  the remaining Set member is the second Queue, the removed Queue still exists,
  the Set's properties stay visible, the browser is no longer busy, another
  component's properties appear and editing its `FirstCreation` reaches the
  kernel object.
- Related unit CTest filter (`PropertyEditorLists`,
  `ModelInspectorSelectionRegression`, `GuiClassroomLifecycle`): 28/28 passed.
- ASan/UBSan (`-fsanitize=address,undefined`, `detect_leaks=0`): the new
  MainWindow regression and the existing Set Resource/Queue removal regression
  passed 2/2. Leak detection was disabled for this focused run.
- `cmake --build --preset gui-app --parallel 2`: passed.
- `ctest --preset tests-unit --output-on-failure`: 2,203/2,203 active tests
  passed; four historical tests were disabled and two local-reference tests
  were skipped.
- `python3 scripts/validate-ai-docs.py`: passed, checking six canonical,
  83 active and 25 retained historical Markdown files.

The User and Developer Manual already describe Set member removal as preserving
the referenced Queue/Resource. This repair restores that documented behavior
and the existing selection behavior; no manual text change was needed. A human
desktop interaction with the maintainer's exact saved model was not performed.
