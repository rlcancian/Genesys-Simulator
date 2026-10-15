---
document_type: technical-reference
authority: informative
owner: project-maintainer
status: planned
last_reviewed: 2026-10-15
---

# AOP-lite with Explicit Aspect Pipeline

## 1. Status and authority

This document records a future implementation plan. It does not authorize
implementation by itself. The architectural decision is indexed by
`HUM-ARCH-003` in [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md). A bounded
executable task must be created in `BACKLOG_AUTONOMOUS.md` before autonomous
implementation begins.

The selected direction is an explicit AOP-lite design for cross-cutting
concerns in C++23, rather than an immediate source weaver or compiler fork.

## 2. Objective

Provide reusable interception points for concerns such as:

- structured logging;
- timing and metrics;
- cache lookup and population;
- audit/context propagation;
- controlled error observation.

The first implementation must preserve existing kernel, parser, plugin, GUI,
persistence and scientific semantics.

## 3. Proposed design

Add a small internal `AspectPipeline` abstraction with:

- an `AspectContext` carrying operation name, object/model identity,
  correlation data and `std::source_location` where useful;
- ordered `before`, `around`, `after` and error hooks;
- a callable-based invocation boundary compatible with C++23;
- explicit registration/configuration of enabled aspects;
- no global mutable interception state by default.

Initial usage should wrap stable service boundaries, for example model
loading, persistence, plugin operations, simulation execution and selected
Worker operations. It should not instrument every method automatically.

## 4. Candidate integrations

- Qt logging categories for GUI/application diagnostics;
- the project's existing logging conventions for kernel diagnostics;
- a small metrics interface before introducing an external telemetry system;
- explicit cache policies with stable keys, invalidation and ownership rules.

External dependencies are not selected by this plan. Any dependency must be
reviewed for build, license, packaging and deployment impact.

## 5. Phased execution plan

1. **Contract:** define `AspectContext`, invocation result/error semantics and
   ordering guarantees.
2. **Minimal pipeline:** implement one synchronous pipeline with no caching and
   focused unit tests.
3. **Logging/timing:** instrument one kernel boundary and one application
   boundary; verify disabled-aspect overhead and diagnostic context.
4. **Metrics:** add bounded counters/timers without changing behavior.
5. **Cache:** add one explicitly approved cache use case with invalidation and
   negative tests for stale data.
6. **Adoption:** expand only to additional boundaries justified by evidence.

Each phase requires a separate bounded task and focused regression evidence.

## 6. Non-goals

- no AspectC++ integration in the first phase;
- no Clang source-to-source rewriting;
- no automatic weaving of plugins or Qt classes;
- no ABI or plugin-interface change;
- no replacement of the existing logging or persistence contracts without a
  separate decision;
- no cache for scientific/model state without an explicit invalidation design.

## 7. Required validation

Future implementation tasks must cover:

- ordering and exception behavior of aspects;
- disabled-pipeline behavior;
- reentrancy and thread-safety assumptions;
- lifetime and ownership of context data;
- logging redaction and absence of secret leakage;
- cache hit, miss, invalidation and stale-value behavior;
- kernel, GUI, Worker and plugin regression tests for each adopted boundary.

The first implementation must use the existing CMake/Ninja presets and must
not claim repository-wide coverage from a focused test alone.

## 8. Stop gates

Escalate before proceeding if implementation requires:

- transparent interception of arbitrary methods;
- compiler/source weaving;
- a new public plugin ABI;
- cross-thread or distributed context propagation;
- caching scientific state whose invalidation cannot be proved;
- a material change to security, persistence or public application behavior.
