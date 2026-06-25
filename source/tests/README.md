# GenESyS Tests

This directory contains automated tests for GenESyS. The data-analysis tool is covered by both unit tests and integration tests.

Tests are regular CMake/CTest targets and can be built or executed from QtCreator after configuring the project with tests enabled. The Makefile commands below are convenience shortcuts used by this repository to run focused subsets from the terminal.

## Data-Analysis Unit Tests

Unit tests for `source/tools/analysis` are located in `source/tests/unit/tools/analysis`.

| Test file | Scope |
| --- | --- |
| `test_tools_dataanalyser.cpp` | `DataAnalyserDefaultImpl` facade behavior, dataset propagation, summary, histogram, boxplot, unsupported roadmap hooks. |
| `test_tools_dataset_loader.cpp` | Text, whitespace and binary dataset loading; statistics; invalid input handling. |
| `test_tools_simulation_results_dataset.cpp` | Raw numeric, legacy record, enriched record and GUI-tabular simulation result parsing. |
| `test_fitter_distributions.cpp` | Distribution fitting, error handling, ranking and structured fit summaries. |
| `test_tools_hypothesistester.cpp` | Confidence intervals, parametric tests, chi-square, Kolmogorov-Smirnov, file-based overloads and reference-value checks. |

When using the repository Makefile, run only the tools unit tests with:

```sh
make run-unit-tests PACKAGE=tools
```

Latest recorded result:

```text
Date: 2026-06-23
Makefile shortcut: make run-unit-tests PACKAGE=tools
Result: 96/96 tests passed on 2026-06-23
```

## Data-Analysis Integration Tests

Integration tests for `source/tools/analysis` are located in `source/tests/integration/tools/analysis`.

| Test file | Scope |
| --- | --- |
| `test_analysis_tool_integration.cpp` | End-to-end use of `DataAnalyserDefaultImpl` with bundled example datasets, validating dataset loading, descriptive structures, fitting, confidence intervals, hypothesis tests, goodness-of-fit tests and consistency between file, memory and file-based overloads. |

When using the repository Makefile, run only the tools integration tests with:

```sh
make run-integration-tests PACKAGE=tools
```

Latest recorded result:

```text
Date: 2026-06-23
Makefile shortcut: make run-integration-tests PACKAGE=tools
Result: 2/2 tests passed
```

## Example Regression Checks

The examples are runnable integration-style demonstrations. `analysis_tools_example.cpp` includes deterministic regression checks over the bundled datasets, while `simulation_analysis_example.cpp` builds a small GenESyS model, writes a `Record` result file and analyzes that simulation output with the analysis facade. They can be built from CMake/QtCreator through the examples target. When using the repository Makefile, run them with:

```sh
make run-examples
```

Latest recorded result:

```text
Date: 2026-06-23
Makefile shortcut: make run-examples
Result: Regression result: ALL CHECKS PASSED
Result: Simulation analysis example: SUCCESS
```
