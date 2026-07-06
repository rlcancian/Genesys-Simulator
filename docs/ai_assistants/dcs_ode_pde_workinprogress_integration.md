# DCS ODE/PDE Integration into WorkInProgress

- Date: 2026-07-05
- Source branch: `2026-1`
- Target branch: `WorkInProgress`
- Temporary branch: `integration-dcs-ode-pde-to-workinprogress-20260705`
- PR: #460, draft
- Status: first implementation slice in progress. The tools-only Tema 8.2 numerical core was ported to the current `WorkInProgress` layout under `source/tools/Continuous/`, with focused unit tests registered in `source/tests/unit/CMakeLists.txt`. Plugin/data/component integration is still pending.

## Objective

Prepare a selective integration path for accepted DCS Tema 8.1 and Tema 8.2 contributions already merged in `2026-1`, without importing rejected academic artifacts or overwriting unrelated `WorkInProgress` changes.

## Repository state confirmed through GitHub connector

`2026-1` and `WorkInProgress` both exist as usable refs. A GitHub compare from `WorkInProgress` to `2026-1` reported a diverged history: `2026-1` was 41 commits ahead of `WorkInProgress` and 133 commits behind it. The ahead-side file list contained accepted ODE/PDE payload plus unrelated or unsuitable files such as `.github/workflows/genesys-ci.yml`, `__temporary_check`, `source/tools/Optimization/OptimizerDefaultImpl1.*`, and historical `documentation/` entries. Those unrelated files were not ported.

## PRs analyzed

### PR #437 — Tema 8.1 — EDOs

- Title: `Tema 8.1 - EDOs`.
- Base: `2026-1`.
- Current GitHub state: closed and merged.
- Merge commit: `5cb8cdd81d3831f45e66db36409a611db78d5dc1`.
- Integration source status: accepted in `2026-1`, but not yet ported in this implementation slice.

Relevant files in #437:

- `source/plugins/data/Continuous/ODESolver.h`
- `source/plugins/data/Continuous/ODESolver.cpp`
- `source/plugins/components/Continuous/ContinuousSystemComponent.h`
- `source/plugins/components/Continuous/ContinuousSystemComponent.cpp`
- `source/plugins/components/Continuous/LSODE.h`
- `source/plugins/components/Continuous/LSODE.cpp`
- `source/plugins/components/Continuous/DiffEquations.h`
- `source/plugins/components/Continuous/DiffEquations.cpp`
- `source/plugins/PluginConnectorDummyImpl1.cpp`
- smoke tests and demonstration models.

Current `2026-1` versions must be treated as authoritative over older evaluation notes and ZIP material. In particular, current `source/plugins/data/Continuous/ODESolver.h` uses `kernel/simulator/model/ModelDataDefinition.h`, not the older invalid include path from the earlier evaluation.

### PR #425 — Tema 8.2 — EDOs e EDPs

- Title: `8.2: Solver de EDO por Factory + Dormand-Prince 5(4) e plugin de difusao N-D`.
- Base: `2026-1`.
- Current GitHub state: closed and merged.
- Merge commit: `7066c4f5f76a675da28f34d9e8c4d1c931e32372`.
- Integration source status: accepted in `2026-1`; the tools-only subset was ported/adapted in this branch.

Relevant files in #425:

- `source/tools/OdeSolverFactory.h`
- `source/tools/DormandPrince54OdeSolver.h`
- `source/tools/DiffusionMethodOfLinesSystem.h`
- `source/tests/unit/test_tools_ode_solver_factory.cpp`
- `source/tests/unit/test_tools_diffusion_mol.cpp`
- `source/plugins/data/Continuous/DiffusionField.*`
- `source/plugins/components/Continuous/DiffusionSimulate.*`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`

The accepted PR no longer contains the original rejected academic artifact set. The current accepted GitHub file list does not include root `README.md`, `relatorio.pdf`, `video_entrega.mp4`, or direct `BioNetwork.*` changes.

### PR #439 — Tema 8.1 controlled evaluation

- Title: `DCS T8.1 controlled evaluation`.
- Base: `2026-1`.
- Current GitHub state: open and not merged.
- Head: `t81ctrl2`.
- Use only as historical/evaluation context. The integration source for Tema 8.1 is the current merged state of #437 in `2026-1`.

## Uploaded reports and ZIP material analyzed

The uploaded Tema 8.1 evaluation report described an earlier state where PR #437 had been technically rejected and PR #439 was open but not merged. That information is now superseded by GitHub for integration purposes because #437 is currently closed and merged.

The uploaded Tema 8.2 evaluation report identified the numerically valuable core files (`OdeSolverFactory`, `DormandPrince54OdeSolver`, `DiffusionMethodOfLinesSystem`, and tests), while rejecting root README replacement, PDF/video artifacts, and domain-specific `BioNetwork.*`/`DiffusionField.*` modifications as they existed at that time. GitHub now shows #425 closed and merged with a cleaner accepted file set under `source/plugins/components/Continuous`, `source/plugins/data/Continuous`, `source/tools`, and `source/tests`.

The uploaded ZIP was inspected as historical context only. Use current `2026-1` files as source of truth.

## Critical WorkInProgress divergence

`WorkInProgress` already reorganized tools into subdirectories. Confirmed examples:

- `source/tools/Continuous/OdeSolver_if.h`
- `source/tools/Continuous/OdeSystem_if.h`
- `source/tools/Continuous/RungeKutta4OdeSolver.h`
- `source/tools/CMakeLists.txt` referencing `Continuous/SolverDefaultImpl1.cpp`, `Statistics/*`, `Optimization/*`, `AIAssistant/*`, and `FactorialDesign/*`.

The accepted Tema 8.2 files in `2026-1` were under root `source/tools/`. This branch intentionally ports them to `source/tools/Continuous/` instead of reintroducing root-level tool headers.

## Implemented in this branch

### Tema 8.2 tools-only numerical core

Created/adapted:

- `source/tools/Continuous/OdeSolverFactory.h`
- `source/tools/Continuous/DormandPrince54OdeSolver.h`
- `source/tools/Continuous/DiffusionMethodOfLinesSystem.h`

Adaptation decisions:

- Preserve the existing `WorkInProgress` layout.
- Include `OdeSolver_if.h`, `OdeSystem_if.h`, and `RungeKutta4OdeSolver.h` from the same `source/tools/Continuous/` directory.
- Keep the implementation header-only, matching the accepted `2026-1` contribution.

### Unit tests

Created/adapted:

- `source/tests/unit/test_tools_ode_solver_factory.cpp`
- `source/tests/unit/test_tools_diffusion_mol.cpp`

CMake updated:

- `source/tests/unit/CMakeLists.txt`

Registered targets:

- `genesys_test_tools_ode_solver_factory`
- `genesys_test_tools_diffusion_mol`

Both targets are added to `genesys_kernel_unit_tests` and `genesys_kernel_unit_tests_run`.

The diffusion MOL test is currently a reduced smoke subset because the GitHub connector blocked creation of the full large test file in a single write operation. It still checks a 1D Dirichlet sine-mode decay, Neumann mass conservation, invalid configuration rejection, and row-major indexing round-trip.

## Explicitly not implemented yet

Not yet ported:

- `source/plugins/data/Continuous/DiffusionField.*`
- `source/plugins/components/Continuous/DiffusionSimulate.*`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`
- `source/plugins/data/Continuous/ODESolver.*`
- `source/plugins/components/Continuous/ContinuousSystemComponent.*`
- selected `LSODE.*` changes
- selected `DiffEquations.*` changes
- `PluginConnectorDummyImpl1.cpp` registrations

Do not copy `PluginConnectorDummyImpl1.cpp` from `2026-1` wholesale. It must be manually merged to preserve newer WCM, AI, Python, biochemical, and other registrations present in `WorkInProgress`.

## Explicitly rejected for this integration

Do not bring:

- root `README.md` replacements from academic submissions;
- `relatorio.pdf`;
- `video_entrega.mp4`;
- ZIP files or Moodle submission artifacts;
- `__temporary_check`;
- unrelated optimization files from the same `2026-1` ahead-set;
- broad `.github/workflows/genesys-ci.yml` changes unless CI policy is intentionally synchronized;
- historical `documentation/` paths as-is; consolidate useful material into `docs/ai_assistants/`.

## Validation performed

Validated by GitHub connector inspection and remote file operations only:

- repository metadata;
- PR state for #425, #437, and #439;
- branch comparison;
- `WorkInProgress` AI-assistant documentation;
- `WorkInProgress` tools layout;
- `WorkInProgress` unit-test CMake structure;
- source file creation and CMake registration in the integration branch.

No local build was run. No CMake/Ninja/CTest execution was possible from ChatGPT Web with GitHub connector.

## Risks remaining

- The new header-only tools need actual CMake/Ninja/CTest validation.
- The reduced diffusion test should later be expanded back toward the full accepted #425 test coverage.
- Plugin/data/component integration is still pending.
- Continuous-time semantics still need review: both DCS themes touch hybrid discrete/continuous simulation, and the current documentation requires explicit tests for time-step and event-calendar interaction.
- `PluginConnectorDummyImpl1.cpp` and `source/tests/unit/CMakeLists.txt` are high-conflict files and must remain manually merged.

## Next steps

1. Wait for CI on PR #460 or run locally:
   - `cmake --preset tests-unit`
   - `cmake --build --preset tests-unit`
   - `ctest --preset tests-unit --output-on-failure`
2. If the tools-only slice passes, port `DiffusionField.*` and `DiffusionSimulate.*`.
3. Add `test_plugins_continuous_diffusion.cpp` only after plugin registration is ported.
4. Port Tema 8.1 `ODESolver` and `ContinuousSystemComponent` after the tools layout is stable.
5. Manually merge plugin registrations into `PluginConnectorDummyImpl1.cpp` without dropping newer `WorkInProgress` entries.
6. Run full `tests-unit` and targeted continuous/hybrid tests before merging into `WorkInProgress`.
