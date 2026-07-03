# Plugins Development Guidance

## Purpose

This document is the stable AI-assistant reference for GenESyS plugin-related work.

It consolidates historical notes from `docs/ai_assistants/oldies/`. Historical findings must be checked against current source files before being treated as current facts.

## Scope

Primary source areas:

- `source/plugins/`
- `source/plugins/components/`
- `source/plugins/data/`
- plugin-related kernel interfaces and managers
- GUI extension points that depend on simulator plugins

Historical source documents:

- `old_plugin_components_method_matrix.md`
- `old_plugin_data_definitions_audit_WiP20261.md`
- `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md`
- `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md`
- `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md`

## General policy

Plugin changes have higher regression risk than local implementation changes because they may affect:

- model loading and saving;
- factory and metadata contracts;
- parser integration;
- data-definition lifecycle;
- GUI catalog and graphical extension behavior;
- future dynamic-library packaging.

Prefer small, reviewable changes with explicit validation.

## Component plugins

Historical component-matrix notes classify component methods by implementation state. Treat that matrix as a triage aid, not as current proof.

Before modifying a component plugin, inspect at least:

- header and implementation file;
- `GetPluginInformation`;
- `LoadInstance`;
- `NewInstance`;
- `_loadInstance`;
- `_saveInstance`;
- `_check`;
- `_createInternalAndAttachedData`;
- `_initBetweenReplications`;
- `_addProperty`, when present.

Do not fill empty methods with placeholder behavior. Either preserve no-op semantics intentionally or implement behavior with tests and serialization/runtime validation.

## Data-definition plugins

Historical audit notes identified higher-risk data definitions including `Queue`, `Resource`, `Variable`, `Sequence`, and `SignalData`.

Guidance:

- Validate load/save symmetry before modifying persistence.
- Check parser-facing behavior before changing names, metadata, or class factories.
- Treat ownership of internal objects and attached data definitions as explicit design work, not a mechanical refactor.
- Avoid changing runtime semantics based only on comments or historical TODOs.

## GUI graphical plugins

Historical GUI audit notes indicate that the GUI has internal controllers and services, but generic third-party graphical extension points require careful integration.

Guidance:

- Keep GUI extension contracts domain-agnostic.
- Gate GUI contributions on required model plugins when applicable.
- Avoid hard-coding biosimulation-specific behavior into generic GUI extension infrastructure.
- Preserve `MainWindow` and controller wiring until extension lifecycle and registry behavior are validated.

## Dynamic plugin architecture direction

Long-term plugin packaging should move toward independently buildable dynamic libraries, but this requires a separate architecture pass.

Before changing dynamic loading or ABI-facing contracts, map:

- factory symbols;
- metadata and versioning;
- symbol visibility;
- ownership across plugin boundaries;
- dependency declarations;
- load and unload lifecycle;
- Debian/PPA packaging implications.

Do not convert static/source-integrated plugins to runtime-loaded packages as part of an unrelated cleanup.

## Validation checklist

For plugin changes, prefer this order:

1. Configure and build the unit-test preset.
2. Run CTest with the unit-test preset.
3. Build the affected application preset if the plugin is GUI, web, or terminal-visible.
4. Load a minimal model that exercises the changed plugin, when possible.
5. Validate save/load round trip if serialization changed.
6. Validate dependency gating if GUI contributions depend on model plugins.

## AI-assistant review rules

- Distinguish model plugins, data-definition plugins, component plugins, and GUI extension plugins.
- Do not invent plugin factories or metadata fields.
- Do not assume methods are unused just because they are empty.
- Do not change plugin names or exported symbols without searching all call sites.
- Keep plugin architecture changes separate from ordinary bug fixes.

## Open follow-up tasks

- Revalidate the component method matrix against current `source/plugins/components`.
- Revalidate the data-definition audit against current `source/plugins/data`.
- Create targeted tests for high-risk data definitions before behavior changes.
- Prepare a separate dynamic-plugin architecture plan covering ABI, metadata, factories, dependencies, and packaging.
- Revalidate GUI extension contracts against current GUI controller code before adding new GUI plugin behavior.
