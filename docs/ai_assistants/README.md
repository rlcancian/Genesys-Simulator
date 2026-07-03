# GenESyS AI Assistant Documentation

This directory is the entrypoint for AI-assisted technical documentation for the GenESyS Simulator.

Read this file before using or updating the other files in this directory.

## Current layout

- `README.md`: entrypoint for AI assistants.
- `current_plans.md`: active plans and migration notes.
- `oldies/`: temporary historical documents retained for traceability.

## Rules for AI assistants

1. Inspect real repository files before making technical claims.
2. Distinguish confirmed facts, inference, and hypotheses to validate.
3. Prefer small, reversible, reviewable changes.
4. Do not invent paths, targets, classes, methods, plugins, workflows, or build commands.
5. Validate changes with CMake, Ninja, and CTest when the environment allows it.

## Historical material

Historical Markdown documents were moved to `oldies/` during the documentation migration. The directory is temporary and should be removed after 2026-11-01, after relevant content has been consolidated into stable documents under `docs/ai_assistants/`.
