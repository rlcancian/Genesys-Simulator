---
document_type: validation-evidence
authority: informative
owner: continuous-models-implementation
created: 2026-10-30
status: active
tracks: 550
---

# PR #550 — ODE component interruption with concurrent entities

## Scope

Extended `ContinuousCalendarTest.ODEComponentWaitsAndResumesAfterDiscreteInterruption`
to send a second entity to the same active `ContinuousSystemComponent` run at
global time 2.5. A real discrete event at 5.5 observes the confirmed ODE state;
the component resumes the existing run and completes at local time 10. No
production code or numerical method changed.

## Evidence

- **Confirmed by execution:** `ModelSimulation::start()` processed the
  interruption at global 5.5 while the ODE exposed confirmed time/state 5.
- **Confirmed by execution:** both distinct requested entity IDs arrived at
  the connected `Dispose` at time 10, exactly once each.
- **Confirmed by execution:** the focused calendar test passed normally and
  under ASan/UBSan with LeakSanitizer disabled.
- **Confirmed by execution:** the focused continuous target passed 41/41;
  `tests-unit` passed 1,942 with four disabled and zero failures;
  `tests-kernel-unit` passed 1,942 with four disabled and zero failures;
  `tests-smoke` passed 3/3.
- **Confirmed in code:** only the ODE calendar fixture assertions changed;
  production sources and the numerical solver were untouched.

## Bounded conclusion

Together with the BioSimulate and DiffusionSimulate cases, this demonstrates
two waiting entities sharing one interrupted run in each concrete continuous
component path covered by the current tests. It does not establish complete
cancellation/destruction, persistence, or all independent cross-model
lifecycle behavior. The fixture directly constructs the component in the
static unit target; it does not establish dynamic plugin discovery or GUI
model loading. The `HUM-SCI-002` condition for ordinary events mutating inputs
already consumed by precomputed work remains unresolved.

## Manual impact

No user-visible behavior or production implementation changed; the existing
continuous-execution reference was updated, so no User or Developer Manual
chapter change was needed for this test-only extension.

## Reproduction

```bash
cmake --build --preset tests-unit --target genesys_test_plugins_continuous_diffusion --parallel 2
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.ODEComponentWaitsAndResumesAfterDiscreteInterruption
ctest --preset tests-unit -R 'ContinuousCalendarTest' --output-on-failure
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion
ctest --preset tests-unit --output-on-failure
cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
ctest --preset tests-smoke --output-on-failure
cmake --build build/observable-asan --target genesys_test_plugins_continuous_diffusion --parallel 2
ASAN_OPTIONS=detect_leaks=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.ODEComponentWaitsAndResumesAfterDiscreteInterruption
```

The source/test commit is `42bb445f2b63830e5622beec0377be4e04598502`. Hosted
CI on this new source commit is pending. PR #550 remains open and draft.
