---
document_type: executed-evidence
authority: informative
owner: project-maintainer
date: 2026-11-05
status: in-progress
scope: Transfer category and LabelDefinition/Label/Goto implementation
---

# Transfer / LabelDefinition / Label / Goto — remote progress

- Repository: `rlcancian/Genesys-Simulator`.
- Integration branch: `WorkInProgress`; base SHA: `3ae85774949ebc87e521a2409eecf8f086bcc43b`.
- Development branch: `WiP202611/transfer-label-goto`.
- Mandate: implement E1–E9 sequentially, produce one Draft PR, **never merge** during remote phase.
- Evidence status: source inspection only so far; no build or test has yet run remotely or locally.

## E1 — Components directory and category

- State: `IMPLEMENTED`, pending compilation/GUI verification.
- Change: moved 12 component classes / 24 .h/.cpp files from `components/MaterialHandling` to `components/Transfer`; preserved C++ class names and behavior.
- Dependencies: 13 source consumers updated, including DummyPlugin connector, Smart/teaching/arenaExamples applications and GUI category mapping.
- Commit SHA: `187725628cd919f0ac47232f858b1d31ad1dd949`.
- Check: static source comparison and indexed reference inventory; tests not executed (`UNKNOWN`).
- Risk: GitHub index may omit references; CI must verify linker/includes.

## E2 — DataDefinitions MaterialHandling → Transfer

- State: `IMPLEMENTED`, static inspection only; compilation `UNKNOWN`.
- Change: seven DataDefinitions / 14 .h/.cpp files moved to `data/Transfer`; their categories changed to `Transfer`.
- Consumers: 13 source files updated (components, Smart examples, connector, teaching, Arena examples).
- Commit SHA: `234b36c8d40ad6a4d37004ffa607b2b330836a91`.
- Behavior and typenames: intended unchanged; tests not yet executed.

## E3 — Rename old data Label to LabelDefinition

- State: `IMPLEMENTED`; compile/test: `UNKNOWN`.
- Moved `data/Logic/Label.{h,cpp}` to `data/Transfer/LabelDefinition.{h,cpp}`; classname, typename, metadata, factories and registry changed.
- Consumers: `Route.{h,cpp}`, `Sequence.{h,cpp}`, `PluginConnectorDummyImpl1.cpp`; static identifier `labeldefinition.so`.
- Receiver pointer remains non-owning, can be set once per live LabelDefinition and is checked against the model's component registry by identity (without dereferencing expired addresses).
- Receiver name is no longer eagerly resolved while DataDefinitions load. New Label component will establish the association after its own load/check.
- The old shadowing label field and duplicate editor control were eliminated; inherited ModelDataDefinition Label text remains available.
- Commit SHA: `ad81f471e1c09a429867d99f55b22a85dedfe039`.
- Tests not yet executed. Outstanding: receiver registration/type and load-order validation in E4/E6/E7.

## E4 — Label receiving ModelComponent

- State: `IMPLEMENTED`; compilation and runtime tests: `UNKNOWN`.
- Files: new `components/Transfer/Label.{h,cpp}`; registered `label.so` in DummyPlugin connector.
- Contract: receiving transfer, zero conventional inputs, one output; one internal owned LabelDefinition, mandatory editable property, persisted definition name, deterministic rebinding in check, no artificial delay.
- Ownership: internalData owns target; pointer validated using registry identity before dereference; second receiver is rejected.
- Commit SHA: `9a68c1a4aa9473a9abc7424b0a19f949431fcf6b`.
- Risks: replacing a definition may invalidate external Goto/Route/Sequence references; must be tested/mitigated before ready-to-merge.

## E5 — Goto sending ModelComponent

- State: `IMPLEMENTED`; compile and end-to-end validation `UNKNOWN`.
- Files: `components/Transfer/Goto.{h,cpp}`; DummyPlugin connector registration `goto.so`.
- Contract: transfer sender; exactly one conventional input, zero outputs; attached non-owning LabelDefinition reference; TimeDelayExpression default 0.0; TimeDelayTimeUnit default second; parser and time-unit conversion; reject negative/nonfinite delays and undefined destination.
- Persistence: name, expression and unit saved; name resolved after loading/check.
- Commit SHA: `f47295355b6f3138bd4e2981d13cfeeec72eaa72`.
- Risk: unload/removed data pointer must be invalidated in attached maps; review E6.

## E6 — Integration and ownership audit

- State: `IMPLEMENTED` (static source audit; build/UI `UNKNOWN`).
- Corrections: restored `Route::DestinationType::Label` public enum while keeping the new `LabelDefinition` C++ type; corrected constness for mutable ConnectionManager methods; rejected non-Label/foreign receiver and guarded shared-definition replacement; prevented out-of-bounds `TimeUnitConvert` for invalid time enums; updated legacy runtime and dangling-reference test includes and old DataDefinition type uses.
- Static registration: connector has distinct `labeldefinition.so`, `label.so`, `goto.so`; CMake source discovery remains `GLOB_RECURSE CONFIGURE_DEPENDS`.
- GUI: existing category coloring changed to Transfer. Port metadata provided; actual graphical/layout/Property Editor activation `REQUIRES_LOCAL_VALIDATION`.
- Commit SHA: `b6b7225cdc146d4fba8a9d743e59cdb7a0d0628a`.
- Limitations: no remote compile, GUI or test evidence yet; deletion of an internally owned LabelDefinition while its owner is alive remains a high-risk scenario requiring targeted testing.

## E7 — Automated behavioral tests

- State: `IMPLEMENTED`, execution `UNKNOWN`.
- Files: `source/tests/unit/test_transfer_label_goto.cpp`; `source/tests/unit/CMakeLists.txt` new GTest executable `genesys_test_transfer_label_goto`.
- Coverage authored: plugin metadata and ports; one internal definition; idempotent check; duplicate receiver; missing Goto destination; invalid expressions and units; zero, positive, negative delays; minute-to-second conversion; identity preservation across transfer and output; multiple Gotos; isolated destinations; `.gen` full check-save-load-check-simulate and no orphaned entities.
- Existing `test_simulator_runtime.cpp` tests Route/Sequence and LabelDefinition renamed; `test_dangling_reference_load.cpp` paths updated.
- Commit SHA: `767c5b4f9371d7dcd969ea518a2e994743ef9894`.
- No CI or local testing has run. All test outcomes `UNKNOWN`, not PASS.
- Missing: explicit `.gui` round-trip and manual GUI Property Editor port/layout validation.

## E8 — Remote validation (pre-PR checkpoint)

- State: `REQUIRES_LOCAL_VALIDATION` for desktop GUI/sanitizers; `BLOCKED` for hosted CI until the Draft PR opens.
- GitHub diff compare: base `3ae85774949ebc87e521a2409eecf8f086bcc43b` to E7 head `767c5b4f9371d7dcd969ea518a2e994743ef9894`: ahead=7, behind=0, 65 changed paths. This is a source/diff check, NOT a compilation/test result.
- Commit combined statuses on E7 SHA: empty; `fetch_commit_workflow_runs`: zero pull_request-triggered runs.
- Why: `.github/workflows/genesys-ci.yml` explicitly triggers on `pull_request` against `WorkInProgress` (or manual workflow_dispatch); no push trigger. The available GitHub connector does not expose dispatch-workflow, and no CI run exists yet.
- Action: open authorized Draft PR at E9 (CI trigger), revisit E8 in subsequent corrections; never claim pre-PR CI PASS.
- Post-PR CI, GUI and sanitizer status: `UNKNOWN` until run IDs/logs are available.

## E9 — Draft PR and Codex handoff

- State: `NOT_STARTED`.
- Draft PR to `WorkInProgress` must remain unmerged; local final validation required.

