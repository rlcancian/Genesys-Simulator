# Architecture notes for `source/tools`

## 1. Architectural overview

The `tools` package provides domain-support services for statistical analysis and numerical procedures. It exposes interfaces used by higher-level workflows and retains legacy implementations for compatibility.

## 2. Current package responsibilities

- Dataset-oriented analysis orchestration (`DataAnalyser_if`).
- Distribution fitting contracts (`Fitter_if`).
- Parametric hypothesis testing (`HypothesisTester_if`, default impl 1).
- Distribution mathematical utilities and quantile inversion.
- Numerical integration and derivation utilities via legacy solver abstractions.
- Binding of abstractions to implementations through traits.

## 3. Approved target decomposition

### Data analysis
- Keep façade role in `DataAnalyser_if`.
- Clarify ownership and lifecycle of returned collaborating services.

### Fitting
- Preserve legacy fitting API.
- Transition from dummy implementation to a concrete default implementation in phased manner (FITTER-1 delivered baseline behavior in `FitterDefaultImpl` without changing default trait binding).

### Hypothesis testing
- Maintain current contract and refine implementation completeness, especially two-population methods.

### Distribution abstractions
- Introduce `Distribution_if` and specialized continuous/discrete interfaces.
- Keep legacy static façade while migration is in progress.

### Numerical tools
- Split legacy solver responsibilities into dedicated interfaces:
  - `Quadrature_if`
  - `RootFinder_if`
  - `OdeSystem_if`
  - `OdeSolver_if`

### Traits/binding
- Keep current traits stable.
- Expand only when safe concrete implementations exist.

## 4. Legacy compatibility strategy

- Do not break existing public signatures in this phase.
- Preserve current trait selections to avoid runtime and build regressions.
- Add new abstractions as header-only contracts until implementations are mature.

## 5. Planned migration order

1. Stabilize documentation and explicit contracts for existing headers.
2. Introduce new interface headers without forcing behavior changes.
3. Add concrete implementations incrementally behind existing traits.
4. Migrate trait bindings once implementations are production-safe.
5. Deprecate legacy conflated abstractions only after full replacement is validated.

## 6. Stability notes and non-goals for this phase

- FITTER-1 introduces baseline fitting algorithms in `FitterDefaultImpl` (uniform, triangular, normal, exponential, erlang) while keeping trait defaults unchanged.
- No full completion of hypothesis-testing theory coverage.
- No numerical refactor of legacy solver internals.
- No cross-package behavior changes outside `source/tools`.
