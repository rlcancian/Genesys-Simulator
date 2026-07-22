---
document_type: reference-index
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# GenESyS AI Assistant Reference Documentation

## 1. Purpose

This directory contains task-specific technical guidance that is too detailed for canonical governance and architecture.

Read only the references required by the selected task. Current state remains in [`../STATUS.md`](../STATUS.md), and pending work remains in the canonical backlogs.

## 2. Reference set

### [`BUILD_TEST_PACKAGING.md`](BUILD_TEST_PACKAGING.md)

Use for:

- CMake/Ninja/CTest presets;
- CI and sanitizer routing;
- application startup-validation patterns;
- Docker helpers;
- Debian lifecycle;
- Doxygen/generated documentation.

### [`KERNEL_PARSER_OWNERSHIP.md`](KERNEL_PARSER_OWNERSHIP.md)

Use for:

- kernel lifecycle;
- parser contracts;
- pointer/reference ownership;
- persistence impact;
- C++23 modernization;
- regression/sanitizer strategy.

### [`PLUGINS.md`](PLUGINS.md)

Use for:

- plugin registration/factories;
- component/data-definition contracts;
- metadata/persistence/ownership;
- static target discipline;
- optional dependencies;
- future stable C ABI.

### [`SCIENTIFIC_DOMAINS.md`](SCIENTIFIC_DOMAINS.md)

Use for:

- continuous/hybrid/modal/Petri/cellular models;
- numerical/statistical validation;
- biochemical/SBML/whole-cell;
- AI virtual-cell direction;
- scientific claim levels.

### [`APPLICATIONS_TOOLS_MODELS.md`](APPLICATIONS_TOOLS_MODELS.md)

Use for:

- shell/worker/Qt6 applications;
- model-specific generation;
- property editors;
- Data Analyser, Optimizer, AI Assistant and DOE;
- application launch/install boundaries.

### [`API_INTEGRATIONS.md`](API_INTEGRATIONS.md)

Use for:

- facade and shell command coverage;
- Python-facing APIs;
- AI providers;
- generated code/external processes;
- worker/HTTP boundary;
- issue-report relay and public compatibility.

## 3. Reference rules

Reference files:

- must not duplicate current CI status or backlog state;
- must distinguish current code, executed evidence, strong indications and historical findings;
- must link to canonical policy/state and source paths when material;
- must not authorize unresolved architecture/security/scientific/product decisions;
- should remain scoped enough for task-specific AI retrieval;
- must be updated when the underlying public contract changes.

## 4. Historical source preservation

The former topic guides and plans are consolidated during D4. Exact prior text remains in Git history. The D4 migration record maps former paths to these references and records any remaining backlog destination.
