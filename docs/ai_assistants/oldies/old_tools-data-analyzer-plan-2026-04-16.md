# Genesys Tools and Data Analyzer Plan

Date: 2026-04-16

This note summarizes the current AI-assisted plan for Genesys GUI tools.

## General Tools Direction

Genesys tools should progressively become standalone-leaning workstations launched from the GUI, not small modal dialogs.

The first two workstations following this pattern are:

- `DataAnalyzerWindow`
- `OptimizerWindow`

Each workstation should have:

- its own folder under `source/applications/gui/qt/GenesysQtGUI/tools/`;
- a main window;
- menus and toolbar;
- a left-to-right or tabbed workflow;
- parameter dialogs before running major analyses;
- clear separation between GUI prototype and backend numerical/tool classes.

## Data Analyzer Direction

The Data Analyzer should approximate professional statistical tools such as JMP and Stat-Ease Design-Expert while remaining focused on simulation output.

Core concepts:

- `Dataset`: numeric observations plus random-variable metadata.
- `Analysis Study`: a collection of one or more datasets analyzed together.
- default analysis scope: all datasets together.
- optional analysis scope: one selected dataset.

Implemented direction:

- dataset loading and metadata dialog;
- import of current Genesys simulation responses as a dataset;
- Analysis Study summary table comparing each dataset with the pooled study scope;
- exploratory statistics split into central tendency and dispersion/shape;
- histogram, raw data and moving-average previews;
- distribution fitting with SSE and approximate chi-square / Kolmogorov-Smirnov p-value diagnostics;
- DOE/RSM prototype with design setup, run sheet, model/ANOVA, graph previews and optimization preview;
- DOE advisor preview with design recommendation text, alpha setting, design-quality diagnostics and response desirability setup.

## DOE/RSM Next Step Detail

The DOE/RSM workflow should continue moving toward the Design-Expert pattern:

- build or choose a design;
- inspect design quality before running simulations;
- run or import the experiment matrix;
- fit a response-surface model;
- inspect ANOVA, lack of fit, residuals and influence diagnostics;
- tune desirability functions for each response;
- search and confirm optimal settings.

Current preview-only widgets now available:

- design family selector;
- model order selector;
- alpha setting;
- randomization option;
- design-quality diagnostics table;
- advisor recommendation label;
- desirability setup table;
- candidate solution table.

Backend work still pending:

- implement real design generation for selected factors;
- compute design metrics from the actual matrix;
- execute Genesys scenarios and replications for the run sheet;
- fit real regression/RSM models;
- compute real ANOVA, lack-of-fit, PRESS/predicted R-squared and adequate precision;
- compute desirability from fitted response predictions.

Important existing detailed note:

- `documentation/AI/data-analyzer-analysis-study-design-2026-04-16.md`

## Optimizer Direction

The Optimizer should become an OptQuest-like workstation.

Current detailed roadmap:

- `documentation/IA/optimizer-workstation-roadmap-2026-04-16.md`
