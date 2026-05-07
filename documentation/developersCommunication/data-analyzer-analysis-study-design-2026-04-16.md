# Data Analyzer: Analysis Study and Dataset Design Notes

Date: 2026-04-16

## Conversation Context

The Genesys GUI is evolving the `Results/Data Analyzer` menu entry into a larger statistical-analysis tool. The current direction is that this tool should behave like a professional statistical environment rather than like a small file-opening dialog.

The user clarified that the central concept is the `dataset`: a set of observations for a random variable. A dataset can be raw, meaning each row contains one observed value of a numeric random variable. The random variable may be continuous or discrete, but the first implementation can assume numeric values.

Each dataset should carry metadata in addition to the raw values:

- dataset name;
- random variable name;
- random variable description;
- random variable type, initially continuous numeric or discrete numeric;
- source, usually a file path, but it may also be a Genesys simulation result snapshot;
- raw numeric observations.

## Grouping Concept

The requested temporary word was `superset`, meaning a group of datasets analyzed together.

The adopted GUI term is `Analysis Study`.

Rationale:

- `Project` is common in tools such as Minitab and JMP, but it could be confused with a Genesys model/project.
- `Workbook` suggests spreadsheet semantics, which are too narrow for simulation output, fitting, inference, DOE and response surfaces.
- `Study` is natural in statistical, experimental and simulation contexts, and supports future DOE workflows.
- `Analysis Study` is explicit enough for the GUI while staying independent from Genesys model terminology.

## Rule for Analysis Scope

The general rule is that the Data Analyzer should always work on an `Analysis Study`, even when the study contains only one dataset.

Every analysis should support:

- viewing results for one selected dataset;
- viewing results for all datasets pooled together.

The GUI should expose this through an analysis-scope combobox. The combobox should list each dataset and include `All datasets together` as the last option. The default selection should be `All datasets together`.

## Menu Behavior

The top-level menu options should not merely jump to tabs. They should open a lightweight parameterization dialog first, then route the user to the relevant analysis tab.

The first parameterization dialog can include:

- selected analysis name;
- analysis scope;
- confidence level;
- later, analysis-specific parameters.

This is a placeholder contract for future richer dialogs.

## Exploratory Data Analysis Requirements

Exploratory statistics should be separated into at least:

- central tendency;
- dispersion and shape.

Central tendency should include:

- arithmetic mean;
- median;
- mode;
- minimum;
- maximum.

Dispersion and shape should include:

- variance;
- standard deviation;
- coefficient of variation;
- quartiles;
- interquartile range;
- kurtosis.

Exploratory views should also include:

- histogram;
- raw data table;
- moving averages.

## Distribution Fitting Requirements

Distribution fitting should continue to show:

- distribution name;
- fitted parameters;
- SSE or equivalent fit metric.

It should also add goodness-of-fit diagnostics:

- chi-square p-value;
- chi-square interpretation;
- Kolmogorov-Smirnov p-value;
- Kolmogorov-Smirnov interpretation.

The decision text should avoid the common phrasing "accept H0" or "reject H0". Instead, it should explain the risk using p-value wording, for example:

> The risk of rejecting this fit hypothesis is 0.073; suggest keeping this distribution.

This wording is intentionally pragmatic for users of the simulator.

## Current Implementation Status

Implemented in the first GUI iteration:

- `DataAnalyzerWindow` is now located under `source/applications/gui/qt/GenesysQtGUI/tools/dataanalyzer/`.
- The window owns an `Analysis Study`, represented by a list of dataset descriptors.
- Dataset loading asks for metadata after the numeric file is parsed.
- Simulation responses can be imported as a dataset snapshot.
- The scope combobox defaults to `All datasets together`.
- The Dataset tab includes a per-dataset and pooled Analysis Study summary table.
- Exploratory statistics are split into central tendency and dispersion/shape.
- Histogram, raw data preview and moving-average preview were added.
- Distribution fitting now shows p-value based goodness-of-fit diagnostics using approximate chi-square and Kolmogorov-Smirnov calculations where the CDF is available.
- Analysis menu actions open a parameterization dialog before selecting the corresponding tab.

## DOE/RSM Direction

The user explicitly likes JMP and especially Stat-Ease Design-Expert. The desired long-term direction is that Genesys DOE should feel closer to Design-Expert:

- design-building workflow for factors, responses, blocks and center points;
- run sheet generated from a design;
- model fitting and ANOVA;
- diagnostics and analysis summary;
- contour plots, 3D response surfaces, profilers and desirability views;
- numerical and graphical optimization for several responses simultaneously.

The current GUI prototype includes fake DOE/RSM data to make this workflow visible before the underlying execution engine is complete:

- Design Builder tab: factors and responses;
- Design Builder setup controls: design family, model order, replications, randomized/fixed run order, preview generation and a reserved Genesys execution action;
- workflow guidance table mapping each preview step to the future Genesys integration point;
- Design-Expert-like advisor label, alpha setting and design-quality diagnostics table;
- Run Sheet tab: demonstration experiment matrix;
- Model / ANOVA tab: demonstration quadratic model and ANOVA table;
- Graphs tab: contour, 3D surface, profiler, desirability and diagnostics previews;
- Optimization tab: response desirability setup and demonstration desirability-ranked solutions.

Pending design work:

- persist and reload complete Analysis Studies;
- support multiple variables per file;
- implement exact goodness-of-fit tests in the tools layer rather than GUI approximations;
- define formal interfaces for regression, ANOVA, DOE and response-surface workflows;
- decide whether Analysis Study should become a kernel/tool model class, not only a GUI model.
