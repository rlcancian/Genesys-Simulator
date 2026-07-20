# GenESyS AI Assistant Documentation

This directory is the entrypoint for AI-assisted technical documentation for the GenESyS Simulator.

Read this file before using or updating the other files in this directory.

## Current layout

- `README.md`: entrypoint for AI assistants.
- `current_plans.md`: active plans and migration notes.
- `build_ci_tests.md`: stable build, CI, CMake/Ninja, and CTest guidance.
- `kernel_development.md`: stable kernel modernization and ownership guidance.
- `plugins_development.md`: plugin development entrypoint.
- `plugins/`: plugin-domain guides grouped by category.
- `applications_development.md`: terminal, web, GUI, and application-structure guidance.
- `tools_and_statistics.md`: statistical tools, data analyzer, optimizer, and DOE/RSM guidance.
- `modal_and_hybrid_simulation.md`: modal models and hybrid discrete-continuous simulation guidance.
- `whole_cell_and_sbml.md`: whole-cell, biochemical, and SBML guidance.
- `python_integration.md`: Python-facing integration guidance.
- `documentation_governance.md`: documentation retention and governance guidance.
- `branch_workflow.md`: repository branch/versioning policy and promotion flow.
- `genesys_2026_consolidation_plan.md`: prioritized post-2026-1 consolidation plan.
- `genesys_2026_test_matrix.md`: executable validation matrix for modules and applications.
- `genesys_2026_module_inventory.md`: module, target, test, documentation, and risk inventory.
- `genesys_2026_consolidation_handoff.md`: current operational handoff for the consolidation work.
- `genesys_2026_human_decisions.md`: approved decisions, clarified options, and remaining human choices for plugins, Qt, scientific validation, optimization, worker security, and release promotion.
- `oldies_inventory.md`: inventory of temporary historical documents retained in `oldies/`.
- `oldies_review_status.md`: review matrix for historical files before deletion.
- `consolidation_map.md`: target map for consolidating historical documents into stable AI-assistant documentation.
- `oldies/`: temporary historical documents retained for traceability.

## Rules for AI assistants

1. Inspect real repository files before making technical claims.
2. Distinguish confirmed facts, inference, and hypotheses to validate.
3. Prefer small, reversible, reviewable changes.
4. Do not invent paths, targets, classes, methods, plugins, workflows, or build commands.
5. Validate changes with CMake, Ninja, and CTest when the environment allows it.
6. Do not delete historical files from `oldies/` before their relevant content has been consolidated or explicitly discarded.
7. Prefer stable documents in this directory over historical files in `oldies/` when both cover the same topic.
8. Read `genesys_2026_human_decisions.md` before making changes that depend on an approved or pending human architectural decision.

## Historical material

Historical Markdown documents were moved to `oldies/` during the documentation migration. The directory is temporary and should be removed after 2026-11-01, after relevant content has been consolidated into stable documents under `docs/ai_assistants/`.

Use `oldies_inventory.md`, `oldies_review_status.md`, and `consolidation_map.md` before editing or deleting material from `oldies/`.
