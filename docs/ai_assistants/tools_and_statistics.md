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

Planned GUI frontend mapping:

```text
source/tools/Statistics/        -> source/applications/gui/dataanalyser/
source/tools/Optimization/      -> source/applications/gui/optimizer/
source/tools/AIAssistant/       -> source/applications/gui/ai_assistant/
source/tools/FactorialDesign/   -> source/applications/gui/doexperiments/
```

The HTTP/background worker is not a statistical/tool backend. Its runtime belongs to the application layer, and its graphical control frontend belongs under the GUI applications area:

```text
source/applications/worker/     -> source/applications/gui/httpworker/
```

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

The professor may provide bibliographic references, thesis material, equations, algorithms, datasets, source code, and benchmark results. Those materials are preferred when they define the intended GenESyS behavior.

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

Status:

- validation framework: `decided`;
- authoritative bibliography per method: `needs-human-decision`.

See `genesys_2026_human_decisions.md` for the complete record.

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

## Optimizer maturity policy

User-visible maturity describes what GenESyS claims the Optimizer can reliably do. A class compiling or a GUI starting does not establish algorithmic maturity.

Maturity levels:

1. **Internal scaffold**: interfaces/UI may exist, but no functioning optimization claim is permitted.
2. **Research prototype / experimental**: real algorithm executes end-to-end with explicit assumptions and selected benchmark validation.
3. **Beta**: intended workflow, reproducibility, cancellation, persistence, reporting, broader benchmarks, and realistic models are covered.
4. **Stable user feature**: supported algorithms/problem classes, correctness, performance, error behavior, versioning, and result semantics are documented and regression-tested.

Current classification:

- `OptimizerDefaultImpl1`: internal scaffold / `partially-implemented`;
- stable user-facing optimizer claim: not authorized;
- initial real algorithm: `needs-human-decision`.

The professor's multiobjective optimization techniques from his doctoral research are a preferred candidate source. Before implementation, record the relevant thesis chapters/papers, algorithm/pseudocode/source, licensing/ownership, supported variables/objectives/constraints, simulation-noise policy, archive/dominance/diversity rules, stopping criteria, reproducibility policy, and benchmark problems.

Recommended implementation sequence:

1. define backend-neutral contracts for decision variables, model mutation, simulation evaluation, objective/constraint evaluation, algorithm state, archive, cancellation, reporting, and persistence;
2. select one reference algorithm from the professor's research;
3. unit-test dominance, constraints, archives, operators/search steps, and stopping rules;
4. validate against declared thesis/reference benchmarks;
5. add stochastic simulation replication/noise handling;
6. expose it as a research prototype only after end-to-end validation;
7. add additional algorithms behind the same backend interface.

See `genesys_2026_human_decisions.md` for the detailed maturity and input checklist.

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

Optimizer and AI Assistant require additional context-handoff review before extraction because they depend more directly on the current simulator/model context, provider configuration, secret handling, or backend object lifetime assumptions.

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

## Open follow-up tasks

- Inventory current Data Analyzer and Optimizer source files.
- Separate GUI prototype code from backend numerical/statistical classes.
- Add reference-backed tests for exploratory statistics and distribution fitting.
- Define a precise policy for chi-square and Kolmogorov-Smirnov diagnostics.
- Collect professor-approved numerical/statistical bibliography and benchmark values.
- Define how Genesys simulation responses become datasets in an Analysis Study.
- Define model/context handoff for standalone Data Analyser and Optimizer executions.
- Select and specify the first real optimization algorithm, preferably from the professor's multiobjective research.
- Define the future Do Experiments GUI workflow on top of FactorialDesign.
