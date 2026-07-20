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
- future AI virtual-cell domain tools and orchestration boundaries

Historical source documents:

- `old_SBML_INTEROPERABILITY_SCOPE.md`
- `old_TINKERCELL_context.md`
- `old_whole_cell_biosimulator_project.md`
- `old_WCM_IMPLEMENTATION_PLAN.md`

## Biochemical model policy

Native GenESyS biochemical definitions should remain the canonical runtime model.

SBML import/export should be treated as an interoperability bridge, not as a replacement for native classes.

Guidance:

- preserve compatibility with native biochemical definitions;
- prefer round-trip stability over broad SBML feature coverage in early phases;
- report unsupported SBML constructs explicitly;
- avoid silent data loss during import/export.

## SBML boundary policy

SBML handling should stay separated from generic kernel behavior.

Guidance:

- keep parser/import/export details in bridge services or dedicated plugins;
- keep GUI workflows as user-facing orchestration layers;
- keep kernel and biochemical data plugins independent from SBML parser specifics;
- provide diagnostics for warnings, errors, and processed object counts.

## Whole-cell modeling policy

Whole-cell modeling is a domain-specific layer above biochemical and simulation primitives.

Guidance:

- preserve the distinction between biochemical infrastructure and whole-cell modeling orchestration;
- validate mathematical and biological semantics before behavior changes;
- treat GLPK/FBA-related behavior and fallback logic as correctness-sensitive;
- keep whole-cell state serialization explicit and testable.

## AI virtual-cell research direction

Decision date: 2026-07-20.

GenESyS will prepare a new research direction inspired by:

- Qian, Dong, and Guo, **Grow AI virtual cells: three data pillars and closed-loop learning**, Cell Research, 2025;
- Qian et al., **Towards the construction of a virtual yeast**, Nature, 2026;
- the WAY — Westlake AI Virtual Cell–Yeast direction;
- Bunne et al. on AI virtual-cell priorities;
- classical mechanistic whole-cell modeling, including Karr et al.

The intended architecture is neuro-symbolic-mechanistic.

Policies:

- LLMs and foundation models do not replace mechanistic simulation;
- ODE, SSA, Petri-net, metabolic/FBA, regulatory, spatial, and hybrid models remain explicit verifiable tools;
- curated data and mechanistic knowledge retain provenance;
- learned representations and transition operators may complement formal models when validated;
- an agent/orchestration layer may route tasks, generate hypotheses, select experiments, call tools, and check consistency;
- closed-loop active learning may connect prediction, experiment selection, new observations, model revision, and revalidation;
- GenESyS should act as simulator, validator, synthetic-trajectory generator, conservation/invariant checker, mechanistic prior, and domain-tool host.

The three data pillars are:

1. prior/mechanistic knowledge;
2. static/subcellular architecture;
3. dynamic cellular states.

See `genesys_ai_virtual_cell_research_direction.md` for the detailed architecture and work packages.

Status:

- research direction: `decided`;
- detailed scientific program, datasets, modules, and first bounded use case: `deferred`;
- current GenESyS implementation must not be described as an implemented AI virtual cell.

## Scientific validation criteria

Scientific validity is not established by successful compilation, a passing smoke test, or plausible-looking output. Evidence must match the level of claim made by the feature/model.

Validation must be considered across these dimensions:

### 1. Software correctness

- lifecycle and ownership;
- persistence and load/save symmetry;
- deterministic/error behavior;
- invalid-input handling;
- absence of known UB/leaks in validated paths;
- reproducible tests.

### 2. Mathematical correctness

- declared equations and algorithms;
- reaction stoichiometry and propensity definitions;
- constraints, conservation laws, units, dimensions, initial conditions, and boundary conditions;
- explicit parameterization when multiple conventions exist.

### 3. Numerical correctness

- solver order and convergence;
- tolerance and step-size policy;
- stability and stiffness behavior;
- non-negativity and conservation policy;
- stochastic sampling and statistical consistency;
- failure and non-finite-value behavior.

### 4. Biochemical/biological semantic correctness

- explicit meaning of species, compartments, reactions, gene-expression events, cell-cycle rules, resource budgets, and states;
- valid domains and biological assumptions;
- distinction between mechanistic representation and empirical approximation.

### 5. Interoperability correctness

- declared SBML level/version/packages and supported construct matrix;
- explicit diagnostics for unsupported constructs;
- preservation of identifiers, units, compartments, reactions, parameters, events, rules, and annotations within the supported subset;
- no silent semantic loss during import/export;
- round-trip tests for supported constructs.

### 6. Empirical or benchmark validation

- analytical cases where available;
- published/curated biochemical or whole-cell models;
- comparator simulators with documented versions/settings;
- laboratory/experimental datasets when the claim is quantitative;
- uncertainty, sensitivity, and parameter-identifiability evidence when applicable.

### 7. Reproducibility and provenance

- model and code version;
- source of parameters and units;
- random seeds;
- solver configuration;
- optional dependency versions;
- calibration/validation datasets;
- result and transformation provenance.

## Software maturity versus scientific claim

The project-wide software target is at least **Level 3 — Beta** for supported functionality before later priorities advance to Level 4.

Scientific claim level is separate:

1. **Educational/demonstrative**
   - illustrates a mechanism or architecture;
   - does not claim quantitative reproduction of a biological system.

2. **Mechanistic research prototype**
   - equations/interactions are meaningful and reference-backed;
   - selected benchmark behavior is validated;
   - quantitative generalization remains limited.

3. **Quantitatively validated model**
   - calibrated and validated against declared data/protocols;
   - uncertainty, sensitivity, parameter identifiability, and prediction intervals are addressed when relevant.

4. **Predictive biological model**
   - prospective prediction claims require strong independent validation;
   - this is a substantially higher bar than software/mathematical correctness.

A Level 3 software feature may still have only a mechanistic-research-prototype scientific claim. Never infer biological predictive validity from software maturity.

## Current claim policy

Until an explicit domain-specific validation package is approved, biochemical and whole-cell functionality must be described as experimental/research-oriented. GenESyS must not make a general predictive-validity claim for these features.

The research program will attempt to improve this level incrementally through bounded models, declared datasets, mechanistic validation, and closed-loop research prototypes.

Status:

- validation dimensions and no-overclaim policy: `decided`;
- AI virtual-cell research direction: `decided`;
- exact initial organism/use case, benchmark suite, supported scientific scope, datasets, and authoritative references: `deferred`.

See:

- `genesys_ai_virtual_cell_research_direction.md`;
- `genesys_2026_decisions_addendum_20260720.md`;
- `genesys_2026_human_decisions.md` for earlier terminology.

## GUI and agent integration policy

GUI features for biochemical, SBML, or whole-cell workflows should remain dependency-gated on the required model plugins.

Avoid hard-coding biosimulation-specific behavior into generic GUI extension infrastructure.

Future AI/agent orchestration must:

- call typed/versioned scientific tools;
- preserve model/data/result provenance;
- keep human approval points for consequential scientific actions;
- prevent unverified LLM output from becoming scientific state;
- apply unit, conservation, domain, and consistency checks;
- distinguish generated hypotheses from validated conclusions.

## Validation checklist

For whole-cell, SBML, or AI virtual-cell changes, prefer this order:

1. Run unit-test validation.
2. Validate affected plugin load/save behavior.
3. Validate mathematical invariants, units, conservation, and invalid-domain behavior.
4. Validate numerical convergence/stability/tolerance behavior.
5. Validate import/export round trip when SBML is involved.
6. Validate diagnostics for unsupported constructs and semantic loss.
7. Validate GUI dependency gating when user-facing workflows are affected.
8. Validate optional numerical dependencies separately, including GLPK and fallback behavior.
9. Compare with declared analytical, published, dataset, or simulator references.
10. Record reproducibility/provenance information for benchmark results.
11. For learned models, record training/validation data, uncertainty, out-of-domain behavior, and data leakage controls.
12. For agent workflows, validate tool routing, evidence capture, consistency checks, and human approval boundaries.

## Open follow-up tasks

- Inventory current biochemical and whole-cell plugin classes.
- Revalidate SBML import/export implementation status against current code.
- Define minimal SBML fixtures for import/export regression tests.
- Decide which SBML constructs are explicitly supported, ignored, or rejected.
- Execute the literature/project landscape phase in `genesys_ai_virtual_cell_research_direction.md`.
- Select one bounded organism/process/use case for the first research prototype.
- Define canonical units, conservation invariants, solver/tolerance policy, and stochastic-validation methodology.
- Select reference models, datasets, publications, and comparator simulators.
- Define acceptance criteria for GLPK-backed and fallback solver behavior.
- Define typed domain-tool and provenance contracts before adding agent orchestration.
- Consolidate TinkerCell context into current GUI/SBML integration policy.
