---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-public-api-change
status: active
tracks: 511
---

# Public Facades and External Integrations Reference

## 1. Scope

Guidance for `SimulatorFacade`, shell command coverage, Python-facing APIs, AI providers, generated code, external processes, worker/HTTP integration and other public boundaries.

Current source code remains authoritative for exact methods and commands. This reference defines coverage and safety expectations, not a frozen API list.

## 2. Facade principle

External clients should depend on deliberate facades/interfaces rather than concrete managers or internal kernel classes.

A facade method must define:

- input/output types and ownership;
- synchronous/asynchronous behavior;
- errors and invalid-state behavior;
- model/simulation preconditions;
- thread and callback assumptions;
- persistence/security implications;
- compatibility expectations.

Do not expose a method merely because an internal manager supports it.

## 3. Shell command coverage

Shell commands should map explicitly to supported facade operations.

Maintain coverage by capability category:

- simulator identity/version;
- plugin discovery;
- model creation/load/save/check;
- component/data-definition inspection and mutation;
- simulation configuration/start/pause/resume/stop;
- replication/results/status;
- tracing/diagnostics;
- exit/help.

For each command, track:

- command syntax;
- facade/backend call;
- output contract;
- invalid input/state behavior;
- scripting stability;
- test coverage.

A generated inventory is preferable to a manually drifting table. Historical coverage matrices are snapshots only.

## 4. Python integration

Python APIs must be narrow, intentional and ownership-safe.

Do not expose directly:

- raw owning pointers;
- manager internals;
- unstable templates/containers;
- callbacks without lifetime wrappers;
- abstract interfaces that Python cannot construct safely;
- mutable internal state without validation.

Prefer:

- value/immutable DTOs;
- opaque handles or dedicated wrappers;
- explicit create/destroy/context-manager behavior;
- copied strings/arrays or clearly scoped views;
- translated structured errors;
- deterministic GIL/thread policy;
- tests for interpreter shutdown and object destruction.

Embedded Python availability is optional and must have explicit enabled/disabled behavior.

## 5. AI provider integration

Separate:

- provider-neutral request/response contracts;
- transport/client implementation;
- credentials;
- model/provider selection;
- redaction/logging;
- retries/timeouts/cancellation;
- tool/function calls;
- application UI/session state.

Secrets must not appear in source, argv, browser-visible environment, generated evidence or logs. Offline deterministic tests must not call external providers.

## 6. Generated code and external processes

For CppForG, PythonForG, compilers, scripts or external simulators:

- validate generated source before execution;
- use controlled temporary/build directories;
- bound runtime, memory and output;
- capture exit status/stdout/stderr safely;
- sanitize paths/arguments;
- define cancellation and cleanup;
- record compiler/runtime/version provenance;
- prohibit secrets in command lines;
- keep generated artifacts out of version control unless intentionally maintained fixtures.

## 7. Worker/HTTP API

The worker public boundary requires explicit:

- bind address and port configuration;
- health/readiness versus protected routes;
- authentication/TLS;
- request/body limits;
- job/session identifiers;
- status/cancellation/expiry;
- file/model upload validation;
- process/filesystem isolation;
- quotas/concurrency;
- audit events and redaction;
- deterministic errors.

A successful loopback health request does not establish a secure deployment profile.

## 8. GUI issue-report relay

A desktop GUI must not contain a write-capable GitHub token.

An approved relay design must provide:

- maintainer-operated server-side credential;
- authenticated/authorized client submissions as required;
- strict schema and size validation;
- rate limiting and abuse controls;
- redaction and audit logging;
- repository/label restrictions;
- explicit failure behavior.

## 9. Plugin ABI boundary

If an external client is a dynamic in-process plugin, use the stable C ABI rules in [`PLUGINS.md`](PLUGINS.md). Do not expose the C++ facade, STL or Qt ABI across package boundaries by convenience.

## 10. Compatibility and versioning

Public boundaries require:

- version/capability discovery;
- additive extension when possible;
- explicit deprecation/migration;
- compatibility fixtures;
- stable installed executable names or documented change;
- no silent semantic change under the same command/API name.

## 11. Validation

- compile-time API/trait checks where useful;
- focused unit tests with local fakes;
- scripting tests for shell output;
- lifetime/interpreter tests for Python;
- transport/error/redaction tests for AI/HTTP;
- process/cleanup tests for external execution;
- package/install lookup tests for public executables/resources.

Current operational coverage is summarized in [`../STATUS.md`](../STATUS.md).
