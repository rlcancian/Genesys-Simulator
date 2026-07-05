# GenESyS Whole-Cell Biosimulator Project

**Technical report — initial analysis and implementation status**  
Branch: `WiP20261` | Date: 2026-06-03  
Reference: Karr JR et al. (2012). *Cell* 150(2):389–401. doi:10.1016/j.cell.2012.05.044

---

## Objective

Transform the GenESyS Simulator into a biossimulator capable of representing,
simulating, and eventually visualising whole-cell models (WCMs), starting with
*Mycoplasma genitalium* and the canonical Karr et al. (2012) model.

The implementation follows an incremental strategy:
1. Stochastic gene expression (Gillespie SSA + Poisson tau-leaping)
2. Cell division (FtsZ ring / mass threshold)
3. Metabolic submodel (Flux Balance Analysis)
4. Full WCM with all 28 submodels from Karr et al.

---

## Scientific Background

### *Mycoplasma genitalium*

*M. genitalium* G37 has the smallest genome of any free-living organism (~525 genes,
~580 kbp). This minimal complexity made it the target for the first whole-cell
computational model (Karr et al. 2012), which demonstrated genotype-to-phenotype
prediction by integrating 28 coupled submodels covering:

- Metabolism (FBA with 473 reactions)
- Transcription and RNA processing
- Translation and ribosome assembly
- Protein folding, modification, degradation
- DNA replication and chromosome segregation
- Cell growth and division (FtsZ ring)
- Regulatory interactions

**Key biological parameters (from CovertLab/WholeCell `data/parameters.json`, MIT):**

| Parameter | Value | Unit | Source |
|-----------|-------|------|--------|
| Cell initial dry weight | 3.93×10⁻¹⁵ | g (3.93 fg) | Karr 2012 |
| Cell cycle length | 32400 | s (~9 h) | Karr 2012 |
| Replication duration | 15571 | s | Karr 2012 |
| RNAP elongation rate | 50.0 | nt/s | Karr 2012 |
| Ribosome elongation rate | 16.0 | AA/s | Karr 2012 |
| Mean NTP concentration | 5.0 | mM | Karr 2012 |
| Geometry density | 1100.0 | kg/m³ | Karr 2012 |

---

## Local Repository Inventory

### `Whole_Cell_Models/` — 15 imported repositories

| Repository | Path | License | Relevance |
|-----------|------|---------|-----------|
| CovertLab/WholeCell | `models/bacteria/mycoplasma_genitalium/CovertLab_WholeCell` | MIT | Canonical WCM (MATLAB); `data/parameters.json`, `data/fixedConstants.json` |
| CovertLab/wcEcoli | `models/bacteria/escherichia_coli/CovertLab_wcEcoli` | stanford_academic_nc | E. coli WCM |
| CovertLab/vEcoli | `models/bacteria/escherichia_coli/CovertLab_vEcoli` | MIT | Vivarium-based E. coli WCM |
| KarrLab/wc_sim | `tools/simulation_engines/KarrLab_wc_sim` | MIT | WCM simulation engine |
| KarrLab/datanator | `tools/databases_and_kbs/KarrLab_datanator` | MIT | Biochemical parameter database |
| BioSimulations | `tools/standards_and_execution/biosimulations_platform` | MIT | COMBINE/SED-ML execution |

**CovertLab/WholeCell `data/` — directly usable files:**
- `parameters.json` — RNAP rate, ribosome rate, cell mass, FtsZ kinetics (MIT)
- `fixedConstants.json` — structural metadata (arrays, compartments)
- `fittedConstants.json` — fitted simulation constants

---

## GenESyS Architecture — Current WCM Status

### Data plugins (`source/plugins/data/WholeCellModeling/`)

| Class | Role | Status |
|-------|------|--------|
| `MolecularSpecies` | Integer copy-count species for Gillespie SSA | ✅ Implemented |
| `StochasticReactionRule` | Reaction rule with stoichiometry and rate constant; computes propensity | ✅ Implemented |
| `WholeCellState` | Shared state container: molecule counts, metabolite pool, resource budget, cell geometry | ✅ Implemented |

### Component plugins (`source/plugins/components/WholeCellModeling/`)

| Class | Role | Status |
|-------|------|--------|
| `StochasticReactionComponent` | Gillespie Direct Method SSA; processes all `StochasticReactionRule` instances | ✅ Implemented |
| `StochasticTranscription` | Poisson tau-leaping mRNA synthesis (RNAP-limited) | ✅ Implemented |
| `StochasticTranslation` | Poisson tau-leaping protein synthesis (ribosome-limited) | ✅ Implemented |
| `CellDivisionEvent` | Cell division triggered by FtsZ ring or mass threshold; binomial partitioning | ✅ Implemented |

### Existing BiochemicalSimulation plugins (ODE-based)

`source/plugins/data/BiochemicalSimulation/`: BioSpecies, BioReaction, BioNetwork,
BioParameter, MetabolicNetwork, MetabolicReaction, GeneticCircuit, GeneticCircuitPart,
GeneticRegulation.

`source/plugins/components/BiochemicalSimulation/`: BioSimulate, BioSteadyState,
MetabolicFluxBalance, GeneticCircuitSimulate, GeneticExpressionStep, BacteriaColony.

### Biochemical tools (`source/tools/Biochemical/`)

| Tool | Description |
|------|-------------|
| `WholeCellParameterReader` | Parses `parameters.json` / `fixedConstants.json` scalar values into a flat map |
| `BioKineticLawExpression` | Expression evaluator for kinetic laws (no external parser dependency) |
| `GlpkFluxBalanceSolver` | FBA LP solver (GLPK optional; built-in fallback for small models) |
| `MassActionOdeSystem` | Mass-action ODE system for BioNetwork |
| `MetabolicFluxBalanceSolver` | FBA solver wrapper |

### Continuous ODE tools (`source/tools/Continuous/`)

`RungeKutta4OdeSolver`, `SolverDefaultImpl1`, `OdeSolver_if`, `OdeSystem_if`.

---

## Terminal Examples — WCM

Location: `source/applications/terminal/examples/wcm/`

### WcmSimpleSsa — Canonical stochastic gene expression

4 reactions (transcription, mRNA degradation, translation, protein degradation) for a
single gene. Analytic steady state: mRNA≈2, protein≈200. Gillespie SSA.
30 replications × 3600 s.

**Build & run:**
```bash
cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmSimpleSsa.cpp
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application
```

### WcmGeneExpression — Central dogma with 4 genes

Transcription (Poisson tau-leaping) + Translation (Poisson tau-leaping) +
mRNA/protein degradation (Gillespie SSA) for 4 genes.
RNAP_free=200, ribosome_free=400. 10 replications × 3600 s.

**Status:** ✅ Runs successfully. Produces ~160 mRNA/step, ~1300 proteins/step.

### WcmMgenitaliumKarr2012 — M. genitalium analog

All four WCM components in pipeline:
`SimClock → Transcription → Translation → MetabolicSSA → CellDivisionEvent`

10 representative M. genitalium genes (MG_001–MG_462), FtsZ ring polymerization,
ATP/ADP energy metabolism, mRNA/protein degradation. 3 replications × 36000 s.

**Status:** ✅ Runs successfully. Cell division detected at t≈840 s per replication.

**Observed output:**
```
StochasticTranscription "Transcription": synthesized 642 mRNA molecules across 10 species
StochasticTranslation "Translation": synthesized 1229 proteins across 10 genes
CellDivisionEvent "CellDivision": division event #1 — cell mass=0.000000 volume=0.000000
```

---

## Known Issues and Limitations

### Bug 1: `_initBetweenReplications` does not reset `_cellMass` / `_cellVolume`

`WholeCellState::_initBetweenReplications()` resets molecule counts and metabolite pool
but leaves `_cellMass` and `_cellVolume` at the post-division value from the previous
replication. For multi-replication studies with mass-based division triggers, this
causes drift.

**Fix:** Add `_cellMass = DEFAULT.cellMass; _cellVolume = DEFAULT.cellVolume;` to
`_initBetweenReplications()`.

### Bug 2: `std::to_string` shows `0.000000` for femtogram values

Trace output uses `std::to_string(mass)` which formats to 6 decimal places. Femtogram
values (1e-15 g) display as `0.000000`. The simulation is numerically correct.

**Fix:** Use `std::scientific` or a helper formatter in the trace calls.

### Limitation 1: FtsZ division time 840 s vs real 32400 s

The WcmMgenitaliumKarr2012 example uses simplified FtsZ kinetics (k_poly=0.002/s,
k_depoly=0.0005/s) which produce ring completion in ~14 min. The real cell cycle
is ~9 hours. The real kinetics involve nucleation (k=4.2×10⁶), elongation
(k=5.1×10⁶), and exchange terms from `parameters.json`.

### Limitation 2: No cell growth between divisions

Cell mass and volume are fixed during the simulation. The real model increases cell
mass at each step via metabolic output.

### Limitation 3: Transcription over-produces mRNA

With `bindingProbability=1.0` and 200 RNAP, the 10-gene model synthesizes ~640
mRNA/step, which is approximately 64 per gene vs the expected ~2-10 for a short-lived
mRNA at steady state. Tuning `bindingProbability` (~0.01) would bring output closer to
biological values.

### Limitation 4: Metabolic FBA not connected to WholeCellState

`MetabolicFluxBalance` (existing plugin) reads `BioNetwork` and `BioSpecies` but is
not yet wired to read/write `WholeCellState`. Integration would enable energy
accounting (ATP consumption per transcription/translation event).

---

## Concept → Plugin Mapping

| WCM Concept (Karr 2012) | GenESyS ModelDataDefinition | GenESyS ModelComponent |
|------------------------|-----------------------------|-----------------------|
| Cell state / Store | `WholeCellState` | — |
| mRNA species | `WholeCellState` molecule count | — |
| Protein species | `WholeCellState` molecule count | — |
| Metabolite | `WholeCellState` metabolite pool | — |
| Discrete molecule (SSA) | `MolecularSpecies` | — |
| Stochastic reaction | `StochasticReactionRule` | `StochasticReactionComponent` |
| Transcription process | — | `StochasticTranscription` |
| Translation process | — | `StochasticTranslation` |
| Cell division | — | `CellDivisionEvent` |
| Metabolic network | `MetabolicNetwork`, `BioNetwork` | `MetabolicFluxBalance` |
| Gene regulatory network | `GeneticCircuit` | `GeneticCircuitSimulate` |
| ODE species (continuous) | `BioSpecies` | `BioSimulate`, `DiffEquations` |

---

## Implementation Plan

### Phase 0 — Already complete ✅

All WholeCellModeling plugins implemented and tested. Three terminal examples running.
Parameters from CovertLab/WholeCell `parameters.json` match all hardcoded defaults.

### Phase 1 — Bug fixes (immediate)

1. Fix `WholeCellState::_initBetweenReplications` to reset `_cellMass` and `_cellVolume`.
2. Improve trace formatting for femtogram values.
3. Tune `bindingProbability` in WcmMgenitaliumKarr2012 (0.01) to produce realistic mRNA counts.

### Phase 2 — Cell growth component

Create `CellGrowthComponent` (`ModelComponent`) that:
- Reads current `_cellMass` from `WholeCellState`
- Applies a linear growth rate each time step
- Sets `_cellVolume = _cellMass / density`
- Can load growth rate from `parameters.json` (`states.MetabolicReaction.meanInitialGrowthRate = 2.14e-05`)

### Phase 3 — FtsZ dynamics improvement

Implement `FtsZPolymerizationComponent` using the real kinetic parameters from
`parameters.json` (activationFwd=1.1, elongationFwd=5.1×10⁶, nucleationFwd=4.2×10⁶).
This will produce realistic division times (~32400 s).

### Phase 4 — Metabolic submodel integration

Connect `MetabolicFluxBalance` to `WholeCellState`:
- Load metabolic network from SBML or internal data
- Produce `WholeCellState::setMetaboliteAmount` updates each step
- Provide ATP budget for resource allocation in StochasticReactionComponent

### Phase 5 — Resource allocation

Implement fair allocation of molecular machines before each SSA step, matching
the Karr 2012 "ResourceAllocation" submodel.

### Phase 6 — Experiment reproduction

Reproduce qualitative results from Karr 2012 Figure 2:
- mRNA copy number distribution per gene
- Protein copy number distribution per gene
- Cell cycle length distribution (target: ~32400 s ± variance)
- Division mass distribution

### Phase 7 — GUI visualization

Integrate results datasets into GenESyS GUI Tools Extensions to plot:
- Molecule copy number over time per species
- Cell mass/volume over time
- FtsZ ring completion over time
- Division event timeline

---

## License and Provenance

All hardcoded biological parameters in the WholeCellModeling plugins are derived from
published values in Karr et al. (2012) and from `CovertLab/WholeCell/data/parameters.json`
(MIT License). No code was copied from the MATLAB repository.

| Source | License | Usage |
|--------|---------|-------|
| Karr et al. (2012), *Cell* | CC / cited work | Scientific parameters |
| CovertLab/WholeCell (`parameters.json`) | MIT | Numerical parameter values |
| CovertLab/wcEcoli | stanford_academic_nc | Reference only, no code copied |

---

## How to Build and Run WCM Examples

```bash
# From WiP20261/Genesys-Simulator/

# WcmSimpleSsa — single-gene SSA validation
cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmSimpleSsa.cpp
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application

# WcmGeneExpression — central dogma, 4 genes
cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmGeneExpression.cpp
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application

# WcmMgenitaliumKarr2012 — M. genitalium analog, 10 genes, cell division
cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=wcm/WcmMgenitaliumKarr2012.cpp
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application
```

---

## References

- Karr JR et al. (2012). A whole-cell computational model predicts phenotype from genotype. *Cell*, 150(2), 389–401.
- Gillespie DT (1977). Exact stochastic simulation of coupled chemical reactions. *J Phys Chem*, 81(25), 2340–2361.
- Raj A & van Oudenaarden A (2008). Nature, nurture, or chance: stochastic gene expression and its consequences. *Cell*, 135(2), 216–226.
- Waltemath D et al. (2016). Toward community standards and software for whole-cell modeling. *IEEE TBME*, 63(10), 2007–2014.
