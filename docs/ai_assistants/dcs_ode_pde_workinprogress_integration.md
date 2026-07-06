# DCS ODE/PDE Integration into WorkInProgress

- Date: 2026-07-05
- Source branch: `2026-1`
- Target branch: `WorkInProgress`
- Temporary branch: `integration-dcs-ode-pde-to-workinprogress-20260705`
- Status: diagnostic and handoff only; no source files were ported in this branch because the GitHub-connector-only analysis found path and architecture divergence that makes blind copying unsafe.

## Objective

Prepare a selective integration path for accepted DCS Tema 8.1 and Tema 8.2 contributions that are already merged in `2026-1`, without importing rejected academic artifacts or overwriting unrelated `WorkInProgress` changes.

## Repository state confirmed through GitHub connector

`2026-1` and `WorkInProgress` both exist as usable refs. A GitHub compare from `WorkInProgress` to `2026-1` reported a diverged history: `2026-1` is 41 commits ahead of `WorkInProgress` and 133 commits behind it. The merge base was `b6bddc5ca3def48606833e9dbc3df23da2424213`; the current `WorkInProgress` tip inspected was `28ebf10a68f80426fbb2fa80f3b98be456f6541e`.

The ahead-side file list contains the accepted ODE/PDE payload, but also unrelated or unsuitable files such as `.github/workflows/genesys-ci.yml`, `__temporary_check`, `source/tools/Optimization/OptimizerDefaultImpl1.*`, and historical `documentation/` entries. These must not be copied blindly into `WorkInProgress`.

## PRs analyzed

### PR #437 — Tema 8.1 — EDOs

- Title: `Tema 8.1 - EDOs`.
- Base: `2026-1`.
- Current GitHub state: closed and merged.
- Merge commit: `5cb8cdd81d3831f45e66db36409a611db78d5dc1`.
- Changed files reported by GitHub:
  - `documentation/continuous-system-ode-solver-validation-2026-06-30.md`
  - `models/TestContinuousSystem.gen`
  - `models/test_harmonic_oscillator.gen`
  - `source/plugins/PluginConnectorDummyImpl1.cpp`
  - `source/plugins/components/Continuous/ContinuousSystemComponent.cpp`
  - `source/plugins/components/Continuous/ContinuousSystemComponent.h`
  - `source/plugins/components/Continuous/DiffEquations.cpp`
  - `source/plugins/components/Continuous/DiffEquations.h`
  - `source/plugins/components/Continuous/LSODE.cpp`
  - `source/plugins/components/Continuous/LSODE.h`
  - `source/plugins/data/Continuous/ODESolver.cpp`
  - `source/plugins/data/Continuous/ODESolver.h`
  - `source/tests/demo_continuous_trace.cpp`
  - `source/tests/smoke/CMakeLists.txt`
  - `source/tests/test_continuous_system.cpp`
  - `source/tests/test_lsode.cpp`
  - `source/tests/test_ode_solver_numerical_validation.cpp`
  - `source/tools/RungeKutta4OdeSolver.h`

Current `2026-1` versions must be treated as authoritative over older evaluation notes and ZIP material, because the PR is now merged. In particular, current `source/plugins/data/Continuous/ODESolver.h` includes `kernel/simulator/model/ModelDataDefinition.h`, not the older invalid `kernel/simulator/ModelDataDefinition.h` path from the earlier evaluation.

### PR #425 — Tema 8.2 — EDOs e EDPs

- Title: `8.2: Solver de EDO por Factory + Dormand-Prince 5(4) e plugin de difusao N-D`.
- Base: `2026-1`.
- Current GitHub state: closed and merged.
- Merge commit: `7066c4f5f76a675da28f34d9e8c4d1c931e32372`.
- Changed files reported by GitHub:
  - `source/plugins/components/Continuous/DiffusionSimulate.cpp`
  - `source/plugins/components/Continuous/DiffusionSimulate.h`
  - `source/plugins/data/Continuous/DiffusionField.cpp`
  - `source/plugins/data/Continuous/DiffusionField.h`
  - `source/tests/CMakeLists.txt`
  - `source/tests/diffusion_demo.cpp`
  - `source/tests/unit/CMakeLists.txt`
  - `source/tests/unit/test_plugins_continuous_diffusion.cpp`
  - `source/tests/unit/test_tools_diffusion_mol.cpp`
  - `source/tests/unit/test_tools_ode_solver_factory.cpp`
  - `source/tools/DiffusionMethodOfLinesSystem.h`
  - `source/tools/DormandPrince54OdeSolver.h`
  - `source/tools/OdeSolverFactory.h`

The accepted PR no longer consists of the original rejected academic artifact set. The current accepted file list does not include root `README.md`, `relatorio.pdf`, `video_entrega.mp4`, or direct `BioNetwork.*` changes, which matches the intended cleanup policy.

### PR #439 — Tema 8.1 controlled evaluation

- Title: `DCS T8.1 controlled evaluation`.
- Base: `2026-1`.
- Current GitHub state: open and not merged.
- Head: `t81ctrl2`.
- This PR is still useful as historical/evaluation context, but it is not the integration source because PR #437 is now merged into `2026-1`.

## Uploaded reports and ZIP material analyzed

The uploaded Tema 8.1 evaluation report described the old state where PR #437 had been technically rejected and PR #439 was open but not merged. That information is now superseded by GitHub for integration purposes, because #437 is currently closed and merged.

The uploaded Tema 8.2 evaluation report identified the numerically valuable core files (`OdeSolverFactory`, `DormandPrince54OdeSolver`, `DiffusionMethodOfLinesSystem`, and tests), while rejecting root README replacement, PDF/video artifacts, and domain-specific `BioNetwork.*`/`DiffusionField.*` modifications as they existed at that time. GitHub now shows #425 closed and merged with a cleaner accepted file set under `source/plugins/components/Continuous`, `source/plugins/data/Continuous`, `source/tools`, and `source/tests`.

The uploaded ZIP was inspected. It contains:

- Tema 8.1 files under a submission folder, including `ContinuousSystemComponent`, `ODESolver`, `DiffEquations`, `LSODE`, tests, models, and a PDF.
- Tema 8.2 files including solver/factory headers, diffusion MOL tests, a nested repository ZIP, `relatorio.pdf`, `video_entrega.mp4`, and academic README material.

For integration, the ZIP is historical context only. Use current `2026-1` files as source of truth.

## Critical WorkInProgress divergence

`WorkInProgress` has already reorganized tools into subdirectories. Confirmed examples:

- `source/tools/Continuous/OdeSolver_if.h` exists in `WorkInProgress`.
- `source/tools/Continuous/OdeSystem_if.h` exists in `WorkInProgress`.
- `source/tools/Continuous/RungeKutta4OdeSolver.h` exists in `WorkInProgress`.
- `source/tools/CMakeLists.txt` in `WorkInProgress` references `Continuous/SolverDefaultImpl1.cpp`, `Statistics/*`, `Optimization/*`, `AIAssistant/*`, and `FactorialDesign/*`.

By contrast, the accepted DCS files in `2026-1` are under root `source/tools/` paths, for example:

- `source/tools/OdeSolverFactory.h`
- `source/tools/DormandPrince54OdeSolver.h`
- `source/tools/DiffusionMethodOfLinesSystem.h`

Therefore, a literal copy from `2026-1` to `WorkInProgress` would partially reintroduce the older layout and duplicate or bypass the current `source/tools/Continuous/` organization. The next implementation step should adapt the accepted solver/factory headers into `source/tools/Continuous/` or deliberately add compatibility wrapper headers after an explicit architectural decision.

## Recommended selective port

### Tema 8.1 — recommended files to port from current `2026-1`

Port after adapting includes to the current `WorkInProgress` layout:

- `source/plugins/data/Continuous/ODESolver.h`
- `source/plugins/data/Continuous/ODESolver.cpp`
- `source/plugins/components/Continuous/ContinuousSystemComponent.h`
- `source/plugins/components/Continuous/ContinuousSystemComponent.cpp`
- selected changes in `source/plugins/components/Continuous/LSODE.h`
- selected changes in `source/plugins/components/Continuous/LSODE.cpp`
- selected changes in `source/plugins/components/Continuous/DiffEquations.h`
- selected changes in `source/plugins/components/Continuous/DiffEquations.cpp`
- registration additions in `source/plugins/PluginConnectorDummyImpl1.cpp`, merged manually into the current `WorkInProgress` file so that AI, WCM, Python, biochemical, and other newer entries are preserved.

Do not port from old ZIP if it conflicts with current `2026-1`.

### Tema 8.2 — recommended files to port from current `2026-1`

Port after adapting the tools layout:

- `source/tools/DiffusionMethodOfLinesSystem.h` -> likely `source/tools/Continuous/DiffusionMethodOfLinesSystem.h`
- `source/tools/DormandPrince54OdeSolver.h` -> likely `source/tools/Continuous/DormandPrince54OdeSolver.h`
- `source/tools/OdeSolverFactory.h` -> likely `source/tools/Continuous/OdeSolverFactory.h`
- `source/plugins/data/Continuous/DiffusionField.h`
- `source/plugins/data/Continuous/DiffusionField.cpp`
- `source/plugins/components/Continuous/DiffusionSimulate.h`
- `source/plugins/components/Continuous/DiffusionSimulate.cpp`
- `source/tests/unit/test_tools_ode_solver_factory.cpp`
- `source/tests/unit/test_tools_diffusion_mol.cpp`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`, only if the plugin registration and data/component files are ported together.
- CMake registrations in `source/tests/unit/CMakeLists.txt`, merged manually with the current `WorkInProgress` file.

## Explicitly rejected for this integration

Do not bring:

- root `README.md` replacements from academic submissions;
- `relatorio.pdf`;
- `video_entrega.mp4`;
- ZIP files or Moodle submission artifacts;
- `__temporary_check`;
- unrelated optimization files from the same `2026-1` ahead-set;
- broad `.github/workflows/genesys-ci.yml` changes unless CI policy is intentionally being synchronized;
- historical `documentation/` paths as-is; if technical handoff is still valuable, consolidate into `docs/ai_assistants/`.

## Implementation recommendation

Because the current target branch has a newer architecture, do not use a mechanical branch-level cherry-pick or a blind file-copy operation. Use one of two approaches:

1. Local implementation preferred:
   - checkout `WorkInProgress`;
   - create a topic branch;
   - copy current accepted files from `2026-1`;
   - move/adapt `source/tools/*Ode*` headers into `source/tools/Continuous/`;
   - update includes in plugins and tests;
   - manually merge `PluginConnectorDummyImpl1.cpp` additions;
   - manually merge `source/tests/unit/CMakeLists.txt` additions;
   - run CMake/Ninja/CTest locally before opening PR.

2. GitHub-connector-only implementation:
   - create branch from `WorkInProgress`;
   - apply only small, manually reviewed files per commit;
   - avoid large source-file replacement when it risks dropping newer `WorkInProgress` changes;
   - open PR as draft and rely on GitHub Actions for CI.

This diagnostic branch followed approach 2 only up to documentation because applying large code files safely through the connector would require manual adaptation of multiple large files and CMake merge points without local build feedback.

## Validation performed

Validated by GitHub connector inspection only:

- repository metadata;
- existence and usability of refs `2026-1` and `WorkInProgress` through file fetch and compare;
- current merged state of PR #425 and PR #437;
- current open/non-merged state of PR #439;
- current `WorkInProgress` AI-assistant documentation;
- current `WorkInProgress` tool layout under `source/tools/Continuous`;
- current `WorkInProgress` unit-test CMake structure;
- current `2026-1` ODE/PDE file presence through compare and file fetch.

No local build was run. No CMake/Ninja/CTest execution was possible from the ChatGPT Web/GitHub connector environment.

## Risks remaning

- Solver/factory headers from `2026-1` must be adapted to the `WorkInProgress` `source/tools/Continuous/` layout.
- `PluginConnectorDummyImpl1.cpp` must be merged manually; replacing the file with the `2026-1` version would drop WCM, AI, Python, biochemical, and other newer registrations present in `WorkInProgress`.
- `source/tests/unit/CMakeLists.txt` must be merged manually; replacing it with the `2026-1` version would regress GUI path updates, worker application changes, WCM tests, generated method inventory tests, and other WorkInProgress-only test structure.
- Continuous-time semantics still need review: both DCS themes touch hybrid discrete/continuous simulation, and the current documentation requires explicit tests for time-step and event-calendar interaction.
- The accepted GitHub state has priority over older evaluation reports, but the older reports still identify conceptual risks that remain valid until tested.

## Next steps

1. Decide whether `OdeSolverFactory`, `DormandPrince54OdeSolver`, and `DiffusionMethodOfLinesSystem` should live under `source/tools/Continuous/` in `WorkInProgress`.
2. Port the three Tema 8.2 header-only numerical files first, adapting includes.
3. Add and register `test_tools_ode_solver_factory.cpp` and `test_tools_diffusion_mol.cpp` in `source/tests/unit/CMakeLists.txt`.
4. Validate the tools-only tests in CI or local CMake/Ninja.
5. Port `DiffusionField` and `DiffusionSimulate` next, then add plugin-level tests.
6. Port Tema 8.1 `ODESolver` and `ContinuousSystemComponent` after the tools layout is stable.
7. Manually merge plugin registrations into `PluginConnectorDummyImpl1.cpp` without dropping newer `WorkInProgress` entries.
8. Run full `tests-unit` and targeted continuous/hybrid tests before merging into `WorkInProgress`.
