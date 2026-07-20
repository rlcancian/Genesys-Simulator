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
- `applications_development.md`: terminal, worker, GUI, and application-structure guidance.
- `tools_and_statistics.md`: statistical tools, data analyzer, optimizer, and DOE/RSM guidance.
- `modal_and_hybrid_simulation.md`: modal models and hybrid discrete-continuous simulation guidance.
- `whole_cell_and_sbml.md`: whole-cell, biochemical, and SBML guidance.
- `python_integration.md`: Python-facing integration guidance.
- `documentation_governance.md`: documentation retention and governance guidance.
- `branch_workflow.md`: repository branch/versioning policy and promotion flow, including the `20261`/`20262` naming convention.
- `genesys_2026_consolidation_plan.md`: prioritized post-2026-1 consolidation plan.
- `genesys_2026_test_matrix.md`: executable validation matrix for modules and applications.
- `genesys_2026_module_inventory.md`: module, target, test, documentation, and risk inventory.
- `genesys_2026_consolidation_handoff.md`: current operational handoff for the consolidation work.
- `genesys_2026_phase0_ci_evidence_20260720.md`: executed Phase 0 CI checkpoint for the current consolidation PR head; supersedes older current-status claims about the three GUI GMDD tests.
- `genesys_2026_human_decisions.md`: initial approved decisions, clarified options, and remaining human choices.
- `genesys_2026_decisions_addendum_20260720.md`: later decisions that supersede conflicting statements, including `20261`/`20262`, stable C ABI, Level 3 maturity policy, intranet worker profile, and AI virtual-cell direction.
- `genesys_numerical_statistical_references_plan.md`: deferred plan for acquiring bibliography, PDFs, datasets, parameterizations, and expected results.
- `genesys_multiobjective_optimizer_future_plan.md`: future Level 3 multiobjective Optimizer research/implementation plan, including ETH Zürich/PISA, hypervolume, and the professor's doctoral research.
- `genesys_ai_virtual_cell_research_direction.md`: neuro-symbolic-mechanistic AI virtual-cell research direction for whole-cell/biochemical evolution.
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
8. Read `genesys_2026_human_decisions.md` and `genesys_2026_decisions_addendum_20260720.md` before making changes that depend on a human architectural, scientific, maturity, security, or branch-governance decision.
9. Read the latest dated execution-evidence document before treating a historical test failure or CI result as current.
10. When two stable decision documents conflict, the later explicitly superseding addendum takes precedence.
11. Do not claim Level 3 or Level 4 maturity without satisfying the documented acceptance criteria.

## Historical material

Historical Markdown documents were moved to `oldies/` during the documentation migration. The directory is temporary and should be removed after 2026-11-01, after relevant content has been consolidated into stable documents under `docs/ai_assistants/`.

Use `oldies_inventory.md`, `oldies_review_status.md`, and `consolidation_map.md` before editing or deleting material from `oldies/`.
