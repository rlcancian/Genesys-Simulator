# GenESyS generated plugin target introspection evidence — 2026-07-21

## 1. Scope

This document records the generated CMake File API codemodel evidence for:

- `genesys_plugins_components`;
- `genesys_plugins_components_minimal`.

It complements `genesys_plugin_target_overlap_inventory_20260721.md` and issue #487. It is evidence-only: no target, source list, link dependency, compile definition, ABI, library type, or runtime behavior is changed.

## 2. Validated checkpoint

- Branch head: `91814e966cfd9d1b523a29905dfdc8d3051bae50`;
- pull request: #489;
- ordinary CI run: `29856007581` — passed;
- introspection run: `29856007683` — passed;
- artifact: `genesys-plugin-target-introspection`;
- artifact ID: `8505306513`;
- artifact digest: `sha256:4f8075d8098d483f37b4ad5d72cec9ecb62ea6ad034268547bbf2efa50c27ba7`;
- configuration: existing `tests-unit` preset on Ubuntu 24.04 with `libglpk-dev` installed;
- data source: CMake File API codemodel v2.

PR #489 was merged into `WorkInProgress` as:

```text
b3e027483066b903a0389285082163b86c2df362
```

## 3. Generated source evidence

| Property | Full | Minimal |
|---|---:|---:|
| Resolved `.cpp` source count | 84 | 84 |
| Source lists identical | yes | yes |
| `GENESYS_HAS_PYTHON_INTEGRATION=1` | yes | yes |
| `GENESYS_HAVE_GLPK` | yes | no |

The codemodel therefore confirms that the two static targets compile the same 84 component translation units while using materially different compile definitions.

This is not a genuine minimal/full source partition.

## 4. Direct dependency evidence

Both component targets depend directly on:

- `genesys_ai_provider` through the public `genesys_plugins_data` path exposed by the generated codemodel;
- `genesys_kernel_util`;
- `genesys_plugins_data`.

The relevant semantic divergence is not the dependency set but the full target's private GLPK configuration.

## 5. Reverse dependency evidence

### 5.1 Full target

The generated `tests-unit` codemodel reports one direct reverse dependency:

```text
genesys_test_plugins_continuous_diffusion
```

No runtime library or application target directly depends on `genesys_plugins_components` in this generated configuration.

Classification: the full target is **test-only in the validated codemodel** and remains an ALL-built duplicate archive.

### 5.2 Minimal target

The generated codemodel reports 27 direct reverse dependencies:

- `genesys_diffusion_ascii_demo`;
- `genesys_kernel`;
- `genesys_kernel_simulator_runtime`;
- `genesys_test_ai_plugins`;
- `genesys_test_cellular_automata_neighborhood`;
- `genesys_test_delay_statistics_lifecycle`;
- `genesys_test_gui_gmdd_layout`;
- `genesys_test_gui_gmdd_layout_autogen_timestamp_deps`;
- `genesys_test_optimizer_ownership_contract`;
- `genesys_test_parser_expressions`;
- `genesys_test_plugins_continuous_diffusion`;
- `genesys_test_queue_statistics_lifecycle`;
- `genesys_test_resource_accounting_lifecycle`;
- `genesys_test_runtime_pluginmanager`;
- `genesys_test_search_remove_runtime`;
- `genesys_test_simulator_plugin_completion_lifetime`;
- `genesys_test_simulator_runtime`;
- `genesys_test_station_statistics_lifecycle`;
- `genesys_test_tools_diffusion_mol`;
- `genesys_test_tools_hypothesistester`;
- `genesys_test_tools_ode_solver_factory`;
- `genesys_test_tools_simulation_results_dataset`;
- `genesys_test_wcm_plugins`;
- `genesys_test_worker_api_router`;
- `genesys_tools`;
- `genesys_worker_app`;
- `genesys_worker_core`.

Classification: the minimal target is the effective static component archive for the generated runtime/test graph.

## 6. Final link-line evidence

The generated final link fragments for `genesys_worker_app` include:

```text
-Wl,--start-group
source/applications/worker/libgenesys_worker_core.a
source/kernel/util/libgenesys_kernel_util.a
source/kernel/statistics/libgenesys_kernel_statistics.a
source/parser/libgenesys_parser.a
source/plugins/data/libgenesys_plugins_data.a
source/plugins/libgenesys_plugins_components_minimal.a
source/kernel/simulator/libgenesys_kernel_simulator_support.a
source/kernel/simulator/libgenesys_kernel_simulator_runtime.a
-Wl,--end-group
source/tools/AIAssistant/libgenesys_ai_provider.a
/usr/lib/x86_64-linux-gnu/libpython3.12.so
```

The full archive is absent from this executable link line.

The selected preset did not expose a final link command for the aggregate custom target `genesys_kernel_unit_tests`, which is expected because custom aggregate targets do not necessarily have a linker invocation.

## 7. GLPK feature-selection inconsistency

Confirmed generated state:

```text
genesys_plugins_components
    84 sources
    GENESYS_HAVE_GLPK
    only direct consumer: genesys_test_plugins_continuous_diffusion

genesys_plugins_components_minimal
    same 84 sources
    no GENESYS_HAVE_GLPK
    runtime/tools/worker/test consumers
```

Consequences:

1. GLPK detection and the configure message apply to the duplicate archive that is not selected by the principal runtime path in this codemodel.
2. The effective runtime archive compiles `MetabolicFluxBalance.cpp` and Whole-Cell component sources without `GENESYS_HAVE_GLPK`.
3. Installing GLPK therefore does not, in the generated runtime graph, prove that applications use the GLPK-backed component implementation.
4. A full-target test may exercise a different compile-time implementation than runtime applications.

Classification: **confirmed P1 build/feature-selection inconsistency**.

## 8. Duplicate compilation and ODR assessment

Confirmed:

- two static archives compile the same 84 translation units;
- the archives use different compile definitions;
- duplicate build time and artifact storage exist;
- one focused test directly depends on both exact targets because `genesys_test_plugins_continuous_diffusion` appears in both reverse-dependency lists.

Not yet proven:

- that one final linker invocation extracts duplicate definitions from both archives;
- that a current executable has an active ODR violation;
- that removing one target is behaviorally neutral across GLPK-enabled and GLPK-disabled configurations.

The `RESCAN` static link groups make symbol extraction order and archive reachability nontrivial. A symbol/link-map check is still useful before deleting a target, although source and feature duplication are already established.

## 9. Architecture options requiring human decision

### Option A — one canonical static component archive

Use one target for the current static runtime and tests, apply GLPK consistently to it, and remove or compatibility-alias the duplicate target.

Advantages:

- smallest conceptual model;
- removes duplicate compilation;
- removes GLPK divergence;
- aligns current reality, where no real minimal subset exists;
- lowest maintenance cost.

Risks:

- target-name compatibility must be handled;
- requires validation with and without GLPK;
- may expose hidden assumptions in the full-target diffusion test.

Recommended immediate architecture based on current evidence.

### Option B — define a true explicit minimal subset

Retain two archives, but replace recursive duplicate globs with explicit manifests and document which component categories are excluded from minimal.

Advantages:

- preserves a lightweight runtime if that is a real requirement;
- permits feature-specific dependencies such as GLPK only in full.

Risks:

- requires a product/architecture definition of “minimal”;
- runtime may lose currently available plugin categories;
- substantially larger test matrix;
- source manifests must remain synchronized.

Choose only if a smaller supported runtime is an explicit requirement.

### Option C — common object-library/core arrangement

Compile shared component objects once and compose static archives or executables from them.

Advantages:

- can reduce duplicate compilation;
- can support multiple facades.

Risks:

- compile definitions such as `GENESYS_HAVE_GLPK` cannot diverge safely for the same object set;
- increases CMake complexity;
- does not define what “minimal” means;
- should not precede a target-contract decision.

Not recommended as the first correction.

### Option D — defer static cleanup until dynamic plugin migration

Leave both targets unchanged until the future stable C ABI plugin architecture is implemented.

Advantages:

- avoids intermediate target migration.

Risks:

- preserves duplicate compilation and the confirmed GLPK runtime inconsistency;
- dynamic migration is deferred and materially larger;
- current static behavior remains misleading.

Not recommended for the P1 inconsistency.

## 10. Recommended bounded correction

Subject to human approval of Option A:

1. choose one canonical target name;
2. compile the 84 component sources once;
3. apply GLPK detection/definition/linking to that canonical target;
4. migrate the focused diffusion test and runtime consumers to the canonical target;
5. preserve a temporary compatibility alias only if downstream target-name compatibility is required;
6. run generated introspection with GLPK installed and absent;
7. run ordinary CI, Phase 0 kernel/smoke, focused ASan/LSan, GUI GMDD, and Whole-Cell/FBA tests;
8. only afterward simplify redundant direct/transitive `LINK_GROUP` entries in a separate PR.

Do not combine this correction with:

- dynamic/shared plugin migration;
- stable C ABI implementation;
- Qt6-only cleanup;
- worker security;
- optimizer algorithms;
- scientific validation.

## 11. Decision blocker

A human decision is required on the intended static architecture:

- **A:** one canonical static archive — recommended;
- **B:** a true explicitly defined minimal subset plus full archive;
- **C:** object-library/common-core composition;
- **D:** defer until dynamic migration — not recommended.

Until that decision is recorded, the safe state is to retain the generated introspection workflow and avoid target modifications.

## 12. Remaining validation limits

- Codemodel evidence was generated only with the `tests-unit` preset.
- Not all GUI/application presets expose final targets in that configuration.
- GLPK-absent codemodel was not captured in this run.
- No `nm`, `readelf`, or linker map was captured.
- No target behavior was changed.
- Dynamic plugin loading remains deferred.
