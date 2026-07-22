---
document_type: reference-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: migration-placeholder
tracks: 511
---

# GenESyS AI Assistant Reference Documentation

## Purpose

This directory will contain topic-specific technical reference material that is too detailed for the canonical architecture and governance documents.

During migration phase D0, existing topic guides remain in their current locations. Do not treat this placeholder as evidence that those guides have already been consolidated.

## Planned reference set

### `BUILD_TEST_PACKAGING.md`

Planned content:

- canonical CMake/Ninja/CTest commands and presets;
- CI workflow responsibilities;
- application build/startup validation patterns;
- model-specific generation runbook;
- Docker and Debian/PPA packaging boundaries;
- sanitizer and diagnostic entry points.

Sources to review include:

- `../build_ci_tests.md`;
- `../docker_packaging.md`;
- stable portions of `../models_and_modelspecific_generation.md`;
- current CMake, presets, workflows, and scripts.

### `PLUGINS.md`

Planned content:

- current static registration/build architecture;
- component/data-definition plugin contracts;
- metadata, factories, persistence, ownership, dependencies, lifecycle;
- optional dependency behavior;
- future stable C ABI boundary;
- electronics, external integration, and general-purpose plugin details.

Sources to review include `../plugins_development.md` and applicable files under `../plugins/`.

### `SCIENTIFIC_DOMAINS.md`

Planned content:

- continuous/hybrid simulation;
- modal models, EFSMs, Petri nets, cellular automata;
- biochemical simulation;
- whole-cell modeling;
- SBML interoperability;
- scientific validation and claim boundaries;
- AI virtual-cell research context.

Sources to review include:

- `../modal_and_hybrid_simulation.md`;
- `../whole_cell_and_sbml.md`;
- relevant files under `../plugins/`;
- approved scientific decision/research documents.

### `APPLICATIONS_TOOLS_MODELS.md`

Planned content:

- shell, worker, main GUI, independent GUIs, and model-specific applications;
- process-launching and context handoff;
- Data Analyser, Optimizer, AI Assistant, and DOE frontend/backend boundaries;
- model generation and compatibility rules.

Sources to review include:

- `../applications_development.md`;
- `../tools_and_statistics.md`;
- `../models_and_modelspecific_generation.md`.

### `API_COVERAGE.md`

Planned content:

- shell-to-`SimulatorFacade` coverage;
- Python-facing coverage;
- generated or manually maintained API exposure matrices;
- explicit exclusions and ownership constraints.

Sources to review include:

- `../terminal_facade_command_coverage.md`;
- `../python_integration.md`;
- historical Python facade coverage after revalidation.

## Reference rules

Reference files:

- must not duplicate current CI run IDs from `STATUS.md`;
- must distinguish current code from historical findings;
- must link to source paths and generated evidence where appropriate;
- must not become task backlogs;
- must not authorize architectural decisions absent from `GOVERNANCE.md` or `ARCHITECTURE.md`;
- should remain short enough for task-specific AI retrieval.
