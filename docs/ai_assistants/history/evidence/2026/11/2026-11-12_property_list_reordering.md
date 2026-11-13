---
document_type: evidence
authority: scoped
owner: ai-assistant
date: 2026-11-12
task: Property Editor list reordering
status: provisional
---

# Property Editor list reordering

## Scope

Local work on `WorkInProgress`, starting at `154475b3161e5c46c312793fb88546643ae8d6a7`, with pre-existing uncommitted work preserved. This record describes local changes and does not claim a commit, release, or hosted validation.

- Each list item has an indexed row with move-up and move-down actions; scalar list values appear under `Value`. The same actions are available in the row's context menu.
- Positional movement retains duplicate values and object identities. Undo and redo use the active model scene's undo stack, including inspector selections with no graphical item.
- `Decide` moves each condition with its kernel connection and graphical output port; the trailing `Else` port remains fixed.
- `Process` moves coherent paired internal release requests without replacing them; missing or stale internal requests are reconciled first. Non-owning `Set` and `Failure` members retain their identities.
- Full and partial model loaders preserve sparse output-port indices. C++ export retains those indices for sparse connections and keeps the existing output for contiguous connections.
- The existing C++ exporter emits scalar controls only; it does not reconstruct list contents. Checking the emitted port statements does not establish a complete C++ model round trip.

## Local evidence

- `cmake --build --preset gui-app --parallel 4`: final-state build passed.
- An isolated `QT_QPA_PLATFORM=offscreen` launch printed the simulator startup banner and remained running until the eight-second timeout; this does not prove that a window was displayed. An Xvfb window-enumeration attempt timed out before launching the application and is inconclusive.
- `QT_QPA_PLATFORM=offscreen ctest --preset tests-unit --output-on-failure -j 2`: 2,066 passed; four tests were disabled. This includes the final GUI, `Process` reconciliation, inspector undo/redo, and all registered list-owner persistence cases.
- `cmake --preset tests-kernel-unit`, `cmake --build build/tests-kernel-unit --target genesys_kernel_unit_tests --parallel 4`, and `QT_QPA_PLATFORM=offscreen ctest --preset tests-kernel-unit --output-on-failure -j 2`: build passed and 2,066 tests passed; the same four tests were disabled.
- `QT_QPA_PLATFORM=offscreen build/tests-unit/source/tests/unit/genesys_test_gui_gmdd_layout`: 113 tests passed before the final stale-action guard, which was subsequently covered by the unit run.
- `build/tests-unit/source/tests/genesys_test_persistence_atomic_save`: all 27 tests passed, including single and multiple sparse output ports, full and partial loading, and generated C++ port statements.
- `build/tests-unit/source/tests/unit/genesys_test_gen_serializer_text`: nine tests passed.
- Focused simulator runtime tests for C++ export, Assign persistence, and Sequence persistence: three tests passed.
- `cmake --build --preset tests-smoke --parallel 2` and `ctest --preset tests-smoke --output-on-failure`: final-state build passed and all three smoke tests passed.
- Automated Qt tests exercise selector and context-menu actions, boundaries, read-only behavior, duplicate values, object identity, undo/redo, `Process` reconciliation, and `.gui` and `.gen` save/load. They run offscreen; no manual interaction is claimed.

The checkout also contains unrelated local preference/render changes and an untracked handoff. These were not part of this work.
