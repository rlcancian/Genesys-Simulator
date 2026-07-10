# Oldies Inventory

## Purpose

This file inventories historical AI-assistant and migration documents temporarily retained under `docs/ai_assistants/oldies/`.

The inventory is intentionally conservative: it groups files by apparent theme based on file names and sampled content. Before deleting or rewriting any source file, inspect the file itself and validate whether its content is still relevant to the current branch.

## Retention policy

- `docs/ai_assistants/oldies/` is temporary.
- The directory and its contents should be removed after 2026-11-01 only after relevant content has been consolidated or explicitly discarded.
- Do not treat this inventory as proof that every historical recommendation is still valid.
- Treat historical technical claims as candidates to revalidate against current code.

## Known historical files

### Repository overview, migration, and governance

- `old_genesys_wiki_consolidated.md`
- `old_report-issue-relay-contract.md`
- `old_post-refactor-validation-report.md`

### Build, CI, tests, and terminal applications

- `old_kernel-tests-build-roadmap-2026-1.md`
- `old_terminal-build-strategy-2026-04-01.md`

### Kernel, simulator, C++ modernization, and memory safety

- `old_kernel-cpp23-modernization-audit.md`
- `old_kernel-memory-leak-review-2026-03-30.md`
- `old_modelsimulation-cpp23-review.md`
- `old_phase2-kernel-simulator-inventory.md`

### Plugins, component matrix, and data definitions

- `old_plugin_components_method_matrix.md`
- `old_plugin_data_definitions_audit_WiP20261.md`

### GUI, graphical plugins, SBML, TinkerCell, and whole-cell modeling

- `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md`
- `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md`
- `old_SBML_INTEROPERABILITY_SCOPE.md`
- `old_TINKERCELL_context.md`
- `old_whole_cell_biosimulator_project.md`
- `old_WCM_IMPLEMENTATION_PLAN.md`

### Tools, data analysis, optimization, and statistical workflows

- `old_tools-data-analyzer-plan-2026-04-16.md`
- `old_data-analyzer-analysis-study-design-2026-04-16.md`
- `old_optimizer-workstation-roadmap-2026-04-16.md`

### Modal models, temporal synchronization, and hybrid/discrete-continuous simulation

- `old_modal_model_efsm_petrinet_plan.md`
- `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md`

### Applications, web, and Python integration

- `old_web-application-roadmap-2026-03-30.md`
- `old_PythonForG_SimulatorFacade_coverage.md`

### Internal data-definition discussions

- `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md`

## Consolidation status legend

- `pending`: not yet reviewed after migration.
- `sampled`: sampled for theme only; not fully revalidated.
- `candidate`: should contribute to a stable document.
- `superseded`: likely obsolete, but requires confirmation before deletion.
- `discard`: safe to remove after explicit review.

## Current status

All files in this inventory are initially `pending` unless a future consolidation pass changes their status in a dedicated stable document.
