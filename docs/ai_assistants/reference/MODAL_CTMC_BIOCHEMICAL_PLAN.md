---
document_type: reference
authority: technical-plan
owner: project-maintainer
last_reviewed: 2026-10-04
review_cadence: on-ctmc-contract-change
status: active
tracks: HUM-SCI-002
---

# Academic CTMC for Biochemical Reaction Networks — Next Phase Plan

## 1. Purpose

This document defines the next backend phase after the ModalModel / DefaultNetwork backend gate closed on 2026-09-20.

The maintainer decision dated 2026-10-04 is to develop an academically grounded **continuous-time Markov chain (CTMC) / Markov jump process model for biochemical reaction networks**. This phase remains backend-only; the Modal/Network GUI remains deferred.

The goal is not to add a superficial continuous-time option to the existing DTMC. The new work must reconcile stochastic chemical kinetics, GenESyS event time, persistence, reproducibility, and the biochemical/whole-cell code that already exists.

## 2. Current-code facts that constrain the design

Confirmed in `WorkInProgress` at the 2026-09-20 backend-gate checkpoint:

- `MarkovChainNetwork` is explicitly a finite time-homogeneous **DTMC**; one activation performs exactly one discrete Markov step using transition probabilities.
- `StochasticReactionComponent` already implements a Gillespie Direct Method SSA loop for a configurable local time window and uses all `StochasticReactionRule` definitions in the model.
- `StochasticReactionRule` already represents integer reactant/product stoichiometry and computes a mass-action-style stochastic propensity as a rate constant multiplied by combinatorial reactant-count factors.
- `StochasticReactionComponent` currently maintains its own RNG (`std::mt19937`) and advances a whole-cell-local clock by a configured time window when requested.

Therefore the CTMC effort is **not greenfield**. It must first determine which existing biochemical stochastic abstractions are scientifically correct and reusable, which timing/RNG behavior must be changed, and how the resulting semantics fit the network-centered architecture without creating a second incompatible stochastic-reaction engine.

## 3. Scientific target

The reference mathematical object is a time-homogeneous stochastic reaction network represented as a CTMC on a discrete state space.

For `S` molecular species, the state is a molecule-count vector

```text
X(t) ∈ N_0^S.
```

For reaction channel `r`, define:

- reactant and product stoichiometry;
- stoichiometric change vector `ν_r`;
- propensity/hazard `a_r(x) >= 0` in state `x`.

For `a_0(x) = Σ_r a_r(x)`:

- if `a_0(x) = 0`, the state is absorbing until an external event changes the model state;
- otherwise the holding time to the next reaction is exponentially distributed with rate `a_0(x)`;
- conditional on an event occurring, reaction `r` is selected with probability `a_r(x) / a_0(x)`;
- the next state is `x + ν_r`.

The infinitesimal generator must be consistent with

```text
q(x, x + ν_r) = a_r(x)
q(x, x)         = -Σ_{y != x} q(x, y)
```

with rates combined when multiple reaction channels lead to the same destination state.

The Chemical Master Equation (CME) is the analytical probability-law reference. Gillespie's Direct Method is the initial exact sample-path simulation reference for the well-mixed model; approximate methods are not the first implementation target.

## 4. Initial supported biochemical scope

The first academically validated subset should be deliberately bounded:

- well-mixed stochastic reaction systems;
- discrete, non-negative integer molecule counts;
- finite set of reaction channels;
- time-homogeneous parameters during an uninterrupted CTMC interval;
- explicit stoichiometric reactant/product changes;
- stochastic mass-action propensities as the mandatory baseline;
- exact event-by-event SSA semantics;
- absorbing states;
- deterministic replication reset and reproducible stochastic streams;
- current-format persistence and load/save symmetry.

The first validation package should cover zero-order, unimolecular and bimolecular/multi-reactant mass-action cases, including repeated reactants such as `2A -> ...`, because the stochastic combinatorial factor is part of the scientific contract.

General arbitrary propensity expressions may be considered after the current parser, units, parameter ownership and reproducibility contracts are audited. They are not required merely to start the CTMC implementation.

## 5. Architecture questions that must be resolved before implementation

### 5.1 CTMC versus the existing DTMC

Do not reinterpret `MarkovChainNetwork` probabilities as continuous-time rates.

The current DTMC contract should remain stable. The implementation phase must determine the smallest clean architecture for a distinct CTMC specialization or a justified shared Markov abstraction. The exact C++ class names are intentionally not prescribed by this document.

### 5.2 Reuse of biochemical reaction definitions

Before creating new species/reaction types, audit at least:

- `StochasticReactionRule`;
- `StochasticReactionComponent`;
- `MolecularSpecies`;
- `WholeCellState`;
- biochemical reaction-network data definitions and `BioSimulate` paths;
- current persistence/registration for those types.

Prefer one scientifically coherent reaction/stoichiometry/propensity representation over duplicate CTMC-only and WholeCell-only definitions when the contracts are genuinely compatible.

### 5.3 Simulation-time integration

This is a mandatory design gate.

The current WholeCell SSA executes many reactions inside a local fixed time window. A network CTMC, however, naturally samples the next reaction time. The next implementation must establish how that sampled time interacts with the GenESyS discrete-event calendar and with `ModalModelDefault` activation.

At minimum, compare the following architectural possibilities against the real event-flow code:

- schedule one CTMC reaction as a GenESyS future event at the sampled holding time;
- expose the sampled delay through a bounded network/adapter contract;
- retain a windowed SSA service only for explicitly isolated whole-cell use cases.

Do not allow an internal biochemical clock and the GenESyS simulation clock to drift silently. This work directly intersects `HUM-SCI-002` (modal/hybrid time synchronization contract).

### 5.4 Random-number infrastructure

The existing WholeCell SSA currently uses a private `std::mt19937`. The implementation phase must evaluate migration to the reproducible GenESyS RNG/sampler infrastructure used by the completed network formalisms, while preserving statistically correct independent uniforms and deterministic reset/seed behavior.

No RNG replacement should be made before focused sequence/reproducibility tests define the intended contract.

## 6. Academic validation requirements

Implementation is not accepted merely because trajectories look plausible.

At minimum, create reference-backed tests/oracles for:

1. **Single first-order reaction** `A -> B` with rate `k` and one initial `A`:
   - `P(A remains at t) = exp(-k t)`;
   - `P(B formed by t) = 1 - exp(-k t)`.
2. **Independent first-order decay** of `N` identical molecules:
   - molecule count at time `t` follows the corresponding binomial survival law;
   - expected count is `N exp(-k t)`.
3. **Competing reactions** `A -> B` and `A -> C`:
   - holding time rate is `k1 + k2` for one `A`;
   - next-reaction probabilities are `k1/(k1+k2)` and `k2/(k1+k2)`.
4. **Two-state reversible CTMC**:
   - transient probabilities must match the closed-form two-state solution;
   - long-run proportions must match the stationary distribution when the chain is irreducible.
5. **Stoichiometric combinatorics**:
   - insufficient reactants imply zero propensity;
   - repeated-reactant channels use the correct combinatorial factor.
6. **Generator invariants**:
   - off-diagonal rates are non-negative;
   - diagonal entries are non-positive;
   - each generator row sums to zero within a declared numerical tolerance.
7. **Reproducibility**:
   - controlled seed/reset reproduces the same event sequence and holding times;
   - statistical validation uses ensembles and justified tolerances rather than a single trajectory.
8. **Persistence**:
   - construct -> check -> save -> recreate -> load -> check -> execute preserves stoichiometry, rates/propensities, state and configuration without duplicating canonical model definitions.

The implementation plan should distinguish exact analytical assertions from Monte Carlo distributional assertions and record tolerances/provenance explicitly.

## 7. Initial academic references

The specification and tests should start from the following authoritative references and may add others after review:

- Gillespie, D. T. (1976), *A General Method for Numerically Simulating the Stochastic Time Evolution of Coupled Chemical Reactions*, Journal of Computational Physics 22(4), 403–434. DOI: `10.1016/0021-9991(76)90041-3`.
- Gillespie, D. T. (1977), *Exact Stochastic Simulation of Coupled Chemical Reactions*, Journal of Physical Chemistry 81(25), 2340–2361. DOI: `10.1021/j100540a008`.
- Anderson, D. F.; Kurtz, T. G. (2015), *Stochastic Analysis of Biochemical Systems*, Springer. DOI: `10.1007/978-3-319-16895-1`.

These references define the initial scientific foundation, not a license to copy algorithms blindly. GenESyS still requires explicit contracts for units, state ownership, event scheduling, persistence and reproducibility.

## 8. Next-phase work packages

The next local agent should execute the phase in this order:

1. **Current-code audit** — map DTMC, WholeCell SSA, stochastic reaction rules, biochemical network definitions, time handling, RNG, persistence and ownership.
2. **Scientific contract** — write the precise CTMC state/rate/stoichiometry/propensity semantics, unit conventions and supported subset, tied to references.
3. **Time-integration decision** — reconcile sampled CTMC holding times with the GenESyS event calendar and `ModalModelDefault`; resolve or narrow `HUM-SCI-002` before changing runtime behavior.
4. **Consolidation design** — decide what is reused from the WholeCell stochastic path and what becomes network-domain functionality; avoid parallel scientific engines.
5. **Reference tests first** — implement analytical and stochastic validation fixtures before the main runtime implementation.
6. **Bounded implementation PRs** — only after the preceding contracts are accepted, implement the CTMC network/runtime in small reviewable changes.
7. **Integration validation** — persistence, factory/registration, reset/reproducibility, aggregate regression, sanitizers where ownership changes, and manual updates.

## 9. Explicit non-goals for the first CTMC phase

Do not combine this work with:

- Modal/Network GUI implementation;
- Cellular Automata migration;
- MDPs or controlled Markov processes;
- spatial stochastic reaction-diffusion/RDME;
- tau-leaping or other approximate accelerations as the primary solver;
- Chemical Langevin/SDE approximations;
- deterministic ODE replacement;
- broad SBML import/export changes;
- full whole-cell architecture redesign;
- unrelated plugin/dynamic-ABI work.

Those may become later phases after the exact well-mixed CTMC contract is validated.

## 10. Completion criterion for this next phase

The CTMC phase must not be called complete until the implemented model is demonstrably a scientifically coherent continuous-time Markov jump process for the declared biochemical subset, with reference-backed mathematics, correct GenESyS time integration, reproducible stochastic execution, persistence, focused tests and aggregate regression.

Until the time-integration contract and reuse boundary with `StochasticReactionComponent` are resolved, the next step is **specification and validation design, not immediate code duplication**.
