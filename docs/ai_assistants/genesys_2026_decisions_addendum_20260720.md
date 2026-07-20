# GenESyS 2026 Decisions Addendum — 2026-07-20

## 1. Purpose

This addendum records decisions and corrections supplied by Professor Rafael Luiz Cancian after the initial consolidation decision document was written.

It supersedes conflicting statements in `genesys_2026_human_decisions.md`, `genesys_2026_consolidation_plan.md`, `genesys_2026_consolidation_handoff.md`, and older documentation.

No functional code change is authorized by this document alone. Each implementation must use a bounded branch, tests, and review.

## 2. Semester branch naming correction

Status: `decided`.

The branch historically called `2026-1` was not deleted. It was renamed to:

```text
20261
```

The branch for the stable result of the second semester of 2026 will be:

```text
20262
```

The intended chronology is:

```text
feature branches -> WorkInProgress -> 20262 -> currentStable -> master
```

Promotion to `20262` is planned only for the end of the second semester of 2026. It is not an immediate consolidation action.

References to `2026-1` or `2026-2` in historical PRs, documents, workflows, or logs must be interpreted according to their date. Current governance documents must use `20261` and `20262`.

## 3. Dynamic plugin ABI decision

Status: `decided`, implementation `deferred`.

Plugins continue to be compiled together through the current static build graph during consolidation.

For the future dynamic-plugin architecture, the selected option is:

> Stable C ABI with opaque handles and versioned function tables.

The future boundary must avoid exposing STL, Qt, or implementation-specific C++ classes directly. The design must define at least:

- `extern "C"` discovery and entry points;
- fixed-width scalar types;
- opaque handles;
- explicit create/destroy ownership pairs;
- versioned function/capability tables;
- structured error reporting without cross-module exceptions;
- explicit string, array, buffer, callback, and allocator conventions;
- API/ABI version negotiation;
- plugin metadata and dependency requirements;
- lifecycle, unload, thread, and callback rules;
- compatibility tests across supported toolchain/package baselines.

This decision does not authorize an immediate migration. First stabilize the static graph and remove overlapping source aggregation.

## 4. Numerical and statistical references

Status: `deferred`, future plan recorded.

The professor will later gather:

- bibliographic references;
- relevant PDFs and converted Markdown/text;
- equations and algorithm specifications;
- authoritative datasets;
- expected numerical results;
- parameterization conventions;
- independent comparator results.

This cannot be completed in the current round. The work is tracked in:

```text
genesys_numerical_statistical_references_plan.md
```

Until a method has a declared reference package, it must not be promoted from provisional validation to scientifically validated behavior.

## 5. Maturity policy for the Optimizer and all GenESyS functionality

Status: `decided`.

The project is not interested in treating maturity levels 1 or 2 as acceptable final delivery states.

The project-wide target is:

1. bring all supported functionality to at least **Level 3 — Beta**;
2. after the supported set reaches Level 3, promote prioritized functionality to **Level 4 — Stable user feature**.

This policy applies to the Optimizer and to other supported GenESyS functionality.

Important interpretation:

- existing functionality below Level 3 remains accurately classified at its current level;
- it must not be relabelled as Beta without satisfying Level 3 acceptance criteria;
- unsupported, obsolete, or intentionally deferred functionality may be removed from the supported set rather than artificially promoted;
- Level 3 requires complete intended workflow, tests, error handling, reproducibility, cancellation where applicable, persistence where applicable, reporting, realistic fixtures, and documented limitations.

The current `OptimizerDefaultImpl1` remains an internal scaffold until a real algorithmic backend and Level 3 workflow exist.

## 6. Initial multiobjective optimization research direction

Status: `deferred`, research direction recorded.

Initial research will focus on:

- evolutionary multiobjective optimization;
- Pareto dominance and external archives;
- hypervolume as a quality indicator;
- hypervolume-based selection and many-objective optimization;
- methods and frameworks associated with ETH Zürich and Eckart Zitzler's research group;
- the PISA framework;
- HypE;
- SPEA/SPEA2;
- IBEA;
- ZDT and related benchmark suites;
- statistical performance assessment of stochastic multiobjective optimizers.

The professor's doctoral research remains a preferred source for the first GenESyS algorithms. The future work must combine the professor's algorithms, thesis material, and benchmark results with the ETH Zürich/PISA research baseline rather than replacing the professor's work with an arbitrary external implementation.

Tracked in:

```text
genesys_multiobjective_optimizer_future_plan.md
```

## 7. Worker deployment profile

Status: `decided`, security implementation pending.

The selected worker deployment profile is:

> Profile B — controlled academic intranet.

The normal use case is a worker running in a laboratory and receiving parallel simulation requests from authorized computers on the private academic network.

Policies:

- no direct public-Internet exposure by default;
- bind only to an explicitly selected private interface/address;
- firewall allowlist for laboratory clients/network ranges;
- authenticated clients;
- TLS for credentials and simulation data;
- cryptographically secure tokens or managed machine identity;
- job, payload, concurrency, CPU, memory, and time limits;
- no unrestricted shell execution;
- dedicated service account and restricted filesystem access;
- audit logging, request IDs, ownership, and result provenance;
- deny-by-default when authentication or configuration is missing.

The exact authentication mechanism remains open. Mutual TLS and short-lived signed tokens over TLS are the principal candidates.

## 8. Whole-cell, biochemical, and AI virtual-cell research direction

Status: `decided` as research direction; implementation and scientific scope `deferred`.

The project will improve the scientific level of whole-cell and biochemical capabilities while preparing a new scientific research project inspired by:

- Qian, Dong, and Guo, **Grow AI virtual cells: three data pillars and closed-loop learning**, Cell Research, 2025;
- Qian et al., **Towards the construction of a virtual yeast**, Nature, 2026;
- the WAY — Westlake AI Virtual Cell–Yeast direction;
- the AI virtual-cell priorities described by Bunne et al.;
- classical mechanistic whole-cell modeling, including the Karr et al. lineage.

The intended architectural interpretation is neuro-symbolic-mechanistic:

- classical mechanistic models are not replaced by LLMs;
- ODE, SSA, Petri-net, metabolic, constraint-based, spatial, and other formal models remain verifiable computational instruments;
- curated databases, mechanistic knowledge, subcellular architecture, and dynamic states form explicit data/knowledge layers;
- learned representations and transition operators may complement formal models;
- an agent/orchestration layer routes tasks, generates hypotheses, selects experiments, calls tools, and checks consistency;
- closed-loop active learning connects prediction, experiment selection, new data, and model revision;
- GenESyS acts as simulator, validator, synthetic-trajectory generator, conservation/invariant checker, mechanistic prior, and domain-tool host.

The research direction is tracked in:

```text
genesys_ai_virtual_cell_research_direction.md
```

Until domain-specific validation packages exist, public claims must remain explicit about what is mechanistically verified, empirically calibrated, experimentally validated, or still conceptual.

## 9. Promotion to `20262`

Status: timing `decided`; final gate `deferred`.

Promotion to `20262` will happen only at the end of the second semester of 2026.

During the semester, the team should accumulate evidence and improve maturity. It should not treat every consolidation PR as a release-promotion event.

Near the end of the semester, a dedicated release-readiness round will:

- define the supported feature set;
- classify each supported feature by maturity level;
- require at least Level 3 for the supported set or explicitly remove/defer lower-maturity items;
- run the agreed build, test, application, model, package, security, and documentation matrix;
- identify P0/P1 blockers;
- prepare release notes and rollback procedures;
- request explicit professor approval for promotion to `20262`.

The exact final checklist should be approved closer to the end-of-semester promotion window, based on the repository state at that time.

## 10. Summary

| Topic | Decision | Status |
|---|---|---|
| Historical first-semester branch | renamed from `2026-1` to `20261` | `decided` |
| Second-semester stable branch | `20262`, promoted only at semester end | `decided` |
| Current plugin build | keep static/joint compilation during consolidation | `decided` |
| Future dynamic ABI | stable C ABI, opaque handles, versioned function tables | `decided`, `deferred` |
| Numerical/statistical references | future acquisition and validation plan | `deferred` |
| Project maturity target | all supported functionality at least Level 3, then priorities to Level 4 | `decided` |
| Initial optimizer research | evolutionary multiobjective, hypervolume, ETH Zürich/PISA plus professor's thesis | `deferred` |
| Worker profile | controlled academic intranet | `decided` |
| Whole-cell direction | neuro-symbolic-mechanistic AI virtual-cell research | `decided`, `deferred` |
| Promotion gate | prepare during semester; execute for `20262` only at semester end | `decided` |
