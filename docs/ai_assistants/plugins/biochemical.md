# Biochemical Plugin Guidance

## Scope

This guide covers biochemical simulation plugins and biosimulation data definitions.

Observed source area:

- `source/plugins/data/BiochemicalSimulation/`

Known sampled files:

- `BioNetwork.h`
- `BioNetwork.cpp`
- `BioSimulatorRunner.cpp`

Related historical material:

- `old_SBML_INTEROPERABILITY_SCOPE.md`
- `old_TINKERCELL_context.md`
- `old_whole_cell_biosimulator_project.md`
- `old_WCM_IMPLEMENTATION_PLAN.md`

## Guidance

- Keep biochemical simulation data definitions separate from generic simulation data definitions.
- Validate SBML import/export assumptions before changing names, metadata, or persistence.
- Treat simulator-runner behavior as integration-sensitive.
- Keep GUI biosimulation extensions dependency-gated on the required model plugins.
- Do not fold whole-cell-specific behavior into biochemical base infrastructure unless the abstraction is explicit.

## Open follow-up

- Inventory all `BiochemicalSimulation` classes.
- Revalidate GUI biosimulation configuration against current GUI extension code.
- Decide the boundary between biochemical plugins and whole-cell modeling plugins.
