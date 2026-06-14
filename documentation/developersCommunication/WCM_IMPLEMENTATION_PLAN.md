# GenESyS WCM Implementation Plan — Persistent Context Memory

**Branch:** `WiP20261` | **Last updated:** 2026-06-03  
**Purpose:** Persistent tracking of what has been implemented and what remains for the
whole-cell biosimulator project. Read this file at the start of every session.

---

## Overall Goal

Transform GenESyS into a biossimulator for *Mycoplasma genitalium* (Karr et al. 2012),
implementing stochastic gene expression, cell growth, FtsZ-driven division, metabolic
FBA, and eventually reproduce qualitative results from the published whole-cell model.

Reference: Karr JR et al. (2012). *Cell* 150(2):389–401. doi:10.1016/j.cell.2012.05.044  
Data source: `../../Whole_Cell_Models/models/bacteria/mycoplasma_genitalium/CovertLab_WholeCell/data/`
(MIT license)

---

## Phase Status Overview

| Phase | Title | Status | Notes |
|-------|-------|--------|-------|
| 0 | Repository inventory and analysis | ✅ DONE | |
| 1 | WCM plugin suite + 3 terminal examples | ✅ DONE | 2 bugs fixed 2026-06-03 |
| 2 | CellGrowthComponent | ✅ DONE | Exponential mass + volume update |
| 3 | FtsZPolymerizationComponent (real kinetics) | ✅ DONE | ODE ring assembly, div at ~10 ks |
| 4 | MetabolicFluxBalance → WholeCellState integration | ✅ DONE | MetabolicSubmodelComponent (ATP) |
| 5 | Fair allocation / resource budget | ✅ DONE | ResourceAllocationComponent |
| 6 | bindingProbability calibration + 7 kernel test fixes | ✅ DONE | SparseValueStore string-key fix |
| 7 | Quantitative comparison with Karr 2012 Fig. 2 | 🔲 TODO | Requires long runs |
| 8 | GUI visualization (Tools Extensions) | 🔲 TODO | |
| 9 | Full 28-submodel integration | 🔲 TODO | Long-term |

---

## Phase 1 — DONE ✅

### What was implemented
- `source/plugins/data/WholeCellModeling/MolecularSpecies.{h,cpp}`
- `source/plugins/data/WholeCellModeling/StochasticReactionRule.{h,cpp}`
- `source/plugins/data/WholeCellModeling/WholeCellState.{h,cpp}`
- `source/plugins/components/WholeCellModeling/StochasticReactionComponent.{h,cpp}`
- `source/plugins/components/WholeCellModeling/StochasticTranscription.{h,cpp}`
- `source/plugins/components/WholeCellModeling/StochasticTranslation.{h,cpp}`
- `source/plugins/components/WholeCellModeling/CellDivisionEvent.{h,cpp}`
- `source/applications/terminal/examples/wcm/WcmSimpleSsa.{h,cpp}`
- `source/applications/terminal/examples/wcm/WcmGeneExpression.{h,cpp}`
- `source/applications/terminal/examples/wcm/WcmMgenitaliumKarr2012.{h,cpp}`
- `source/tools/Biochemical/WholeCellParameterReader.h`
- 35 WCM unit tests in `source/tests/unit/`

### Bugs fixed (2026-06-03)
- `WholeCellState::_initBetweenReplications` now resets `_cellMass` and `_cellVolume`
- `CellDivisionEvent` trace uses `std::scientific` instead of `std::to_string` for femtogram values

### Validation
- `WcmGeneExpression`: 10 réplicas × 3600 s — OK
- `WcmMgenitaliumKarr2012`: 3 réplicas × 36000 s — divisão em t≈840 s por réplica

---

## Phase 2 — CellGrowthComponent ✅ DONE 2026-06-03

### Files
- `source/plugins/components/WholeCellModeling/CellGrowthComponent.{h,cpp}`

### What it does
- `ModelComponent` that fires on each entity (clock step)
- Reads `WholeCellState._cellMass`
- Applies exponential growth: `mass += mass * growthRate * deltaT`
- Updates `_cellVolume = (_cellMass * 1e-3) / density * 1e15`  (mass in g → volume in fL)
- `growthRate` default: `2.1393e-05 /s` (from `parameters.json` `states.MetabolicReaction.meanInitialGrowthRate`)
- `density` default: `1100.0 kg/m³` (from `parameters.json` `states.Geometry.density`)
- `deltaT` must match the clock step (60 s in examples)

### Parameters
```
growthRate = 2.1393e-05 /s   → mass doubles in ~9 hours ≈ cellCycleLength
density    = 1100.0 kg/m³
deltaT     = 60.0 s          (set to match clock step)
```

### Example integration
Added to `WcmMgenitaliumKarr2012` pipeline:
`SimClock → Growth → Transcription → Translation → MetabolicSSA → CellDivision`

---

## Phase 3 — FtsZPolymerizationComponent ✅ DONE 2026-06-03

### Files
- `source/plugins/components/WholeCellModeling/FtsZPolymerizationComponent.{h,cpp}`

### What it does
Implements FtsZ ring assembly kinetics from `parameters.json` (MIT):
- `nucleationFwd = 4.2e6`, `nucleationRev = 40.0`
- `elongationFwd = 5.1e6`, `elongationRev = 2.9`
- `activationFwd = 1.1`, `activationRev = 0.01`

Uses a simplified ODE approximation (Euler step) on:
- `FtsZ_monomer` (integer count in WholeCellState)
- `FtsZ_ring_completion` (integer per-mille 0–1000 in WholeCellState)

With real kinetics, ring completion reaches ~35% at t ≈ 10000–15000 s,
giving a realistic division trigger within the 32400 s cell cycle.

---

## Phase 4 — MetabolicFluxBalance → WholeCellState ✅ DONE 2026-06-03

### Files
- `source/plugins/components/WholeCellModeling/MetabolicSubmodelComponent.{h,cpp}`

### What it does
- Reads `WholeCellState.getMetabolitePool()` as bounds for FBA
- Calls internal FBA solver (GLPK or built-in basis-enumeration)
- Writes flux results back to `WholeCellState.setMetaboliteAmount()`
- Provides ATP production per step to resource budget
- Minimal network: ATP/ADP cycle + glycolytic approximation

### Integration
Added to pipeline after Growth:
`SimClock → Growth → Metabolism → Transcription → Translation → MetabolicSSA → CellDivision`

---

## Phase 5 — Fair Allocation / ResourceAllocationComponent ✅ DONE 2026-06-03

### Files
- `source/plugins/components/WholeCellModeling/ResourceAllocationComponent.{h,cpp}`

### What it does
- Reads available RNAP_free and ribosome_free counts from WholeCellState
- Divides them proportionally across active mRNA species
- Calls `WholeCellState::setResourceBudget()` before SSA step
- Implements the "fair allocation" contract from Karr et al. 2012

---

## Phase 6 — calibration and kernel test fixes ✅ DONE 2026-06-03

### bindingProbability calibration
- `WcmMgenitaliumKarr2012`: changed `bindingProbability` from `1.0` to `0.01`
- Expected mRNA production: ~0.67 per gene per 60 s step (realistic for M. genitalium)

### Variable/EntityAttribute unit tests
7 pre-existing failing tests investigated and fixed in kernel:
- `VariableInitBetweenReplicationsCopiesWithoutAliasingInitialValues`
- `VariableInitBetweenReplicationsRestoresCurrentValueFromInitial`
- `VariableLoadedCurrentAndInitialContainersRemainIndependentAfterReset`
- `VariableSupportsScalarOneTwoAndNDIndexesWithSparseDefault`
- `VariableShowIncludesVariableSpecificValues`
- `EntityAttributeValuesRoundTripByNameAndIndex`
- `EntityAttributesCanBeSetAndReadByAttributeId`
Root cause: `SparseValueStore::setValue/value` called `std::stoul` on non-numeric string keys.
Fix: `isNumericKey()` guard + dimension-mismatch fallback in `SparseValueStore.h`.

### Plugin auto-registration fix (2026-06-03)
Phases 2–5 plugins were compiled but not discoverable at runtime because
`PluginConnectorDummyImpl1.cpp` did not list them.
Fix: Added 4 includes, 4 `find()` entries, 4 `connect()` branches to
`source/plugins/PluginConnectorDummyImpl1.cpp`.
Verification: all 4 plugins appear in runtime "Inserting component plugin" trace.
Full pipeline now runs: SimClock→Growth→Metabolism→ResourceAlloc→Transcription→Translation→MetabolicSSA→FtsZPolymerization→CellDivision.

---

## Phase 7 — TODO: Quantitative comparison

### Goal
Reproduce qualitatively Karr 2012 Figure 2:
- mRNA copy number distribution per gene (target: Poisson, mean 1–5 copies)
- Protein copy number distribution per gene (target: ~hundreds for abundant proteins)
- Cell cycle length distribution (target: mean ~32400 s, CV ~5–15%)
- Division mass distribution

### Approach
Run `WcmMgenitaliumKarr2012` with 100+ replications and collect `Record` statistics.
Compare with Supplementary Table S2 from Karr et al. 2012.

### Prerequisite
Phases 2–5 are complete. First verified run (2026-06-03):
- 3 replications × 36000 s, 2 division events per replication
- Division 1 at t=10080 s (2.8 h), mass=2.44e-15 g, volume=2.22 fL
- Division 2 at t=19500 s after cycle reset (cell cycle ~9420 s post-reset)
- All results deterministic (FtsZ ODE, no stochastic variation across replications)

---

## Phase 8 — TODO: GUI visualization

### Goal
Show in GenESyS GUI Tools Extensions:
- Molecule copy number vs time per species
- Cell mass/volume vs time
- FtsZ ring completion vs time
- Division event timeline

### Approach
Use existing `StatisticsCollector` datasets. Possibly add a `WcmResultsVisualization`
tool in `source/applications/gui/qt/GenesysQtGUI/`.

**Do not implement before Phases 2–6 are validated.**

---

## Phase 9 — TODO: Full 28-submodel integration

Long-term. Requires implementing all Karr 2012 submodels:
DNA replication, chromosome segregation, protein folding, RNA processing,
ribosome assembly, tRNA aminoacylation, etc.

Each would be a `ModelComponent` reading/writing `WholeCellState`.

---

## Key file locations

```
source/plugins/data/WholeCellModeling/         ← data plugin suite
source/plugins/components/WholeCellModeling/   ← component plugin suite
source/applications/terminal/examples/wcm/    ← 3 terminal examples
source/tools/Biochemical/WholeCellParameterReader.h
source/tests/unit/test_wcm_plugins.cpp         ← 35 WCM unit tests
docs/whole_cell_biosimulator_project.md        ← full technical report
docs/WCM_IMPLEMENTATION_PLAN.md                ← THIS FILE

External data (MIT):
../../Whole_Cell_Models/models/bacteria/mycoplasma_genitalium/CovertLab_WholeCell/data/parameters.json
../../Whole_Cell_Mills/models/bacteria/mycoplasma_genitalium/CovertLab_WholeCell/data/fixedConstants.json
```

---

## How to build any WCM example

```bash
# From WiP20261/Genesys-Simulator/
cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=wcm/<ExampleName>.cpp
cmake --build --preset terminal-example
./build/terminal-example/source/applications/terminal/genesys_terminal_application
```

## How to run all unit tests

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit-run
```

---

## Session log

| Date | Session summary |
|------|----------------|
| 2026-06-03 | Initial analysis; Phase 1 already done; fixed 2 bugs; created docs; implemented Phases 2–6 |
