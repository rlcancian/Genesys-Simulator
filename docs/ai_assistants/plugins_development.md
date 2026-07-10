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

## Validation checklist

For plugin changes, prefer this order:

1. Configure and build the unit-test preset.
2. Run CTest with the unit-test preset.
3. Build the affected application preset if the plugin is GUI, web, or terminal-visible.
4. Load a minimal model that exercises the changed plugin, when possible.
5. Validate save/load round trip if serialization changed.
6. Validate dependency gating if GUI contributions depend on model plugins.
