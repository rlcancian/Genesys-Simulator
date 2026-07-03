# Python Integration Guidance

## Purpose

This document is the stable AI-assistant reference for Python-facing integration in GenESyS.

Historical notes from `oldies/` must be checked against current source files before being treated as current implementation facts.

## Scope

Primary conceptual areas:

- PythonForG component flow.
- Python-facing simulator facade coverage.
- external integration plugins that expose or execute Python-facing behavior.
- ownership and lifetime boundaries between C++ and Python wrappers.

Historical source document:

- `old_PythonForG_SimulatorFacade_coverage.md`

## Current direction from historical notes

The historical PythonForG coverage notes indicate that scripts receive a `simulator` object directly and that the component flow follows an analogy with CppForG using initialization and dispatch hooks.

The exposed surface focuses on controlled public methods of `SimulatorFacade` and wrapper classes. APIs that require callbacks, raw opaque pointers, templates, or open abstract interfaces were explicitly excluded in that phase.

## Integration policy

Python integration must not expose unstable C++ internals casually.

Guidance:

- Keep Python-facing APIs narrow and intentional.
- Preserve simulator/model/component lifetime boundaries.
- Do not expose plugin-manager or parser-manager internals without a dedicated wrapper policy.
- Avoid passing ownership ambiguously across the Python/C++ boundary.
- Treat exceptions, error reporting, and trace behavior as part of the public scripting contract.

## Relationship with external integration plugins

Python integration overlaps with external integration, but it deserves its own guide because language bindings and object lifetime are distinct concerns.

Use `plugins/external_integration.md` for plugin-domain concerns and this file for Python-facing API and wrapper concerns.

## Validation checklist

For Python integration changes, prefer this order:

1. Run unit-test validation.
2. Validate C++ build of the affected integration code.
3. Run minimal PythonForG script examples if available.
4. Validate error reporting for invalid scripts or invalid model operations.
5. Validate that object wrappers do not outlive their backing C++ objects.

## Open follow-up tasks

- Revalidate current PythonForG source files and bindings.
- Decide whether Python API coverage should be documented as a generated or manually maintained matrix.
- Define ownership policy for wrapper classes.
- Decide whether PluginManager and ParserManager should remain excluded or receive dedicated wrapper layers.
- Add minimal scripting regression examples if the test framework supports them.
