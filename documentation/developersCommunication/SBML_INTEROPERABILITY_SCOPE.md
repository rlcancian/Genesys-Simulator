# SBML Interoperability Scope (TinkerCell-Aligned)

## Objective

Define a practical and incremental SBML interoperability scope for the native biochemical line in GenESyS, preserving:

- plugin-oriented model extensibility;
- GUI extension independence from kernel implementation details;
- backward compatibility with current `BioSpecies`, `BioParameter`, `BioReaction`, and `BioNetwork`.

## Guiding Principles

1. Native GenESyS biochemical definitions remain the canonical runtime model.
2. SBML import/export is a bridge layer, not a replacement for native classes.
3. Round-trip stability is preferred over broad SBML feature coverage in the first delivery.
4. Unsupported SBML constructs must produce explicit diagnostics, never silent data loss.

## Phase 1 (Initial Delivery) - In Scope

### Import to Native Biochemical Model

- Read SBML core model metadata into `BioNetwork` basic controls (`startTime`, `stopTime`, `stepSize`) when available.
- Map SBML species to `BioSpecies`:
  - `id/name` -> definition name (deterministic naming policy);
  - `initialAmount` or `initialConcentration` -> `initialAmount` and `amount`;
  - `constant` -> `constant`;
  - `boundaryCondition` -> `boundaryCondition`;
  - unit string (when present) -> `unit`.
- Map SBML global parameters to `BioParameter`:
  - `id/name` -> definition name;
  - numeric value -> `value`;
  - unit string -> `unit`.
- Map SBML reactions to `BioReaction`:
  - reactants/products with stoichiometry;
  - reversible flag;
  - kinetic law:
    - if directly reducible to mass-action constant form, use `rateConstant` / `reverseRateConstant`;
    - otherwise store native expression text in `kineticLawExpression` / `reverseKineticLawExpression`.

### Export from Native Biochemical Model

- Export selected `BioNetwork` membership (species/reactions) to one SBML model.
- Export all referenced `BioSpecies`, `BioParameter`, and `BioReaction` fields listed above.
- Export reaction kinetic semantics from:
  - direct constants (`rateConstant`, `reverseRateConstant`) or
  - stored kinetic-law expressions.

## Phase 1 - Explicitly Out of Scope

- SBML Level 3 package support beyond baseline core (Layout, Render, FBC, Groups, Comp, etc.).
- Events, rules, algebraic constraints, and piecewise assignment semantics.
- Compartments with full spatial semantics and unit-conversion automation.
- Automatic graphical layout conversion between SBML layout/render and GenESyS scene.
- Stochastic/discrete biochemical simulation semantics from SBML annotations.

## Plugin and GUI Boundaries

- Kernel and biochemical data plugins stay independent from SBML parser specifics.
- Interoperability is implemented by dedicated bridge services/plugins (import/export endpoints).
- GUI adds user-facing import/export workflows via graphical extension plugins:
  - `Import SBML...` (preview + diagnostics + naming conflict policy);
  - `Export SBML...` (target network selection + report).

## Expression and Kinetics Compatibility Policy

- Imported kinetic laws are preserved as textual expressions whenever exact mass-action mapping is not guaranteed.
- Any symbol unresolved against imported `BioSpecies`/`BioParameter` sets must be reported as import error.
- Function/operator support follows current `BioKineticLawExpression` capabilities for runtime execution.

## Diagnostics and Traceability

- Every import/export operation must return:
  - success/failure status;
  - warnings list (non-fatal losses or simplifications);
  - errors list (fatal blockers);
  - counts: species, parameters, reactions processed.
- Diagnostics should be available both in GUI and programmatic API payloads.

## Acceptance Criteria for Phase 1

1. Import of representative mass-action SBML models produces executable native `BioNetwork` simulation.
2. Export from native biochemical definitions generates SBML accepted by libSBML validation.
3. Import -> export -> import round-trip preserves:
   - species/parameter/reaction names;
   - stoichiometry and reversible flags;
   - kinetic-law text or equivalent constants.
4. Unsupported constructs are reported with explicit, user-visible diagnostics.

## Suggested Next Technical Tasks

1. Implement internal mapping contracts (`SBML <-> Bio*`) with unit tests.
2. Add import/export service API in biochemical plugin layer.
3. Add GUI extension commands for SBML import/export with preview and diagnostics.
4. Add regression suite with curated SBML fixtures inspired by TinkerCell module patterns.
