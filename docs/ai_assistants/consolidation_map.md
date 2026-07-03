# Historical Documentation Consolidation Map

## Purpose

This file maps historical documents in `docs/ai_assistants/oldies/` to future stable documentation under `docs/ai_assistants/`.

The goal is to avoid losing useful technical analysis while preventing `oldies/` from becoming a permanent archive of stale plans.

## Consolidation rules

1. Inspect the source document before moving any claim into a stable document.
2. Revalidate technical claims against the current branch before presenting them as current facts.
3. Preserve useful historical context only when it affects current architecture, build, tests, packaging, or migration decisions.
4. Do not consolidate broad TODOs without converting them into specific, testable follow-up tasks.
5. Keep stable documents short enough to remain operational for future AI assistants.

## Proposed stable documents

### `build_ci_tests.md`

Purpose: stable reference for CMake/Ninja/CTest presets, terminal-app build strategy, smoke/unit test execution, and CI follow-up.

Candidate sources:

- `old_kernel-tests-build-roadmap-2026-1.md`
- `old_terminal-build-strategy-2026-04-01.md`
- relevant sections of `old_post-refactor-validation-report.md`

### `kernel_development.md`

Purpose: stable reference for kernel modernization, simulator inventory, memory-safety risks, ownership conventions, and C++23 modernization strategy.

Candidate sources:

- `old_kernel-cpp23-modernization-audit.md`
- `old_kernel-memory-leak-review-2026-03-30.md`
- `old_modelsimulation-cpp23-review.md`
- `old_phase2-kernel-simulator-inventory.md`

### `plugins_development.md`

Purpose: stable reference for plugin architecture, component method coverage, data definitions, graphical plugin coupling, and plugin migration risks.

Candidate sources:

- `old_plugin_components_method_matrix.md`
- `old_plugin_data_definitions_audit_WiP20261.md`
- `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md`
- `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md`
- `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md`

### `applications_development.md`

Purpose: stable reference for terminal, web/httpworker, GUI, MCP, and application-structure migration decisions.

Candidate sources:

- `old_web-application-roadmap-2026-03-30.md`
- `old_terminal-build-strategy-2026-04-01.md`
- relevant sections of `old_genesys_wiki_consolidated.md`

### `tools_and_statistics.md`

Purpose: stable reference for data analyzer, optimizer, statistical analysis, experimental design, and future analytical tooling.

Candidate sources:

- `old_tools-data-analyzer-plan-2026-04-16.md`
- `old_data-analyzer-analysis-study-design-2026-04-16.md`
- `old_optimizer-workstation-roadmap-2026-04-16.md`

### `modal_and_hybrid_simulation.md`

Purpose: stable reference for modal models, EFSM/Petri net planning, temporal synchronization, and discrete-continuous integration risks.

Candidate sources:

- `old_modal_model_efsm_petrinet_plan.md`
- `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md`
- relevant sections of whole-cell and SBML planning documents if they affect temporal semantics.

### `whole_cell_and_sbml.md`

Purpose: stable reference for SBML interoperability, TinkerCell context, whole-cell biosimulator planning, and biological modeling integration.

Candidate sources:

- `old_SBML_INTEROPERABILITY_SCOPE.md`
- `old_TINKERCELL_context.md`
- `old_whole_cell_biosimulator_project.md`
- `old_WCM_IMPLEMENTATION_PLAN.md`

### `python_integration.md`

Purpose: stable reference for PythonForG and Python-facing simulator facade coverage.

Candidate sources:

- `old_PythonForG_SimulatorFacade_coverage.md`

### `documentation_governance.md`

Purpose: stable reference for documentation migration, historical retention, issue/relay contracts, and AI-assistant update rules.

Candidate sources:

- `old_report-issue-relay-contract.md`
- `old_post-refactor-validation-report.md`
- relevant sections of `old_genesys_wiki_consolidated.md`
- `oldies_inventory.md`

## Suggested consolidation order

1. `build_ci_tests.md`
2. `kernel_development.md`
3. `plugins_development.md`
4. `applications_development.md`
5. `tools_and_statistics.md`
6. `modal_and_hybrid_simulation.md`
7. `whole_cell_and_sbml.md`
8. `python_integration.md`
9. `documentation_governance.md`

## Deletion gate for `oldies/`

Do not delete `oldies/` until all of the following are true:

1. Every listed historical file has been reviewed.
2. Useful content has been moved to a stable document or explicitly marked obsolete.
3. Stable documents avoid stale branch/date assumptions unless clearly identified as historical.
4. A final search confirms that no active README or plan still points to `oldies/` as the primary source of current guidance.
