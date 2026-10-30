---
document_type: validation-evidence
authority: informative
owner: continuous-models-implementation
created: 2026-10-30
status: active
tracks: 550
---

# PR #550 — DiffusionSimulate concurrent waiting entities

## Scope

Extended the concrete `DiffusionSimulate` calendar interruption regression to
deliver two entities into the same active run. The first arrives at time 0,
the second at 2.5, and a discrete event interrupts continuous advancement at
5.5. The test verifies each original entity is forwarded exactly once at
completion time 10. No production code or numerical method changed.

## Evidence

- **Confirmed by execution:** the concrete component completes the one shared
  `DiffusionField` run; at global 5.5 the ordinary event observes confirmed
  local time 5.0.
- **Confirmed by execution:** both distinct requested entity IDs arrive at the
  connected `Dispose` at 10.0, once each.
- **Confirmed by execution:** the focused test passes normally and under
  ASan/UBSan (`ASAN_OPTIONS=detect_leaks=0`).
- **Confirmed by execution:** the complete continuous diffusion target passes
  41/41; full unit suite and kernel suite each pass 1,942 with four disabled
  and zero failures; smoke passes 3/3.
- **Confirmed in code:** the test changes only the fixture helper and
  expectations in `test_plugins_continuous_diffusion.cpp`.

## Test fixture boundary

The unit target has no dynamic descriptor registered for `DiffusionSimulate`.
As documented in the preceding evidence, the fixture keeps a valid checked
source-to-sink model graph and injects two scheduled entities into the actual
concrete component instance. This exercises component dispatch, shared-run
association, event-calendar continuation, and downstream forwarding. It does
not establish dynamic factory discovery or persisted GUI model loading.

## Reproduction

```bash
cmake --build --preset tests-unit --target genesys_test_plugins_continuous_diffusion --parallel 2
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.DiffusionSimulateRetainsEntityAcrossCalendarInterruption
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion
ctest --preset tests-unit --output-on-failure
cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
ctest --preset tests-smoke --output-on-failure
cmake --build build/observable-asan --target genesys_test_plugins_continuous_diffusion --parallel 2
ASAN_OPTIONS=detect_leaks=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.DiffusionSimulateRetainsEntityAcrossCalendarInterruption
```

The test change is commit `be3eaac42`. Hosted CI for this source checkpoint is
pending at evidence capture. The kernel CTest rerun is logged at
`/tmp/genesys-be3-kernel-ctest.log`. PR #550 remains draft. Full cancellation,
destruction, persistence, and cross-model lifecycle coverage remain incomplete;
ordinary discrete mutation of inputs already used by calculated work remains
the explicit `HUM-SCI-002` condition. **Maturity: `PARTIALLY_VALIDATED`.**
