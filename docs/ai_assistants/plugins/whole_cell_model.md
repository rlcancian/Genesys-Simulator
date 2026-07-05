# Whole-Cell Model Plugin Guidance

## Scope

This guide covers whole-cell modeling plugins.

Observed source areas:

- `source/plugins/components/WholeCellModeling/`
- `source/plugins/data/WholeCellModeling/`

Known sampled files include whole-cell state/data classes and components for stochastic reactions, cell growth, fate decisions, division events, resource allocation, transcription, translation, metabolic submodels, and compartment exchange.

## Guidance

- Treat whole-cell modeling as a domain-specific layer, not as a generic plugin bucket.
- Preserve the distinction between biochemical infrastructure and whole-cell modeling orchestration.
- Validate mathematical and biological semantics before changing component behavior.
- Be careful with GLPK/FBA-related behavior and optional dependency handling.
- Keep whole-cell model state serialization explicit and testable.

## Open follow-up

- Inventory all `WholeCellModeling` components and data definitions.
- Identify which classes require GLPK-backed validation and which have built-in fallback behavior.
- Consolidate whole-cell historical planning documents into a stable whole-cell/SBML guide.
