# GenESyS Tests

This directory contains automated tests for GenESyS. The data-analysis tool is covered by both unit tests and integration tests.

## Data-Analysis Unit Tests

Unit tests for `source/tools/analysis` are located in `source/tests/unit/tools/analysis`.

| Test file | Scope |
| --- | --- |
| `test_tools_dataanalyser.cpp` | `DataAnalyserDefaultImpl` facade behavior, dataset propagation, summary, histogram, boxplot, unsupported roadmap hooks. |
| `test_tools_dataset_loader.cpp` | Text, whitespace and binary dataset loading; statistics; invalid input handling. |
| `test_tools_simulation_results_dataset.cpp` | Raw numeric, legacy record, enriched record and GUI-tabular simulation result parsing. |
| `test_fitter_distributions.cpp` | Distribution fitting, error handling, ranking and structured fit summaries. |
| `test_tools_hypothesistester.cpp` | Confidence intervals, parametric tests, chi-square, Kolmogorov-Smirnov, file-based overloads and reference-value checks. |

Run only the tools unit tests with Makefile target:

```sh
make run-unit-tests PACKAGE=tools
```

Latest recorded result:

```text
Date: 2026-06-22
Command: make run-unit-tests PACKAGE=tools
Result: 95/95 tests passed
```

## Data-Analysis Integration Tests

Integration tests for `source/tools/analysis` are located in `source/tests/integration/tools/analysis`.

| Test file | Scope |
| --- | --- |
| `test_analysis_tool_integration.cpp` | End-to-end use of `DataAnalyserDefaultImpl` with bundled example datasets, validating dataset loading, descriptive structures, fitting, confidence intervals, hypothesis tests, goodness-of-fit tests and consistency between file, memory and file-based overloads. |

Run only the tools integration tests with Makefile target:

```sh
make run-integration-tests PACKAGE=tools
```

Latest recorded result:

```text
Date: 2026-06-22
Command: make run-integration-tests PACKAGE=tools
Result: 2/2 tests passed
```

## Example Regression Checks

The analysis example is a runnable integration-style demonstration and includes deterministic regression checks over the bundled datasets.

```sh
make run-examples
```

Latest recorded result:

```text
Date: 2026-06-22
Command: make run-examples
Result: Regression result: ALL CHECKS PASSED
```
