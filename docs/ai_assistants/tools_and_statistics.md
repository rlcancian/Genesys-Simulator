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

- its own folder under the GUI tools area;
- a main window;
- menus and toolbar;
- a clear workflow;
- parameter dialogs before major analyses;
- separation between GUI prototype and backend numerical/tool classes.

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

## Validation checklist

For tools/statistics changes, prefer this order:

1. Run unit-test validation.
2. Add numerical tests for algorithmic changes.
3. Validate GUI startup if the tool UI changed.
4. Validate import/export behavior with small datasets.
5. Validate edge cases: empty data, one observation, repeated values, non-finite values, and incompatible dataset groups.

## Open follow-up tasks

- Inventory current Data Analyzer and Optimizer source files.
- Separate GUI prototype code from backend numerical/statistical classes.
- Add tests for exploratory statistics and distribution fitting.
- Define a precise policy for chi-square and Kolmogorov-Smirnov diagnostics.
- Define how Genesys simulation responses become datasets in an Analysis Study.
