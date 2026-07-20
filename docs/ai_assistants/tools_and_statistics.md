# Tools and Statistics Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS tools, data analysis, optimization, and statistical workflows.

Historical notes from `oldies/` must be checked against current source files before being treated as current implementation facts.

## Scope

Primary conceptual areas:

- Data Analyzer GUI tool.
- Optimizer GUI tool.
- datasets and analysis studies.
- exploratory statistics.
- distribution fitting.
- input analysis and output analysis.
- DOE, RSM, ANOVA, desirability, and optimization workflows.

Historical source documents:

- `old_tools-data-analyzer-plan-2026-04-16.md`
- `old_data-analyzer-analysis-study-design-2026-04-16.md`
- `old_optimizer-workstation-roadmap-2026-04-16.md`

## Architecture direction

Tools should evolve toward standalone-leaning workstations launched from the GUI, not small modal dialogs.

For each major tool, prefer:

- its own folder under the GUI applications area;
- a main window;
- menus and toolbar;
- a clear workflow;
- parameter dialogs before major analyses;
- separation between GUI prototype and backend numerical/tool classes.

The current architectural direction is:

- `source/tools/` contains reusable tool backends and algorithms;
- `source/applications/gui/<tool-or-application>/` contains graphical frontends;
- the main GenESyS GUI should eventually launch independent graphical frontends instead of owning every tool window directly;
- process boundaries require explicit model/context handoff rather than raw pointer sharing.

## GUI frontend mapping

```text
source/tools/Statistics/        -> source/applications/gui/dataanalyser/
source/tools/Optimization/      -> source/applications/gui/optimizer/
source/tools/AIAssistant/       -> source/applications/gui/ai_assistant/
source/tools/FactorialDesign/   -> source/applications/gui/doexperiments/
source/applications/worker/     -> source/applications/gui/httpworker/
```

The worker is an application runtime, not a statistical/tool backend.

## Data Analyzer concepts

The central concept is the dataset: numeric observations plus metadata.

Recommended dataset metadata includes:

- dataset name;
- random-variable name;
- random-variable description;
- random-variable type;
- source;
- raw numeric observations.

The grouping concept is `Analysis Study`: a collection of one or more datasets analyzed together.

Analyses should support both one selected dataset and all datasets pooled together when statistically valid.

## Statistical correctness policy

Statistical functionality must be treated as correctness-sensitive.

Before implementing or changing statistical algorithms:

- define assumptions;
- define valid input domains;
- define behavior for small samples and degenerate samples;
- avoid misleading p-values or goodness-of-fit claims;
- document numerical limitations;
- add regression tests with known expected results.

## Authoritative numerical/statistical reference policy

Decision date: 2026-07-20.

An authoritative reference is the declared oracle for the exact mathematical formulation and parameterization implemented by GenESyS. Another software package may corroborate a result, but it is not automatically the specification.

Validation evidence should be ordered as follows:

1. analytical invariants or closed-form solutions;
2. declared primary bibliographic/standards reference;
3. independent high-quality implementation used as a cross-check;
4. published or curated benchmark datasets;
5. property, convergence, conservation, metamorphic, and randomized tests;
6. regression fixtures preserving previously validated behavior.

For each algorithm, document:

- definition and parameterization;
- valid domain and preconditions;
- units/scaling;
- small-sample and degenerate-case policy;
- numerical method and stopping criteria;
- absolute/relative tolerances and their rationale;
- overflow, underflow, non-finite, and error behavior;
- exact citation and expected values;
- comparator implementation/version when used;
- deterministic seeds and reproducibility conditions for stochastic methods.

R, SciPy, Boost.Math, GNU Scientific Library, or another independent package may be used as a comparator, but the test/documentation must state whether the package is normative or merely corroborating.

Current status:

- validation framework: `decided`;
- bibliography, PDFs, converted material, datasets, parameterizations, and expected results: `deferred` for a future acquisition round;
- provisional methods must not be reclassified as scientifically validated without a declared reference package.

See:

- `genesys_numerical_statistical_references_plan.md`;
- `genesys_2026_decisions_addendum_20260720.md`.

## DOE and optimization policy

DOE/RSM workflows should remain explicit about what is implemented and what is only a preview.

A robust flow should eventually include:

- design selection or construction;
- design-quality diagnostics;
- experiment run sheet or import path;
- response-surface model fitting;
- ANOVA and lack-of-fit checks;
- residual and influence diagnostics;
- desirability functions;
- candidate solution search and confirmation.

## Project maturity policy

The project-wide target is:

1. bring all supported functionality to at least **Level 3 — Beta**;
2. after the supported set reaches Level 3, promote prioritized functionality to **Level 4 — Stable user feature**.

Levels 1 and 2 remain accurate diagnostic states for current incomplete functionality, but they are not accepted final delivery states for supported features.

A feature may be removed from the supported set or explicitly deferred instead of being falsely labelled Beta.

## Optimizer maturity and research direction

Maturity levels:

1. **Internal scaffold**: interfaces/UI may exist, but no functioning optimization claim is permitted.
2. **Research prototype / experimental**: real algorithm executes end-to-end with explicit assumptions and selected benchmark validation.
3. **Beta**: intended workflow, reproducibility, cancellation, persistence, reporting, broader benchmarks, and realistic models are covered.
4. **Stable user feature**: supported algorithms/problem classes, correctness, performance, error behavior, versioning, and result semantics are documented and regression-tested.

Current classification:

- `OptimizerDefaultImpl1`: internal scaffold / `partially-implemented`;
- Level 3 claim: not yet authorized;
- minimum target for supported Optimizer functionality: Level 3;
- later prioritized target: Level 4.

Initial future research will cover:

- evolutionary multiobjective optimization;
- Pareto dominance and external archives;
- hypervolume and hypervolume contribution;
- ETH Zürich / Eckart Zitzler research lineage;
- PISA;
- HypE;
- SPEA/SPEA2;
- IBEA;
- ZDT and related benchmarks;
- statistical performance assessment of stochastic optimizers.

The professor's doctoral multiobjective optimization techniques, code, references, and benchmark results are preferred inputs for the first GenESyS algorithm implementation.

Recommended implementation sequence:

1. acquire thesis/research material and external references;
2. define backend-neutral contracts for decisions, model mutation, simulation evaluation, objectives, constraints, archive, indicators, cancellation, reporting, and persistence;
3. unit-test dominance, constraints, archive behavior, indicators, operators/search steps, and stopping rules;
4. implement one real reference algorithm;
5. validate against declared benchmarks;
6. add stochastic simulation replication/noise handling;
7. integrate realistic GenESyS model evaluation;
8. complete the Level 3 GUI/workflow;
9. only then prioritize Level 4 algorithms/workflows.

See:

- `genesys_multiobjective_optimizer_future_plan.md`;
- `genesys_2026_decisions_addendum_20260720.md`.

## GUI extraction policy

When extracting a GUI tool into its own graphical application:

- first preserve existing behavior inside the main GUI;
- then create the standalone graphical executable;
- then change the main GUI to launch that executable through the application-launching service;
- only then remove direct widget ownership from the main GUI;
- keep backend numerical changes separate from GUI folder moves;
- add tests for backend algorithms before changing statistical behavior;
- validate import/export and startup behavior independently from model-editor behavior.

Data Analyser is the preferred first tool extraction candidate because file/dataset workflows can be validated without requiring full live-model process sharing.

Optimizer and AI Assistant require additional context-handoff review because they depend directly on simulator/model context, provider configuration, secret handling, or backend lifetime assumptions.

Do Experiments should be introduced only after the FactorialDesign workflow and GUI requirements are specified.

## Validation checklist

For tools/statistics changes, prefer this order:

1. Run unit-test validation.
2. Add reference-backed numerical tests before algorithmic changes.
3. Validate analytical invariants, convergence/order, and edge cases.
4. Validate GUI startup if the tool UI changed.
5. Validate import/export behavior with small datasets.
6. Validate empty data, one observation, repeated values, non-finite values, incompatible dataset groups, and invalid parameters.
7. Record comparator versions, tolerances, and exact reference provenance.
8. For stochastic algorithms, record seed, replication, confidence, and noise-handling policy.
9. Verify the Level 3 acceptance checklist before labelling a supported workflow Beta.

## Open follow-up tasks

- Inventory current Data Analyzer and Optimizer source files.
- Separate GUI prototype code from backend numerical/statistical classes.
- Execute `genesys_numerical_statistical_references_plan.md` when the professor can provide source material.
- Add reference-backed tests for exploratory statistics and distribution fitting.
- Define a precise policy for chi-square and Kolmogorov–Smirnov diagnostics.
- Define how GenESyS simulation responses become datasets in an Analysis Study.
- Define model/context handoff for standalone Data Analyser and Optimizer executions.
- Execute the research and architecture phases in `genesys_multiobjective_optimizer_future_plan.md`.
- Define the future Do Experiments GUI workflow on top of FactorialDesign.
