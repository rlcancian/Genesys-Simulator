---
document_type: evidence
authority: executed-evidence
owner: project-maintainer
recorded_at: 2026-10-16
status: active
scope: pr-550-continuous-models-local-audit
---

# PR #550 Continuous Models — Local Audit Handoff

## A. Identification

- Task: local audit and stabilization of the continuous-model refactor in PR #550.
- Repository: `rlcancian/Genesys-Simulator`.
- Branch: `WiP20261016/ContinuousModels`; PR #550 targets `WorkInProgress` and remains draft.
- Initial branch HEAD: `c9d1533d3d65582ae4f18115dbccec04dc267429`.
- `origin/WorkInProgress` at audit start: `6df79436ff9d72e302e9d2b047fc5a8f6e1f297b`.
- Merge base: `6df79436ff9d72e302e9d2b047fc5a8f6e1f297b`.
- Implementation commit: `6cb42d1c43fe6472a9f8ccd63cdebbbf7f69ef11` (`fix(continuous): defer state evolution to sync events`).
- Final implementation commit: `6cb42d1c43fe6472a9f8ccd63cdebbbf7f69ef11`; the evidence/manual commit follows it. No merge to `WorkInProgress` is performed.
- Environment: Ubuntu 24.04.5 LTS, x86_64; GCC/G++ 13.3.0; CMake 3.28.3; Ninja 1.11.1; CTest 3.28.3; Qt 6.4.2; Git 2.43.0; GitHub CLI 2.45.0.
- Toolchain memory constrained parallel compilation to `--parallel 2`.

## B. Received implementation diagnosis

### Correct or retained

- The extraction into `ContinuousModelDataDefinition` and `evolveState(currentTime, dt, errorMessage)` are retained; numerical state evolution remains in the specialized subclasses.
- The runtime uses local time starting at zero and persists `StepsToSynchronize`; old `AutoSchedule`/`AutoAdvance` fields remain read as compatibility fallbacks.
- The solver’s error/maximum-step checks and the analytic oscillator tolerance were not weakened.

### Defects confirmed and corrected

- The received `_continueEvolution()` advanced the numerical state before the scheduled synchronization event was dispatched. This allowed the continuous state to run ahead of the kernel calendar and conflicted with reactive `ContinuousSystemComponent` integration. Evolution now occurs only when the synchronization event is dispatched.
- Synchronization scheduling used the current time rather than the next synchronization boundary; grouped step intervals and the mandatory final partial synchronization were not represented correctly. The base now computes the next local boundary, caps it at `StopTime`, and maps it to absolute calendar time using the captured origin.
- Floating-point remainders could create an extra tiny step or fail to land exactly on the final target. The final step now snaps to the requested target within a small machine-epsilon allowance.
- Destruction of a continuous definition with a queued `InternalEvent` could leave a callback targeting a destroyed object. Its destructor now removes and destroys pending events that refer to it.
- Legacy nonzero `startTime` was silently ignored. It is now rejected with a diagnostic, and the ODE, diffusion, and biochemical loaders reject failed non-empty records. Empty records remain accepted for plugin field/template generation.
- `DiffusionField.h` retained declarations for `getLastStatus` and related methods after those methods moved to the base, causing the reported undefined reference. The redundant declarations were removed.
- The BioNetwork test still called removed `setAutoSchedule(true)`; it now configures `StepsToSynchronize`, and exercises two replications.
- The GUI property test expected the removed continuous `StartTime` control; it now asserts that this property is absent.

### CI failures on the received HEAD

The branch’s then-current hosted checks were still red at initial HEAD `c9d1533d`: run `38957790914` (`tests-unit`), `38957790739` (`tests-kernel-unit` and `tests-smoke`), and `38957790713` (plugin link evidence). The local compile reproduced the stale `setAutoSchedule` call and the local link reproduced `DiffusionField::getLastStatus()` being undefined. The smoke oscillator failure was also reproduced before the timing correction; after the correction, `test_continuous_system` passes unchanged.

## C. Corrections and affected files

Implementation commit `6cb42d1c` changes:

- `source/plugins/data/Continuous/ContinuousModelDataDefinition.{h,cpp}`: pending-event lifetime, captured global origin, grouped/final synchronization boundaries, deferred evolution, partial-step rounding and absolute-target handling.
- `source/plugins/data/Continuous/ODESolver.cpp`, `DiffusionField.cpp`, `DiffusionField.h`, and `source/plugins/data/BiochemicalSimulation/BioNetwork.cpp`: loader failure handling, explicit unsupported legacy-start rejection, and removal of stale inherited declarations.
- `source/plugins/components/Continuous/DiffusionSimulate.{h,cpp}`: current `StepsToSynchronize` contract replaces stale `AutoSchedule` guidance.
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`: default, invalid legacy start, synchronization boundaries, pending-event destruction, and nonzero global-origin integration tests.
- `source/tests/unit/test_simulator_runtime.cpp`: two-replication BioNetwork calendar test uses the new control.
- `source/tests/unit/test_gui_property_editor.cpp`: assertion reflects the intentional absence of a continuous-model `StartTime` property.

The manual and generated PDF/index changes are being committed separately from implementation. No changes were made to `Failure`, general kernel hierarchy, scientific formulations, or the `ModelComponent` async contract.

Risks: strict rejection of old nonzero continuous starts may reject previously accepted files; the diagnostic is explicit rather than silently changing their time origin. The `DiffusionSimulate` component still has its own configurable `StartTime`; the interaction of a nonzero component override with the new zero-origin continuous API remains unresolved and is listed below.

## D. Build results

Successful configurations/builds:

- `cmake --preset tests-unit`; `cmake --build --preset tests-unit --parallel 2`.
- `cmake --preset tests-kernel-unit`; `cmake --build --preset tests-kernel-unit --parallel 2`.
- `cmake --preset tests-smoke`; `cmake --build --preset tests-smoke --parallel 2`.
- Focused ASan/UBSan build in `/tmp/genesys-pr550-asan-build` for `genesys_test_plugins_continuous_diffusion` and `genesys_test_simulator_runtime`.
- `docs/developers/ManualGenESyS_subrepo/make.sh` completed successfully and regenerated `docs/ManualGenESyS.pdf` (386 pages, 8,069,102 bytes).
- User documentation: `doxygen docs/users/DoxyfileUser` generated its intermediate LaTeX sources, but `make -C build/doxygen/users/latex` failed on pre-existing Unicode `Δ` in `source/plugins/components/WholeCellModeling/CellGrowthComponent.h:14` (and related source comments). The tracked user PDF was not overwritten; unrelated scientific comments were not changed.

The full manual build emits `xdvipdfmx` warnings about annotations outside page boundaries in existing pages; it exits successfully. No C++ build warnings attributable to these edits were found in the recorded build outputs. Logs are local and not versioned: `/tmp/genesys-pr550-tests-unit-final.log`, `/tmp/genesys-pr550-tests-unit-final2.log`, `/tmp/genesys-pr550-kernel-build-final.log`, `/tmp/genesys-pr550-kernel-ctest-final.log`, `/tmp/genesys-pr550-smoke-build-final3.log`, `/tmp/genesys-pr550-smoke-ctest-final3.log`, and `/tmp/genesys-pr550-asan-final.log`.

## E. Tests executed

| Suite / test | Result | Evidence and scope |
|---|---|---|
| `tests-unit` | PASS: 1,918 executed, 0 failed, 4 disabled; 1,922 listed | Final run `/tmp/genesys-pr550-tests-unit-final2.log`; full unit regression. |
| `tests-kernel-unit` | PASS: 1,917 executed, 0 failed, 4 disabled; 1,921 listed | `/tmp/genesys-pr550-kernel-ctest-final.log`; full kernel preset before the final additional origin test, which was then built and run directly in the kernel preset. |
| New T0=100 calendar test under kernel preset | PASS: 1/1 | `genesys_test_plugins_continuous_diffusion --gtest_filter=DiffusionFieldPluginTest.SynchronizationUsesGlobalOriginCapturedAtStart`. |
| `tests-smoke` | PASS: 3/3 | Final run `/tmp/genesys-pr550-smoke-ctest-final3.log`, including unchanged `test_continuous_system` oscillator accuracy test. |
| BioNetwork calendar/two-replication integration | PASS: 1/1 | Direct test `SimulatorRuntimeTest.BioNetworkStepSynchronizationAdvancesWithKernelInternalEvents`; also passed in the kernel preset suite before the test was renamed. |
| DiffusionField focused suite | PASS: 18/18 | `genesys_test_plugins_continuous_diffusion`; synchronization boundaries, zero sync, partial stop, deletion, and persistence checks. |
| ASan + UBSan focused checks | PASS: 3/3 | Event-target destruction, global origin, and two-replication BioNetwork. `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`; LeakSanitizer was explicitly disabled. No repository-wide memory claim. |
| Developer manual | PASS with existing layout warnings | `./make.sh` in `docs/developers/ManualGenESyS_subrepo`; PDF regenerated. |
| User manual | FAIL, unrelated existing source-content blocker | Doxygen source generation succeeded; PDF generation failed on unsupported Unicode delta in existing whole-cell comments. Existing tracked PDF remains unchanged. |

The CTest inventory contains 980 generated `KernelSimulatorMethodInventory` placeholder cases; these are reported in raw CTest counts but excluded from behavioral-test counts. Accordingly, the unit behavioral execution count is 938 and kernel behavioral count is 937, plus the subsequently run focused origin test. Four historical Search/Remove tests were disabled and not executed.

The first sanitizer run exposed an invalid test-fixture receiver: `InternalEvent::show()` requires a `ModelDataDefinition`, while the fixture used a plain helper object. The fixture was corrected to use the `DiffusionField` probe as receiver; the final focused ASan/UBSan run then completed without sanitizer diagnostics.

## F. Temporal correctness evidence

- `StepsToSynchronize=0`: initialization posts no event; explicit simulation completes locally through `StopTime` without calendar events.
- `1`: event after every step; the matrix expects `1,2,...,10`.
- `2`: `2,4,6,8,10`.
- `3`: `3,6,9,10`; the last event is the final stop synchronization.
- `5`: `5,10`.
- `20`: stop is reached and synchronized once at `10`.
- Partial final step: `StopTime=2.5`, `Step=1`, sync count 2 yields `2,2.5`; decimal case `StopTime=1`, `Step=.1`, sync count 3 yields `.3,.6,.9,1`.
- Nonzero global origin: a real calendar event at `T=100` starts evolution; synchronization completes at `100.2` and final stop at `100.25`, while local current time ends at `.25`.
- Kernel integration: the BioNetwork test processes events through the actual calendar across two replications. State is not advanced before a queued synchronization event is dispatched.

The tests distinguish scheduling from dispatch and check for no queued events at completion. No claim is made that every possible solver, consumer, or concurrent mutation schedule has been exhaustively proven.

## G. Sanitizer and memory-safety evidence

- Configuration: GCC 13.3.0, Debug, `-fsanitize=address,undefined -fno-omit-frame-pointer` on compile and link.
- Executed targets: `genesys_test_plugins_continuous_diffusion` and `genesys_test_simulator_runtime`, each with focused event lifetime/calendar tests.
- Runtime flags: `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`.
- Result: final focused set passed with no ASan/UBSan diagnostic. The initial fixture-only invalid-vptr diagnostic was corrected as documented above.
- Limitation: LeakSanitizer was disabled; this evidence is not a claim of leak freedom or full suite sanitizer coverage.

## H. Remaining issues

| Severity | Area | Evidence / impact | Recommendation | Decision |
|---|---|---|---|---|
| P1 | `DiffusionSimulate` component time-window override | It retains its own `StartTime` and calls `ContinuousModelDataDefinition::simulate(startTime, stopTime, step)`, whose compatibility overload rejects nonzero starts. Existing tests cover zero only. A nonzero component override can therefore fail instead of preserving legacy behavior. | Decide whether component `StartTime` means local offset, global launch time, or should be removed; then implement and add a real calendar integration test. | Yes; do not reinterpret scientific time silently. |
| P2 | `ODESolver` defaults | Constructor currently defaults to `StopTime=60`, `StepSize=.01`, `StepsToSynchronize=1`, inherited from the received branch’s compatibility profile. This starts automatic synchronization/evolution at replication initialization and may create 6,000 calendar events even if a model did not explicitly request it. | Maintainer should confirm that this default is intentional; no default was changed in this stabilization. | Yes if changing the default. |
| P2 | legacy model compatibility | Nonzero persisted continuous `StartTime` now fails closed. This is deterministic but may reject old models previously accepted with an altered/ambiguous meaning. | Inventory representative legacy models and document migration to local zero origin. | No architectural decision required to retain explicit rejection; compatibility fixtures remain desirable. |
| P2 | User Manual PDF | Doxygen generation succeeds, but `pdflatex` fails on `Δ` in existing `CellGrowthComponent.h` source documentation. User PDF remains unchanged. | Track/fix Unicode-to-LaTeX escaping in a separate documentation cleanup; do not edit unrelated biochemical semantics here. | No. |
| P2 | GitHub checks at initial HEAD | Current check query on the pre-fix PR head showed the documented unit/kernel/smoke/link failures. New commits must be checked on hosted CI after push. | Wait for checks on final pushed HEAD; keep PR draft until reviewed. | No. |

## I. Conclusion and handoff

Classification: **PARCIALMENTE VALIDADO**.

Locally confirmed: grouped and final synchronization timing, local/global origin mapping including `T0=100`, two-replication BioNetwork runtime, no pre-dispatch numerical advancement, pending-event cancellation on destruction, stale declaration link fix, loader rejection behavior, smoke oscillator accuracy, unit/kernel suites and focused ASan/UBSan.

Not confirmed: nonzero `DiffusionSimulate` override semantics, LeakSanitizer, full-suite sanitizers, real GUI interaction, all legacy model files, successful User Manual PDF generation, and hosted CI on the final pushed SHA. The PR remains open and draft; no merge was performed. The next technical step is a maintainer decision for the component `StartTime` contract, followed by a focused consumer integration test and final hosted-check review.
