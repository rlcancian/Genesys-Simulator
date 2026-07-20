# Plugins Development Guidance

## Purpose

This document is the stable AI-assistant entrypoint for GenESyS plugin-related work.

Detailed plugin guidance is split by domain under `docs/ai_assistants/plugins/`.

## Domain guides

- `plugins/electronic.md`: electronics/SPICE-related plugins.
- `plugins/external_integration.md`: external integration plugins, including C++/Python-facing integration notes.
- `plugins/biochemical.md`: biochemical simulation plugins and SBML-facing biosimulation context.
- `plugins/whole_cell_model.md`: whole-cell modeling plugins.
- `plugins/modal_model.md`: modal model, EFSM, Petri net, and submodel plugins.
- `plugins/continuous_hybrid.md`: continuous and hybrid/discrete-continuous plugins.
- `plugins/other_plugins.md`: standard discrete-event and general-purpose plugin categories.

## General policy

Plugin changes have higher regression risk than local implementation changes because they may affect model load/save, factories, metadata contracts, parser integration, data-definition lifecycle, GUI catalog behavior, and future packaging.

Prefer small, reviewable changes with explicit validation.

## Current static-build decision

Decision date: 2026-07-20.

For the current consolidation phase, plugins remain compiled together with GenESyS through the static CMake build graph. Do not begin a broad conversion to separately distributed dynamic libraries during baseline stabilization.

Current plugin work should instead:

- map every plugin source to its CMake target and final executable;
- remove or redesign overlapping source aggregation between full/minimal static plugin targets;
- document registration, factories, metadata, dependencies, persistence, and lifecycle;
- preserve existing plugin semantics while unit/smoke/CI baselines are stabilized;
- keep dynamic-plugin research and student implementations as historical/design input, not as an approved production migration.

Status:

- current static aggregation policy: `decided`;
- dynamic plugin migration: `deferred`;
- future ABI architecture: `decided` as a stable C ABI boundary, implementation deferred.

## Future dynamic-plugin ABI decision

The selected long-term in-process dynamic-plugin boundary is:

> Stable C ABI with opaque handles and versioned function tables.

Future design requirements:

- `extern "C"` entry points;
- fixed-width scalar types;
- opaque handles rather than exposed C++ object layouts;
- explicit create/destroy ownership functions;
- versioned capability/function tables;
- no STL or Qt types crossing the ABI;
- no C++ exceptions crossing the ABI;
- explicit string, array, callback, buffer, and allocator conventions;
- structured error/diagnostic results;
- API/ABI version negotiation and deprecation policy;
- plugin metadata and dependency declarations;
- lifecycle, unload, callback, and thread-safety contracts;
- compatibility tests for supported operating-system/toolchain/package baselines.

This decision does not authorize immediate implementation. The static graph must first be stabilized and made non-overlapping.

Out-of-process services remain appropriate for coarse-grained external solvers, AI services, distributed workers, or untrusted extensions, but they are not the selected replacement for ordinary in-process plugin components.

See:

- `genesys_2026_decisions_addendum_20260720.md`;
- `genesys_2026_human_decisions.md` for the earlier option analysis.

## Source areas

Primary source areas:

- `source/plugins/`
- `source/plugins/components/`
- `source/plugins/data/`
- plugin-related kernel interfaces and managers
- GUI extension points that depend on simulator plugins

## Historical source documents

Historical material remains under `docs/ai_assistants/oldies/`. Do not treat historical notes as current facts without revalidation.

Primary historical sources for plugin work:

- `old_plugin_components_method_matrix.md`
- `old_plugin_data_definitions_audit_WiP20261.md`
- `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md`
- `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md`
- `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md`

## Cross-cutting review rules

- Distinguish model plugins, data-definition plugins, component plugins, and GUI extension plugins.
- Do not invent plugin factories or metadata fields.
- Do not assume empty methods are unused.
- Do not change plugin names or exported symbols without searching all call sites.
- Keep plugin architecture changes separate from ordinary bug fixes.
- Validate load/save symmetry when persistence changes.
- Validate GUI dependency gating when GUI contributions depend on model plugins.
- Do not pass ownership of STL/Qt/C++ objects across a future binary boundary.
- Do not allow one module to allocate an object that another module destroys unless the ABI contract explicitly supplies the matching destroy function.

## Validation checklist

For plugin changes, prefer this order:

1. Configure and build the unit-test preset.
2. Run CTest with the unit-test preset.
3. Build the affected application preset if the plugin is GUI, worker, or terminal-visible.
4. Load a minimal model that exercises the changed plugin, when possible.
5. Validate save/load round trip if serialization changed.
6. Validate dependency gating if GUI contributions depend on model plugins.
7. Inspect link maps when modifying plugin target composition.
8. Run ASan/LSan or equivalent diagnostics for plugin ownership/lifecycle changes when a validated diagnostic configuration exists.
9. For future dynamic plugins, validate ABI version negotiation, invalid-version rejection, load/unload, error propagation, ownership, and cross-build compatibility.
