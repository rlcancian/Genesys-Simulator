# GenESyS AI Virtual Cell Research Direction

## 1. Purpose

Record the intended scientific and architectural direction for evolving GenESyS whole-cell and biochemical capabilities toward an AI virtual-cell research platform.

Status: `decided` as research direction; detailed scientific program and implementation remain `deferred`.

This document does not claim that current GenESyS whole-cell functionality already implements an AI virtual cell.

## 2. Primary conceptual references

The initial research direction is inspired by:

- Qian, Dong, and Guo, **Grow AI virtual cells: three data pillars and closed-loop learning**, Cell Research, 2025, DOI `10.1038/s41422-025-01101-y`;
- Qian et al., **Towards the construction of a virtual yeast**, Nature, 2026, DOI `10.1038/s41586-026-10574-9`;
- the WAY — Westlake AI Virtual Cell–Yeast project;
- Bunne et al., **How to build the virtual cell with artificial intelligence: priorities and opportunities**, Cell, 2024;
- Karr et al., **A whole-cell computational model predicts phenotype from genotype**, Cell, 2012;
- related autonomous-science and closed-loop biology projects to be surveyed in a dedicated literature review.

These references must be collected, converted where legally permitted, and analysed in a future scientific-planning round.

## 3. Core interpretation

The intended architecture is **neuro-symbolic-mechanistic**.

The project must not treat LLMs or foundation models as replacements for mechanistic cellular models.

Instead:

- mechanistic models remain explicit, inspectable, testable tools;
- ODE, SSA, Petri nets, metabolic/FBA, regulatory, spatial, and other formal models retain their mathematical semantics;
- curated databases and scientific knowledge remain traceable sources;
- learned representations may encode high-dimensional cellular states;
- learned transition operators may predict perturbation-response dynamics where validated;
- agent/LLM layers orchestrate tools, generate hypotheses, select experiments, and check consistency;
- closed-loop learning connects predictions, experiment selection, new observations, model updating, and revalidation.

GenESyS should become a host for formal scientific instruments inside a broader agent system, not a black-box conversational substitute for simulation.

## 4. Three data pillars

The research program should organize its data and model contracts around three pillars.

### 4.1 Prior/mechanistic knowledge

Examples:

- biochemical reactions and stoichiometry;
- gene-regulatory relationships;
- metabolic pathways;
- conservation laws;
- kinetic laws and parameter priors;
- cellular-process literature;
- curated ontologies and databases;
- known causal or mechanistic constraints.

GenESyS role:

- encode formal models and constraints;
- validate invariants;
- execute mechanistic simulations;
- provide explainable causal traces.

### 4.2 Static/subcellular architecture

Examples:

- compartments and organelles;
- spatial topology;
- molecular complexes;
- membrane and transport structure;
- chromosome/genome organization;
- structural proteomics and localization.

GenESyS role:

- represent component networks and compartments;
- host spatial, modal, and hybrid models;
- enforce topology and structural constraints;
- persist versioned architecture.

### 4.3 Dynamic cellular states

Examples:

- transcriptomic, proteomic, metabolomic, and flux states;
- perturbation response;
- cell-cycle and growth state;
- stochastic event histories;
- time-series and single-cell trajectories;
- environmental and experimental context.

GenESyS role:

- generate and replay trajectories;
- combine discrete and continuous dynamics;
- provide synthetic training/validation data;
- compare predicted and observed state transitions;
- record provenance, seeds, solver configuration, and uncertainty.

## 5. Proposed layered architecture

### Layer 1 — scientific data and provenance

Responsibilities:

- dataset ingestion;
- metadata and units;
- experiment/perturbation descriptions;
- licensing and citation;
- quality control;
- versioning and provenance.

### Layer 2 — mechanistic model registry

Responsibilities:

- biochemical networks;
- whole-cell submodels;
- ODE/SSA/Petri/FBA/spatial models;
- SBML/native import/export;
- validity domains and assumptions;
- reference models and benchmark fixtures.

### Layer 3 — executable domain tools

Each tool must expose a typed, versioned contract and structured diagnostics.

Candidate tools/modules include:

- genome and gene regulation;
- transcription and translation;
- metabolism and energy;
- signalling and stress response;
- cell cycle, growth, and division;
- transport and compartments;
- spatial/structural organization;
- phenotype/fitness and experimental readouts.

The exact eight-module decomposition from the virtual-yeast proposal should be studied rather than copied without domain review.

### Layer 4 — learned representations and transition models

Possible future responsibilities:

- multimodal embeddings;
- state compression;
- perturbation-response prediction;
- surrogate models;
- uncertainty estimation;
- active-learning acquisition functions.

Learned components must declare training data, generalization domain, uncertainty, and failure modes.

### Layer 5 — orchestration and agent layer

Responsibilities:

- route scientific questions to domain tools;
- construct and validate workflows;
- generate hypotheses;
- request simulations or external experiments;
- compare outputs across tools;
- check unit, conservation, and consistency constraints;
- manage iterative closed-loop campaigns;
- produce evidence-linked reports.

The LLM/agent layer must not directly rewrite scientific state without validated tool calls and provenance.

### Layer 6 — closed-loop experimental learning

Future integration may include:

- experiment proposal;
- feasibility and safety constraints;
- simulation-based prioritization;
- laboratory execution or imported measurements;
- discrepancy analysis;
- model/parameter update;
- revalidation and audit trail.

## 6. GenESyS responsibilities

GenESyS should evolve to provide:

- a reusable simulation runtime;
- formal model execution;
- hybrid discrete-continuous orchestration;
- mechanistic validation;
- conservation and invariant checks;
- synthetic trajectory generation;
- uncertainty and replication management;
- model persistence and versioning;
- domain-tool interfaces;
- benchmark execution;
- provenance-rich outputs;
- integration points for AI/agent orchestration.

It should not embed all external databases, foundation models, or laboratory systems directly into the kernel.

## 7. Scientific claim levels

The project-wide minimum maturity target is Level 3 for supported software functionality, but scientific claim level is a separate axis.

Scientific outputs must be labelled as one of:

1. educational/demonstrative;
2. mechanistic research prototype;
3. quantitatively validated for a declared system/domain;
4. predictive claim supported by prospective independent validation.

A Level 3 software implementation can still have only a mechanistic-prototype scientific claim. Software maturity must not be confused with biological predictive validity.

## 8. Research work packages

### VC0 — literature and project landscape

- collect the primary papers;
- study virtual yeast, WAY, Bunne et al., Karr lineage, and closed-loop biology projects;
- identify open-source code, datasets, ontologies, and licenses;
- produce a comparison matrix.

### VC1 — GenESyS capability inventory

- map current whole-cell, biochemical, SBML, continuous, modal, AI, and worker components;
- classify what is implemented, scaffolded, broken, or historical;
- identify scientific and software gaps.

### VC2 — canonical data and knowledge model

- define species, compartments, reactions, genes, proteins, states, perturbations, units, evidence, and provenance;
- define mapping between native GenESyS models and SBML/other formats;
- define the three-pillar data model.

### VC3 — mechanistic tool contracts

- define typed interfaces for domain simulators/validators;
- isolate kernel-independent services;
- add benchmark fixtures and diagnostics;
- define versioning and capability discovery.

### VC4 — multimodal/learned model integration

- define embedding/state/transition interfaces;
- select initial datasets and tasks;
- establish uncertainty and out-of-domain policy;
- integrate learned components as tools, not hidden replacements.

### VC5 — agent orchestration

- define workflow graph and tool-call contracts;
- implement evidence/provenance capture;
- add consistency checks and human approval points;
- prevent unverified LLM output from becoming scientific state.

### VC6 — closed-loop prototype

- choose one bounded yeast or microbial use case;
- combine mechanistic simulation, learned prediction, and experiment-selection logic;
- validate against a declared dataset or controlled experimental collaboration;
- record all iterations and model updates.

## 9. Initial bounded use case recommendation

A future scientific project should begin with one bounded organism/process and a small subset of modules rather than claim a complete virtual cell.

Candidate characteristics:

- data-rich organism such as yeast or a microbial model;
- well-defined perturbation-response task;
- curated mechanistic model;
- available time-series or perturbation omics;
- measurable outputs;
- explicit conservation/consistency constraints;
- tractable experimental or published validation path.

The exact organism, modules, dataset, and claim level require scientific-team approval.

## 10. Validation policy

Each tool/model must record:

- scientific question and supported domain;
- equations/algorithm and references;
- units and parameter sources;
- calibration and validation split;
- uncertainty and sensitivity;
- comparator models/simulators;
- seeds and solver configuration;
- data and model versions;
- supported and unsupported constructs;
- failure and out-of-domain behavior.

No silent semantic loss is acceptable in SBML or other interoperability paths.

## 11. Risks

- overclaiming biological validity from software tests;
- replacing mechanistic constraints with opaque learned predictions;
- data leakage and invalid benchmark splits;
- unit/identifier/ontology mismatch;
- unsupported SBML constructs silently ignored;
- non-identifiable parameters;
- unstable closed-loop feedback;
- LLM hallucination entering scientific state;
- incomplete provenance;
- licensing/privacy restrictions on biological data;
- excessive scope before a bounded prototype is validated.

## 12. Immediate next action

Do not implement this architecture in the current consolidation PR.

The next action is a dedicated research-planning round that collects the cited papers and project material, maps current GenESyS capabilities, and selects one bounded scientific use case.
