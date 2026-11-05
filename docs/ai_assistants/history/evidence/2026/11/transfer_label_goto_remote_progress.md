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


## CI findings — PR #564, initial attempt

- Date: 2026-11-05; PR: https://github.com/rlcancian/Genesys-Simulator/pull/564 (Draft, not merged).
- GitHub Actions run `42081180321` (Phase 0) — job `127434302492`, `tests-smoke`: compile `FAIL`; emitted `Transfer/Conveyor.h:6:10: fatal error: plugins/data/MaterialHandling/Segment.h: No such file or directory`.
- Corrective action: review all 22 moved/created Transfer headers; changed 9 headers with residual `MaterialHandling` include paths (Conveyor, Transporter; Access, Exit, Move, Start, Stop, Store, Unstore).
- Evidence: https://github.com/rlcancian/Genesys-Simulator/actions/runs/42081180321 ; no tests executed in failed job.
- The original CI attempt is not a PASS; re-run through new PR head to verify.

## CI findings — PR #564, second attempt

- Run `42081621236`, tests-kernel-unit job `127435858437`: `FAIL` at plugin connector compile, `PluginConnectorDummyImpl1.cpp:62:10: fatal error: components/MaterialHandling/PickStation.h: No such file or directory`.
- Root cause: pre-existing relative include omitted from earlier exact-prefix replacement. Fixed to `components/Transfer/PickStation.h`.
- Regression audit found existing `source/tests/unit/test_simulator_runtime.cpp` still exercised obsolete LabelDefinition→arbitrary sink association and asserted `MaterialHandling` categories. Updated to the new required owning `Label` receiver and `Transfer` metadata, retaining Route/Sequence behavioral coverage.
- State: source corrected and committed at next checkpoint; follow-up CI required. Original failure remains FAIL.

## CI findings — PR #564, third attempt and corrective commit

- Run `42082527441`, job `127439171086`, `tests-kernel-unit`: **FAIL** at compilation of `source/tests/unit/test_station_statistics_lifecycle.cpp` due to obsolete include `plugins/data/MaterialHandling/Station.h`; no CTest execution took place in this job.
- The companion `tests-smoke` job completed successfully for this run. This does **not** establish success of kernel/unit tests.
- Corrective commit: `a1c318f70020faf36ee56d2daec2b49803780d1f` — changed that test include to `plugins/data/Transfer/Station.h` without modifying runtime behavior.
- Validation of the corrective head: `UNKNOWN` pending a new workflow run. GUI interaction, `.gui` roundtrip and sanitizers remain `REQUIRES_LOCAL_VALIDATION`.
- The PR remains **draft and unmerged**. E8 is still `IN_PROGRESS`; E9 handoff is not acceptance or merge authorization.

## HANDOFF — CODEX LOCAL VALIDATION AND MERGE

### Provenance and scope

- Repository: `rlcancian/Genesys-Simulator`.
- Development branch: `WiP202611/transfer-label-goto`; initial base `WorkInProgress` SHA `3ae85774949ebc87e521a2409eecf8f086bcc43b`.
- Draft PR: https://github.com/rlcancian/Genesys-Simulator/pull/564 ; **do not merge before maintainer acceptance**.
- Current implementation source checkpoint: `a1c318f70020faf36ee56d2daec2b49803780d1f`, followed by documentation commits. Obtain authoritative final HEAD from `git rev-parse origin/WiP202611/transfer-label-goto` immediately before validating.
- Original E1–E7 commits: `187725628`, `234b36c8`, `ad81f471`, `9a68c1a4`, `f4729535`, `b6b7225c`, `767c5b4f`; later CI diagnostics/corrections: `8ba7b59b`, `d0a0a1e1`, `055aa617`, `a1c318f7`; inspect exact history in PR, including later documentation updates.
- Directories renamed: `source/plugins/components/MaterialHandling/` to `source/plugins/components/Transfer/`, and `source/plugins/data/MaterialHandling/` to `source/plugins/data/Transfer/`. Former `source/plugins/data/Logic/Label.{h,cpp}` replaced by `source/plugins/data/Transfer/LabelDefinition.{h,cpp}`.
- Created: `source/plugins/components/Transfer/Label.{h,cpp}`, `Goto.{h,cpp}`, `source/tests/unit/test_transfer_label_goto.cpp`. Updated: static connector, GUI category mapping and graphical serializer, Route, Sequence, smart examples, unit CMake, existing test includes. Full file inventory: PR Files changed.
- Contracts: `Goto` (1 conventional input, 0 output, `sendTransfer`) references an existing attached `LabelDefinition`, parses a delay, converts to replication base units, and schedules one event; `LabelDefinition` records one receiving `Label`; `Label` (0 input, 1 output, `receiveTransfer`) dispatches the same entity over conventional output. No historical old-type persistence migration is required.
- Label currently owns its associated definition through `ModelDataDefinitionAssociations::_internalDataInsert` / internal-data deletion; Goto and Route/Sequence must remain observers and must not delete it. Confirm teardown when referenced objects are removed early or replaced.

### Cloud evidence and limitations

- Authored focused test target: `genesys_test_transfer_label_goto` in `source/tests/unit/CMakeLists.txt`, covering metadata, duplicate receiver, attached destination, delays, event identity, two Gotos, isolated destinations, and a `.gen` roundtrip.
- Prior CI run `42081180321` failed due to old include in moved headers; corrected by `d0a0a1e1`. Subsequent run `42081621236` failed in plugin connector include; corrected by `055aa617`.
- CI run `42082527441`: `tests-kernel-unit` compile **FAIL** in `test_station_statistics_lifecycle.cpp` due to obsolete Station include; `tests-smoke` job completed successfully. Fixed by `a1c318f7`; inspect subsequent CI *at final head* for actual outcome. No earlier failed job should be described as PASS.
- Not yet demonstrated: fully successful final-head focused CTest, complete GUI property-editing and port rendering, `.gui` roundtrip, ASan/UBSan/LSan and deletion/replace stress; all **UNKNOWN** or **REQUIRES_LOCAL_VALIDATION**.
- Because the canonical CI often tests a PR merge ref, record both PR head and merge SHA with each claim. `WorkInProgress` advanced at least two commits since the original base.

### Required local validation (Ubuntu 24.04 / C++23 / Qt6)

1. Read `docs/ai_assistants/README.md`, `GOVERNANCE.md`, `ARCHITECTURE.md`, `STATUS.md`, and applicable runbooks; fetch and check out the feature branch without altering `WorkInProgress`.
2. Confirm `git status --short` is clean, exact `git rev-parse HEAD`, new upstream base, and final PR diff. Detect obsolete active include paths using `git grep -n 'plugins/.*MaterialHandling/' -- source`; distinguish legacy example *directory names* from actual moved-plugin paths.
3. Run canonical presets from root `README.md` / `CMakePresets.json` (check their current definitions first):
   `cmake --preset tests-unit && cmake --build --preset tests-unit --parallel "$(nproc)" && ctest --preset tests-unit --output-on-failure`;
   `cmake --preset tests-kernel-unit && cmake --build --preset tests-kernel-unit --parallel "$(nproc)" && ctest --preset tests-kernel-unit --output-on-failure`;
   `cmake --preset tests-smoke && cmake --build --preset tests-smoke --parallel "$(nproc)" && ctest --preset tests-smoke --output-on-failure`;
   `cmake --preset gui-app && cmake --build --preset gui-app --parallel "$(nproc)"`.
4. Execute the actual `genesys_test_transfer_label_goto` target and inspect **all** assertions. Add/fix regressions for nonfinite parsed delays, deletion of `LabelDefinition` while `Label` and `Goto` survive, removal of a `Label` with attached senders, repeated ModelCheck, failed replacement with external references, and model teardown. A skipped test or a process exit code with no assertions is insufficient evidence.
5. Build/run GUI and confirm both plugins appear in `Transfer`, port counts and absence of a conventional edge between Goto and Label, Property Editor configuration of label destination/delay/time unit, and creation/association of a unique internal definition.
6. Test `Create -> Goto` and separately `Label -> Dispose`, same definition, 0 and positive delays in equal and different units; inspect future-event timestamps, entity pointer/ID identity, counts, independent definitions, two senders, and duplicate-receiver rejection. Repeat after `save -> load -> check -> simulate`, in both `.gen` and `.gui` when supported. Verify Route and Sequence now refer to `LabelDefinition`.
7. For memory/undefined behavior use a clean debug build with CMake's documented sanitizer options if present. If not, configure a **separate** build directory with the compiler/linker flags `-fsanitize=address,undefined -fno-omit-frame-pointer -g` where compatible; check ASan/LSan diagnostics and avoid claiming global leak freedom from one focused run.
8. Inspect User/Developer Manual impact. Per `GOVERNANCE.md`, user-visible plugins and architecture require appropriate manual updates and, when possible, compilation/regeneration of `docs/ManualGenESyS.pdf` before final approval. Keep this work in same PR or an explicitly linked documentation PR.
9. On any failure, make small commits on the *feature branch*, preserve regression tests, update this evidence and PR body, rerun final-head CI. Do not disable checks or weaken assertions to turn CI green.
10. Merge only after final-head CI, local functional/GUI/lifetime tests, manual impacts, reviewer findings, ownership invariants, no critical regression and **explicit maintainer acceptance**. The remote phase does not authorize the merge.

### Principal residual risks

- **Ownership/lifetime:** a definition internally owned by Label is also registered with the model; deletion/replacement may leave the owner's pointer or attached references stale. The source uses registry membership guards, which prevent some dereferences but do not prove safety of reallocation/pointer reuse or destructor order.
- **Persistence:** order-independent name binding is implemented in methods but must be demonstrated by actual `.gen` and `.gui` reload paths; confirm no temporary/default definition remains orphaned after load.
- **Event semantics:** one Goto passage must produce exactly one Label event, then the Label conventional-output event, preserving entity identity and simulated time units.
- **GUI:** metadata alone does not prove real port drawing, clickability or Property Editor usability.
- **Build/CI:** residual old includes and transitive dependencies caused several verified failures; do not extrapolate from individual successful object compilations to the complete test suite.
