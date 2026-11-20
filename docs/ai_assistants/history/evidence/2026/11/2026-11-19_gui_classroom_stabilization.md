---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
tracks: GUI classroom stabilization
branch: WorkInProgress
base_commit: 06ffeb7565f76c8774ff7f921ade9ea2bdca770e
status: validated-local
---

# GUI classroom stabilization, 2026-11-19

The audit started from a clean `WorkInProgress` checkout at
`06ffeb7565f76c8774ff7f921ade9ea2bdca770e`. PR #590's five GUI corrections
were present. Its Save/Save As manual-dialog gap motivated direct Qt widget tests.

## Confirmed defects and corrections

- **P0 queued callback crash:** `PropertyEditorController` scheduled zero-delay
  lambdas capturing a raw `this` without a Qt lifetime context. The regression
  destroyed the controller before event delivery and produced SIGSEGV. The
  controller now owns the timer context, so destruction cancels those callbacks.
- **P0 partial save:** serializer record errors, including duplicate names or a
  failed `SaveInstance`, were logged while save still replaced the previous
  file and marked the document clean. Persistence now refuses the save before
  filesystem replacement if the current record pass reported errors. Internal
  components present in both model registries are serialized once by identity.
- **P0 stale `.gen` export:** the explicit export copied the hidden text editor
  and ignored a failed transient model-to-text refresh. It now saves the live
  model transactionally; a failed save leaves the document dirty. The chosen
  filename and format survive subsequent close-and-save operations.
- **P1 failed-open Undo loss:** a rejected model load cleared the existing
  document's undo stack. It is cleared only after a successful load.
- **P1 document switching:** Previous/Next changed the active kernel model
  without synchronizing and restoring each model's dirty state or clearing the
  Property Editor binding. Both actions now do so. Property commits mark the
  active document dirty before deferred tree updates run; ten consecutive
  commits coalesce into one tree refresh.

The existing graphical `Save` path still uses the transactional serializer.
The `ModelLifecycleController` legacy save/close methods are not connected to
current menu actions; the active `MainWindow` flows were exercised.

## Executed evidence

- Red checkpoints: queued controller callback caused SIGSEGV; duplicate-name
  GUI save replaced a previous `.gen` and cleared dirty flags; invalid open
  cleared Undo; ten queued property commits had not yet marked dirty.
- Thirteen focused Qt tests drive real save, open, close, cancel, failed graphical save,
  `.gen` export, model switching, and three classroom models. The Create →
  Dispose, Create → Decide → Assign branches → Dispose, and Create → Seize →
  Delay → Release → Dispose scenarios each create 10 entities after `.gui`
  save, close and reopen. The models have deterministic arrival and delay times.
- A Linux inotify assertion checks that ten Property Editor edits create no
  transient `genesys-model-*` serialization file in the temp directory; the
  existing saved file remains byte-for-byte unchanged until explicit Save.
- The focused destroyed-controller and commit-coalescing tests passed under
  AddressSanitizer and UBSan with `detect_leaks=0` in
  `build/tests-decide-sanitize` (2/2). This covers those paths only.
- `cmake --preset gui-app` and `cmake --build --preset gui-app --parallel 2`
  passed. The resulting application opened a visible 1280x800 window under
  isolated Xvfb and exited cleanly. This is a startup check, not a human visual
  inspection or complete interactive drag/drop test.
- `ctest --preset tests-unit --output-on-failure` passed 2,202/2,202 active
  tests, with four historical disabled tests and two skipped local-reference
  tests. `ctest --preset tests-smoke --output-on-failure` passed 3/3.
- User and developer manual chapters were updated and the 400-page PDF was
  regenerated with the maintained `make.sh`.

No broad claim of GUI crash freedom follows from this bounded audit. The
complete manual classroom interaction, including drag/drop and deletion of a
referenced Resource through a desktop session, was not exercised.

## Final checkpoint

- `ctest --preset tests-kernel-unit --output-on-failure` passed 2,213/2,213
  active tests, with four historical disabled tests and two skipped
  local-reference tests.
- `python3 scripts/validate-ai-docs.py` passed, checking six canonical,
  82 active and 25 retained historical Markdown files.
- `git diff --check` passed before committing. The direct
  `WorkInProgress` commit and push are verified in the final handoff.
