# Oldies Review Status

## Purpose

This file tracks the consolidation status of historical documents under `docs/ai_assistants/oldies/`.

The existence of a stable guide does not automatically make the source historical file deletion-ready. A historical file becomes deletion-ready only after explicit review.

## Status legend

- `guide-created`: an initial stable guide exists for the theme.
- `needs-review`: the historical file still needs detailed review before deletion.
- `consolidated`: useful content has been moved or intentionally summarized.
- `obsolete`: content appears obsolete but still needs explicit deletion approval.
- `discard-after-review`: safe to delete after review and deletion gate.

## Current matrix

| Historical file | Target stable guide | Status |
|---|---|---|
| `old_kernel-tests-build-roadmap-2026-1.md` | `build_ci_tests.md` | `guide-created`, `needs-review` |
| `old_terminal-build-strategy-2026-04-01.md` | `build_ci_tests.md`, `applications_development.md` | `guide-created`, `needs-review` |
| `old_kernel-cpp23-modernization-audit.md` | `kernel_development.md` | `guide-created`, `needs-review` |
| `old_kernel-memory-leak-review-2026-03-30.md` | `kernel_development.md` | `guide-created`, `needs-review` |
| `old_modelsimulation-cpp23-review.md` | `kernel_development.md` | `guide-created`, `needs-review` |
| `old_phase2-kernel-simulator-inventory.md` | `kernel_development.md` | `guide-created`, `needs-review` |
| `old_plugin_components_method_matrix.md` | `plugins_development.md`, `plugins/other_plugins.md` | `guide-created`, `needs-review` |
| `old_plugin_data_definitions_audit_WiP20261.md` | `plugins_development.md`, plugin domain guides | `guide-created`, `needs-review` |
| `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md` | `plugins_development.md`, GUI/application guidance | `guide-created`, `needs-review` |
| `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md` | `plugins_development.md`, biochemical/GUI guidance | `guide-created`, `needs-review` |
| `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md` | `plugins_development.md` | `guide-created`, `needs-review` |
| `old_web-application-roadmap-2026-03-30.md` | `applications_development.md` | `guide-created`, `needs-review` |
| `old_tools-data-analyzer-plan-2026-04-16.md` | `tools_and_statistics.md` | `guide-created`, `needs-review` |
| `old_data-analyzer-analysis-study-design-2026-04-16.md` | `tools_and_statistics.md` | `guide-created`, `needs-review` |
| `old_optimizer-workstation-roadmap-2026-04-16.md` | `tools_and_statistics.md` | `guide-created`, `needs-review` |
| `old_modal_model_efsm_petrinet_plan.md` | `modal_and_hybrid_simulation.md`, `plugins/modal_model.md` | `guide-created`, `needs-review` |
| `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md` | `modal_and_hybrid_simulation.md`, `plugins/continuous_hybrid.md` | `guide-created`, `needs-review` |
| `old_SBML_INTEROPERABILITY_SCOPE.md` | `whole_cell_and_sbml.md`, `plugins/biochemical.md` | `guide-created`, `needs-review` |
| `old_TINKERCELL_context.md` | `whole_cell_and_sbml.md` | `guide-created`, `needs-review` |
| `old_whole_cell_biosimulator_project.md` | `whole_cell_and_sbml.md`, `plugins/whole_cell_model.md` | `guide-created`, `needs-review` |
| `old_WCM_IMPLEMENTATION_PLAN.md` | `whole_cell_and_sbml.md`, `plugins/whole_cell_model.md` | `guide-created`, `needs-review` |
| `old_PythonForG_SimulatorFacade_coverage.md` | `python_integration.md`, `plugins/external_integration.md` | `guide-created`, `needs-review` |
| `old_report-issue-relay-contract.md` | `documentation_governance.md` | `guide-created`, `needs-review` |
| `old_post-refactor-validation-report.md` | `documentation_governance.md`, `build_ci_tests.md` | `guide-created`, `needs-review` |
| `old_genesys_wiki_consolidated.md` | multiple guides | `needs-review` |

## Deletion rule

No file listed here should be deleted only because a guide exists. Delete only after detailed review and after the oldies deletion gate described in `documentation_governance.md` and `current_plans.md`.
