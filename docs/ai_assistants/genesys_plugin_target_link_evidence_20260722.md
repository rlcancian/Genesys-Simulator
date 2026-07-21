# GenESyS plugin target link and GLPK evidence — 2026-07-22

## 1. Scope

This document records generated link, symbol, and GLPK-present/absent evidence for the current static component targets:

- `genesys_plugins_components`;
- `genesys_plugins_components_minimal`.

It complements:

- `genesys_plugin_target_overlap_inventory_20260721.md`;
- `genesys_plugin_target_introspection_evidence_20260721.md`;
- issue #487.

No target, source list, compile definition, ABI, library type, runtime behavior, or test expectation was changed to produce this evidence.

## 2. Integrated diagnostic workflow

PR #491 added:

```text
.github/workflows/genesys-plugin-target-link-evidence.yml
```

Merged into `WorkInProgress` as:

```text
8520c542a80697aad16527a89cec6b4d675d5797
```

The workflow configures the existing `tests-unit` graph twice on Ubuntu 24.04:

1. GLPK development files installed;
2. GLPK development files absent.

Both jobs build:

```text
genesys_test_plugins_continuous_diffusion
```

The workflow extracts CMake File API codemodel data, archive symbols with `nm`, direct dependencies, final link fragments, and verbose build logs.

## 3. Validation checkpoint

Validated branch head:

```text
0e40d892d7fe9f0a34b4152f32710eab0825638e
```

### 3.1 Ordinary CI

Run `29873203393`:

- configure: passed;
- aggregate build: passed;
- CTest: passed;
- GUI GMDD diagnostics: passed.

### 3.2 Diagnostic matrix

Run `29873203342`:

- GLPK-present job: passed;
- GLPK-absent job: passed;
- focused target built in both modes;
- evidence extraction passed;
- artifact upload passed.

Artifacts:

| Mode | Artifact | ID | Digest |
|---|---|---:|---|
| GLPK present | `genesys-plugin-target-link-evidence-present` | `8512049198` | `sha256:7be663e231d7b7e1f59168dad61c784d22afdf191b3f77714fe5f0e2a276da6f` |
| GLPK absent | `genesys-plugin-target-link-evidence-absent` | `8512054888` | `sha256:1898e247713fd0d23454a6d6fb1b35bc2e8f0867473a44c5f8d6e7f08a05b029` |

## 4. Common generated state

In both dependency modes:

- full resolved source count: 84;
- minimal resolved source count: 84;
- source lists: identical;
- focused diffusion test directly depends on both targets;
- final link fragments contain both static archives.

The focused target therefore reaches two archives built from the same component source tree.

## 5. GLPK-absent evidence

Compile definitions:

```text
genesys_plugins_components
    GENESYS_HAS_PYTHON_INTEGRATION=1

genesys_plugins_components_minimal
    GENESYS_HAS_PYTHON_INTEGRATION=1
```

Symbol evidence:

| Property | Value |
|---|---:|
| Full defined global symbols | 26,258 |
| Minimal defined global symbols | 26,258 |
| Common symbols | 26,258 |
| Full-only symbols | 0 |
| Minimal-only symbols | 0 |

Classification:

- the two archives are symbol-equivalent in the GLPK-absent configuration;
- compiling both remains duplicate build work and archive storage;
- the final focused-test link contains both equivalent archives;
- no duplicate-symbol linker failure occurred in the validated build.

## 6. GLPK-present evidence

Compile definitions:

```text
genesys_plugins_components
    GENESYS_HAS_PYTHON_INTEGRATION=1
    GENESYS_HAVE_GLPK

genesys_plugins_components_minimal
    GENESYS_HAS_PYTHON_INTEGRATION=1
```

Symbol evidence:

| Property | Value |
|---|---:|
| Full defined global symbols | 26,205 |
| Minimal defined global symbols | 26,258 |
| Common symbols | 26,198 |
| Full-only symbols | 7 |
| Minimal-only symbols | 60 |

The semantic divergence is concentrated in FBA implementation symbols.

Full-only includes:

```text
GlpkFluxBalanceSolver::solve(MetabolicFluxBalanceSolver::Problem const&)
```

Minimal-only includes:

```text
MetabolicFluxBalanceSolver::solve(MetabolicFluxBalanceSolver::Problem const&)
MetabolicFluxBalanceSolver::_dot(...)
MetabolicFluxBalanceSolver::_matrixRank(...)
MetabolicFluxBalanceSolver::_nextBasis(...)
MetabolicFluxBalanceSolver::_removeRedundantRows(...)
MetabolicFluxBalanceSolver::_satisfiesConstraints(...)
MetabolicFluxBalanceSolver::_solveLinearSystem(...)
MetabolicFluxBalanceSolver::_withinBounds(...)
```

The focused-test final link contains:

- `libgenesys_plugins_components.a`;
- `libgenesys_plugins_components_minimal.a`;
- `/usr/lib/x86_64-linux-gnu/libglpk.so`.

Classification:

- one executable reaches two archives compiled from the same 84 sources with different FBA implementations;
- the successful link means an active duplicate-definition linker failure was not demonstrated;
- archive extraction and link order determine which object definitions satisfy unresolved references;
- the current graph is materially ambiguous and should not be treated as a deliberate full/minimal architecture.

## 7. Confirmed conclusions

Confirmed:

1. There is no real source-level minimal/full partition.
2. Both archives are linked into the focused diffusion test.
3. Without GLPK, the archives expose identical global symbol sets.
4. With GLPK, they expose different FBA implementations.
5. The runtime-effective minimal archive still lacks `GENESYS_HAVE_GLPK`.
6. Duplicate compilation and storage are active costs.
7. The GLPK feature-selection inconsistency is a P1 build/runtime concern.

Not demonstrated:

- a current duplicate-symbol linker failure;
- a runtime crash caused by both archives;
- scientific correctness of either FBA implementation;
- behavioral equivalence of a future consolidated target;
- compatibility requirements for external consumers of either target name.

## 8. Architecture decision boundary

Issue #492 records the required human choice:

- **A:** one canonical static archive — recommended;
- **B:** define a true explicit minimal subset plus full archive;
- **C:** object-library/common-core composition;
- **D:** defer until dynamic migration — not recommended.

Until that decision is recorded, do not:

- remove or rename either target;
- move sources;
- change GLPK definitions or links;
- create aliases that implicitly select an architecture;
- begin shared/dynamic plugin migration.

## 9. Recommended decision

Select **Option A: one canonical static component archive**.

Rationale:

- the source sets are already identical;
- minimal has no documented exclusion contract;
- runtime uses minimal while GLPK is applied to full;
- one archive removes duplicate compilation and feature-selection ambiguity;
- it is the smallest architecture consistent with current behavior.

## 10. Required validation after a future decision

If Option A is approved, the implementation PR must validate:

1. GLPK-present codemodel and focused build;
2. GLPK-absent codemodel and fallback build;
3. ordinary `tests-unit` configure/build/CTest;
4. Phase 0 kernel/direct runner/CTest and smoke;
5. GUI GMDD diagnostics;
6. focused ASan/LeakSanitizer workflow;
7. Whole-Cell and FBA tests;
8. shell, worker, main GUI, and independent GUI application builds;
9. target-name compatibility or a temporary alias;
10. generated source, dependency, symbol, and final-link evidence after consolidation.

Do not combine the future correction with dynamic ABI work, Qt cleanup, worker security, optimizer algorithms, or scientific model validation.
