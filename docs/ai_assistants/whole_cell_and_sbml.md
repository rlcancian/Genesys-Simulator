# Whole-Cell and SBML Guidance

## Purpose

This document is the stable AI-assistant reference for whole-cell modeling, biochemical modeling, SBML interoperability, and related GUI/API boundaries.

Historical notes from `oldies/` must be checked against current source files before being treated as current implementation facts.

## Scope

Primary source areas:

- `source/plugins/data/BiochemicalSimulation/`
- `source/plugins/components/WholeCellModeling/`
- `source/plugins/data/WholeCellModeling/`
- GUI extensions for biochemical and SBML workflows
- future SBML import/export services

Historical source documents:

- `old_SBML_INTEROPERABILITY_SCOPE.md`
- `old_TINKERCELL_context.md`
- `old_whole_cell_biosimulator_project.md`
- `old_WCM_IMPLEMENTATION_PLAN.md`

## Biochemical model policy

Native GenESyS biochemical definitions should remain the canonical runtime model.

SBML import/export should be treated as an interoperability bridge, not as a replacement for native classes.

Guidance:

- Preserve compatibility with native biochemical definitions.
- Prefer round-trip stability over broad SBML feature coverage in early phases.
- Report unsupported SBML constructs explicitly.
- Avoid silent data loss during import/export.

## SBML boundary policy

SBML handling should stay separated from generic kernel behavior.

Guidance:

- Keep parser/import/export details in bridge services or dedicated plugins.
- Keep GUI workflows as user-facing orchestration layers.
- Keep kernel and biochemical data plugins independent from SBML parser specifics.
- Provide diagnostics for warnings, errors, and processed object counts.

## Whole-cell modeling policy

Whole-cell modeling is a domain-specific layer above biochemical and simulation primitives.

Guidance:

- Preserve the distinction between biochemical infrastructure and whole-cell orchestration.
- Validate mathematical and biological semantics before behavior changes.
- Treat GLPK/FBA-related behavior and fallback logic as correctness-sensitive.
- Keep whole-cell state serialization explicit and testable.

## GUI integration policy

GUI features for biochemical, SBML, or whole-cell workflows should remain dependency-gated on the required model plugins.

Avoid hard-coding biosimulation-specific behavior into generic GUI extension infrastructure.

## Validation checklist

For whole-cell or SBML changes, prefer this order:

1. Run unit-test validation.
2. Validate affected plugin load/save behavior.
3. Validate import/export round trip when SBML is involved.
4. Validate diagnostics for unsupported constructs.
5. Validate GUI dependency gating when user-facing workflows are affected.
6. Validate optional numerical dependencies separately.

## Open follow-up tasks

- Inventory current biochemical and whole-cell plugin classes.
- Revalidate SBML import/export implementation status against current code.
- Define minimal SBML fixtures for import/export regression tests.
- Decide which SBML constructs are explicitly supported, ignored, or rejected.
- Consolidate TinkerCell context into current GUI/SBML integration policy.
