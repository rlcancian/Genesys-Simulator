# GenESyS static plugin target overlap inventory — 2026-07-21

## 1. Scope

This document maps the current CMake source selection, compile configuration, dependencies, and consumers of:

- `genesys_plugins_components`;
- `genesys_plugins_components_minimal`.

It is evidence-only. No source, target, link list, ABI, library type, GLPK behavior, or runtime loading behavior is changed.

Tracking issue: #487.

## 2. Confirmed target definitions

### 2.1 Full target

Defined in:

```text
source/plugins/components/CMakeLists.txt
```

Source selection:

```cmake
file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_plugins_components STATIC
    ${GENESYS_PLUGINS_COMPONENTS_SOURCES}
)
```

Direct public dependencies:

- `genesys_kernel_util`;
- `genesys_plugins_data`.

Optional private GLPK configuration:

- `GENESYS_HAVE_GLPK` compile definition;
- GLPK include directories;
- GLPK libraries.

### 2.2 Minimal target

Defined one directory above in:

```text
source/plugins/CMakeLists.txt
```

Source selection:

```cmake
file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/components/*.cpp"
)

add_library(genesys_plugins_components_minimal STATIC
    ${GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES}
)
```

Direct public dependencies:

- `genesys_plugins_data`;
- `genesys_kernel_util`.

No GLPK compile definition, include directory, or library is applied to this target.

## 3. Source overlap

### 3.1 Confirmed equivalence of selection domains

The full glob is evaluated from `source/plugins/components` and selects recursively:

```text
source/plugins/components/**/*.cpp
```

The minimal glob is evaluated from `source/plugins` and selects recursively:

```text
source/plugins/components/**/*.cpp
```

Therefore, based on the current CMake expressions, both targets select the same source domain.

### 3.2 Classification

| Property | Full | Minimal | Result |
|---|---|---|---|
| Library type | STATIC | STATIC | same |
| Component source domain | all recursive component `.cpp` | all recursive component `.cpp` | same |
| Public data dependency | yes | yes | same |
| Public util dependency | yes | yes | same |
| C++ standard | C++23 | C++23 | same |
| GLPK definition/includes/link | optional private | absent | different |

Classification: **accidental or transitional overlap**, not a genuine minimal/full partition.

No evidence was found in the inspected CMake definitions of an explicit source subset, exclusion rule, category list, feature gate, or generated manifest that would make `minimal` smaller.

## 4. Runtime propagation

`genesys_kernel_simulator_runtime` links publicly to:

- `genesys_kernel_util`;
- `genesys_plugins_data`;
- `genesys_plugins_components_minimal`;
- `genesys_kernel_simulator_support`;
- `genesys_kernel_statistics`;
- `genesys_parser`.

Consequently, every consumer of `genesys_kernel_simulator_runtime` receives a transitive dependency path to `genesys_plugins_components_minimal`.

The root aggregate target `genesys_kernel` also depends explicitly on `genesys_plugins_components_minimal` and does not depend on the full target.

## 5. Direct consumers inspected

### 5.1 Applications

| Consumer | Direct plugin target in link group | Runtime also present | Redundancy |
|---|---|---:|---|
| `genesys_shell` | minimal | yes | minimal and several runtime-public dependencies repeated |
| `genesys_worker_app` | minimal | yes | minimal repeated; worker core also links runtime |
| `genesys_modelspecific_app` | minimal | yes | minimal and runtime-public dependencies repeated |
| `genesys_gui_application` | minimal | yes | minimal and runtime-public dependencies repeated |
| `genesys_dataanalyser_gui_application` | minimal | yes | minimal and runtime-public dependencies repeated |
| `genesys_optimizer_gui_application` | minimal | yes | minimal and runtime-public dependencies repeated |
| `genesys_ai_assistant_gui_application` | minimal | yes | minimal and runtime-public dependencies repeated |

### 5.2 Worker core

`genesys_worker_core` links directly to `genesys_kernel_simulator_runtime`; therefore it receives the minimal component archive transitively.

`genesys_worker_app` then links both `genesys_worker_core` and the full explicit runtime/minimal group, creating more than one dependency path to the same static archives.

### 5.3 Smoke tests

The inspected smoke targets link `genesys_plugins_components_minimal` directly and also link `genesys_kernel_simulator_runtime`:

- `genesys_smoke_simulator_start`;
- `genesys_test_continuous_system`;
- `genesys_test_lsode`;
- `genesys_demo_continuous_trace`.

`genesys_smoke_simulator_start` additionally lists `genesys_kernel_simulator_runtime` twice in the same link declaration.

### 5.4 Unit and focused tests

The test orchestration repeatedly uses link groups containing both:

- `genesys_plugins_components_minimal`;
- `genesys_kernel_simulator_runtime`.

This is consistent with the application pattern and intentionally relies on `LINK_GROUP:RESCAN` to resolve static archive cycles.

## 6. Full-target consumer finding

Current repository search and inspected CMake files did not identify an executable, test, aggregate target, or runtime library that links the exact target `genesys_plugins_components`.

Observed references to the full name are:

- its own target definition;
- comments/messages about plugin static libraries;
- references where the searched substring also matches `genesys_plugins_components_minimal`.

Classification: **strong indication that the full target is currently an ALL-built but link-orphaned archive**.

This is not yet promoted to an absolute repository-wide conclusion because the connector search is not a generated CMake graph. A directed CMake File API or `cmake --graphviz` validation should confirm it before target removal.

## 7. GLPK semantic divergence

The same component source tree is compiled twice, but only the full archive receives:

```text
GENESYS_HAVE_GLPK
```

and GLPK include/link settings.

The runtime and all inspected applications/tests use the minimal archive. Therefore:

- the archive configured for GLPK appears not to be the archive selected by runtime consumers;
- components such as `MetabolicFluxBalance` and WholeCell FBA may compile into the runtime archive without `GENESYS_HAVE_GLPK`, even when GLPK was detected for the full archive;
- the configure message can state that GLPK-backed behavior is enabled while the linked runtime path may still use the built-in fallback.

Classification: **P1 build/feature-selection inconsistency** and **strong functional risk**.

This document does not claim the final executable behavior without inspecting generated compile commands and link graphs for a GLPK-enabled configuration.

## 8. ODR and static-link risk

### 8.1 Two archives with duplicate object definitions

The two targets compile the same translation units into separate static archives. This creates duplicate symbol definitions at the archive level.

### 8.2 Current final-link exposure

No inspected final target links both exact archives. Therefore, duplicate compilation is confirmed, while a final-link ODR violation caused by linking full and minimal together is **not currently confirmed**.

### 8.3 Rescan groups

Most final consumers place minimal, runtime, data, parser, statistics, support, and util inside a `RESCAN` group. This addresses cyclic archive resolution but also obscures redundant direct/transitive dependencies and makes future addition of the full archive riskier.

Classification:

- duplicate compile/storage cost: **confirmed**;
- different feature macros for duplicate source domain: **confirmed**;
- current final executable containing both full and minimal archives: **not observed**;
- future ODR risk if both are linked: **high**.

## 9. Dependency graph summary

```text
components/**/*.cpp
    ├── genesys_plugins_components [STATIC, optional GENESYS_HAVE_GLPK]
    │     └── no exact consumer observed
    │
    └── genesys_plugins_components_minimal [STATIC, no GLPK settings]
          └── genesys_kernel_simulator_runtime [PUBLIC]
                ├── genesys_worker_core
                ├── shell / worker / model-specific apps
                ├── main and independent GUIs
                └── unit, focused, and smoke tests
```

Many final consumers also list minimal and runtime directly in the same `LINK_GROUP:RESCAN`.

## 10. Recommended bounded follow-up

### 10.1 First validation-only PR

Add a CMake graph/introspection check without changing target behavior:

1. generate or inspect the CMake File API target graph for representative presets;
2. extract resolved source lists for full and minimal;
3. assert and report whether the lists are identical;
4. list exact reverse dependencies of both targets;
5. capture compile definitions for GLPK-enabled and GLPK-disabled configurations;
6. capture final link lines for shell, worker, main GUI, optimizer GUI, unit aggregate, and smoke targets.

### 10.2 Candidate correction after graph confirmation

Preferred small correction if the generated graph confirms this inventory:

- select one canonical component archive for the static runtime;
- apply GLPK configuration to that canonical archive;
- remove the orphan duplicate target or redefine `minimal` as an alias only if CMake and downstream compatibility permit;
- simplify direct link lists only in a separate PR after proving transitive interfaces are sufficient.

Do not combine target consolidation with dynamic plugin ABI work.

### 10.3 Alternative if a real minimal subset is required

Define an explicit source manifest for the minimal runtime and document excluded categories. Do not use two recursive globs over the same directory.

## 11. Human decision boundary

No human decision is required to run the graph/introspection validation.

A human architectural decision is required before choosing between:

- one canonical static archive;
- a true explicit minimal subset plus a full archive;
- a future object-library/common-core arrangement;
- dynamic/shared plugin migration.

The already recorded future ABI choice remains a stable C ABI with opaque handles and versioned function tables. That choice does not determine the immediate static-target cleanup.

## 12. Validation limitations

- This inventory is based on repository CMake source and connector search, not generated CMake File API replies.
- Exact source-file count was not generated in this documentation step.
- No GLPK-enabled build was executed.
- No final link map or symbol table was inspected.
- No target or runtime behavior was changed.

## 13. Conclusion

Confirmed:

- full and minimal select the same component source domain;
- the two archives differ materially in GLPK compile/link configuration;
- runtime and all inspected consumers select minimal directly or transitively;
- redundant direct/transitive link paths are widespread;
- the full target has no exact consumer observed.

Next safe action: generate target/source/reverse-dependency/link-line evidence from CMake itself before applying any target consolidation patch.
