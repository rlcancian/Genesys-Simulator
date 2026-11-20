---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: completed
immutable_after_completion: true
tracks: 511
---

# Technical Reference Consolidation — 2026-07-22

## 1. Scope

Phase D4 consolidates the former top-level topic guides, plans, coverage matrices and `plugins/` guides into six active references:

- `../../reference/BUILD_TEST_PACKAGING.md`;
- `../../reference/KERNEL_PARSER_OWNERSHIP.md`;
- `../../reference/PLUGINS.md`;
- `../../reference/SCIENTIFIC_DOMAINS.md`;
- `../../reference/APPLICATIONS_TOOLS_MODELS.md`;
- `../../reference/API_INTEGRATIONS.md`.

The root repository `README.md` is updated to link only to the canonical documentation structure.

## 2. Former source categories

### Build/test/package

- `build_ci_tests.md`;
- `docker_packaging.md`;
- build/model-generation portions of `models_and_modelspecific_generation.md`.

### Kernel/parser/ownership

- `kernel_development.md`;
- matrix/indexed-value portions of `matrix_values_and_multidimensional_assignments_plan.md`;
- durable ownership/lifecycle rules from prior audits and consolidated evidence.

### Plugins

- `plugins_development.md`;
- all former guides under `plugins/`;
- durable current static/future C-ABI decisions.

### Scientific domains

- `modal_and_hybrid_simulation.md`;
- `whole_cell_and_sbml.md`;
- scientific-domain plugin guides;
- `genesys_ai_virtual_cell_research_direction.md`;
- durable validation rules from numerical/statistical and optimizer plans.

### Applications/tools/models

- `applications_development.md`;
- `tools_and_statistics.md`;
- `models_and_modelspecific_generation.md`;
- `ExpressionBuilder_property_editor_plan.md`;
- matrix plan application portions;
- completed GUI/DCS integration notes.

### API/integrations

- `terminal_facade_command_coverage.md`;
- `python_integration.md`;
- former external-integration plugin guide;
- worker/AI/relay public-boundary policies.

## 3. Backlog routing

Unimplemented work is not treated as reference truth. It remains routed to:

- `../../BACKLOG_AUTONOMOUS.md` when bounded and executable without new decisions;
- `../../BACKLOG_HUMAN.md` when architecture, security, science, product, maturity or release authority is required.

The numerical/statistical reference plan, multiobjective optimizer plan and AI virtual-cell direction no longer act as parallel active plans in the root.

## 4. Historical preservation

Former exact text remains in Git history. Completed integration narratives and prior plans are represented by this migration record, the monthly evidence ledger, the changelog, and commit/PR/issue history.

D4 removes superseded root/topic files rather than keeping long redirect stubs because the root README and canonical docs are updated in the same branch.

## 5. Non-changes

D4 changes documentation organization only. It does not implement or alter:

- CMake targets or presets;
- plugins or ABI;
- Qt applications;
- parser/kernel behavior;
- numerical/statistical/optimization methods;
- SBML/whole-cell/AI virtual-cell functionality;
- worker security;
- packages or releases.

## 6. Follow-up

D5 adds structural/link/front-matter/backlog validation and enforces the top-level allowlist. D6 consolidates oldies tracking and classification while preserving the post-2026-11-01 deletion gate.
