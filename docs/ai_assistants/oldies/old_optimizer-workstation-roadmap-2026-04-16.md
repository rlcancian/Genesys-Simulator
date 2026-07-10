# Genesys Optimizer Workstation Roadmap

Date: 2026-04-16

This note persists the agreed direction for the Genesys Optimizer GUI so future AI sessions can resume the work with context.

## Product Direction

The Optimizer should evolve into a standalone-leaning workstation, similar in spirit to OptQuest in Arena, but integrated with the current Genesys model.

The GUI should support:

- collecting `SimulationControl` objects from the current model;
- collecting `SimulationResponse` objects from the current model;
- selecting which controls are decision variables;
- selecting which responses are monitored KPIs;
- defining optimization objectives with parser expressions;
- defining feasibility constraints with parser expressions;
- configuring the optimization technique and execution limits;
- starting, pausing, resuming, stepping and stopping optimization runs;
- monitoring objective progress over iterations;
- showing retained best solutions and, later, Pareto fronts for multiobjective techniques;
- persisting and loading complete optimization studies.

## Architecture Direction

The GUI application should not itself be the optimization algorithm.

The intended separation is:

- `OptimizerWindow`: rich workstation GUI and study editor.
- `Optimizer_if`: stable interface for simulation-based optimization engines.
- concrete optimizer implementations: algorithm/service classes such as genetic algorithm, hill climbing, particle swarm, NSGA-II, etc.
- parser/model bridge: evaluates objective and constraint expressions against controls, responses and model data.
- experiment/scenario bridge: applies candidate control values, executes replications, collects responses and reports candidate quality.

## Implementation Status After First Step

Implemented in `source/applications/gui/qt/GenesysQtGUI/tools/optimizer/`:

- new `OptimizerWindow` main window;
- main menu and toolbar;
- tabs for start, controls, responses, objectives, constraints, technique/settings, run monitor and report;
- collection of controls and responses from the current model through `OptimizerDefaultImpl1`;
- selection tables for controls and responses;
- editable lower/upper bound metadata for controls;
- objective creation dialog;
- constraint creation dialog;
- technique selector with Genetic Algorithm, Hill Climbing, Particle Swarm and multiobjective GA preview entries;
- execution settings mapped to `Optimizer_if::OptimizationSettings`;
- run lifecycle buttons connected to `OptimizerDefaultImpl1`;
- simple progress plot fed by safe preview values while the backend still has no real algorithm;
- menu action `Simulator/Optimizer` now launches the workstation instead of the old modal dialog.

Current limitation:

- `OptimizerDefaultImpl1` is still a lifecycle/configuration scaffold. It validates settings and advances iterations, but does not yet evaluate candidate solutions, mutate controls, run model replications, or retain real best solutions.

## Next Steps

1. Define an `OptimizationTechnique_if` or equivalent service abstraction.
2. Implement a minimal concrete genetic algorithm technique that can work on numeric control bounds.
3. Add a parser evaluation bridge for objective and constraint expressions.
4. Add a model execution bridge that applies a candidate, runs the requested replications and collects selected responses.
5. Store candidate snapshots: control values, response values, objective values, constraint status and replication summaries.
6. Replace the GUI-only progress preview with real objective history from the backend.
7. Populate best solutions from the backend and allow applying a selected solution back to the model.
8. Add persistence for Optimization Study files.
9. Add multiobjective views: Pareto table, Pareto scatter plot, objective tradeoff matrix and desirability view.
10. Add distributed/parallel evaluation hooks once the simulation parallelization plan matures.

## Resume Prompt

When resuming this plan, start by reading this file, then inspect:

- `source/applications/gui/qt/GenesysQtGUI/tools/optimizer/OptimizerWindow.h`
- `source/applications/gui/qt/GenesysQtGUI/tools/optimizer/OptimizerWindow.cpp`
- `source/tools/Optimizer_if.h`
- `source/tools/OptimizerDefaultImpl1.h`
- `source/tools/OptimizerDefaultImpl1.cpp`
- `source/kernel/simulator/SimulationControlAndResponse.h`
