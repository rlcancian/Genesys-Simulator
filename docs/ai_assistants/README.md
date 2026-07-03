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
- `oldies_inventory.md`: inventory of temporary historical documents retained in `oldies/`.
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

## Historical material

Historical Markdown documents were moved to `oldies/` during the documentation migration. The directory is temporary and should be removed after 2026-11-01, after relevant content has been consolidated into stable documents under `docs/ai_assistants/`.

Use `oldies_inventory.md` and `consolidation_map.md` before editing or deleting material from `oldies/`.
