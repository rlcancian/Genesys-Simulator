# GenESyS standalone worker validation evidence — 2026-07-22

## 1. Scope

This document records the bounded standalone validation of the existing `genesys_worker_app` preset and `genesys-worker` executable.

The validation covers build, startup, one public health request through loopback, exact HTTP/JSON response, request-limited shutdown, and residual-process checks.

It does not approve the current bind policy or establish security readiness for deployment.

## 2. Integrated change

PR #499 added:

```text
.github/workflows/genesys-worker-validation.yml
```

Merge commit in `WorkInProgress`:

```text
8f42f50992b0bd53759018f09e46f48434839bf7
```

Issue #498 was closed as completed after the artifact was reviewed.

## 3. Validated head and workflows

Validated branch head:

```text
65e94f7e1a1c2db5d026fc7542d373d89df98a06
```

### 3.1 Focused worker validation

Run:

```text
29896225187
```

Conclusion: `success`.

Artifact:

```text
genesys-worker-validation
Artifact ID: 8520123900
```

### 3.2 Ordinary CI

Run:

```text
29896225132
```

Conclusion: `success`.

The ordinary `tests-unit` configure/build/CTest path and focused GUI GMDD diagnostics passed.

## 4. Executed workflow

The focused workflow:

1. configured the existing `genesys_worker_app` preset;
2. built with CMake and Ninja;
3. located exactly one executable named `genesys-worker`;
4. selected an ephemeral port;
5. launched the process with `--max-requests 1`;
6. waited for the listener and recorded `ss -ltnp` evidence;
7. sent one request to `http://127.0.0.1:<port>/health`;
8. verified HTTP 200;
9. verified exact JSON `{"ok":true,"status":"up"}`;
10. verified process termination after the configured request limit;
11. recorded remaining listeners and processes;
12. uploaded evidence even on failure.

## 5. Artifact evidence

The artifact recorded:

```text
LISTEN 0 16 0.0.0.0:44559 0.0.0.0:* users:(("genesys-worker",pid=5515,fd=3))
```

The request itself was sent only through loopback:

```text
http://127.0.0.1:44559/health
```

Response headers:

```text
HTTP/1.1 200 OK
Content-Type: application/json
Content-Length: 25
Connection: close
```

Response body:

```json
{"ok":true,"status":"up"}
```

Worker log:

```text
[genesys-worker] listening on port 44559
```

After the bounded request:

- no `genesys-worker` process remained;
- the ephemeral worker listener was absent;
- the workflow exited successfully.

## 6. Confirmed facts

Confirmed in the executed environment:

- the worker preset configures on Ubuntu 24.04;
- the worker target builds with Ninja;
- the executable starts successfully;
- the public health endpoint responds through loopback;
- the health response contract is deterministic for the exercised case;
- `--max-requests 1` terminates the process after one request;
- no worker process remains after shutdown;
- the current server binds to `0.0.0.0`/`INADDR_ANY`, not to loopback only.

## 7. Security interpretation

The selected deployment profile remains Profile B — controlled academic intranet.

The approved policy requires:

- no direct public-Internet exposure by default;
- binding only to an explicitly selected private interface/address;
- authenticated clients;
- TLS for credentials and simulation data;
- cryptographically secure tokens or managed machine identity;
- resource limits, restricted execution, audit logging, and deny-by-default behavior.

The current wildcard listener does not independently satisfy that profile.

Issue #500 tracks the required bind-address contract decision. The recommended option is a safe loopback default with explicit private-address configuration for laboratory deployment, but no production change has been authorized by this evidence document.

Authentication remains a separate open design choice between mutual TLS and short-lived signed tokens over TLS.

## 8. Validated and unvalidated boundaries

### Validated

- preset configuration;
- standalone build;
- executable discovery;
- bounded process startup;
- one loopback `GET /health` request;
- HTTP 200 and exact JSON body;
- request-limit shutdown;
- no residual process.

### Not validated

- authenticated endpoints;
- session or job lifecycle;
- model upload or simulation execution;
- token entropy, expiry, rotation, revocation, or storage;
- TLS/mTLS;
- quotas and concurrency limits;
- process sandboxing;
- filesystem isolation;
- audit completeness;
- private-interface deployment;
- public-Internet exposure;
- package or service-unit deployment.

## 9. Remaining decisions and work

### Decision issue #500

Select the exact bind contract:

- explicit required bind address;
- loopback default with explicit private override;
- deployment-configuration-only address;
- preserve wildcard binding — not recommended.

### Future bounded validation

After the bind policy is selected:

1. add explicit address configuration;
2. test loopback and private-interface binds;
3. test invalid/unavailable address failure;
4. reject wildcard/public binds under the approved profile;
5. verify interaction with authentication-required startup;
6. preserve the bounded health test as a regression guard.

## 10. Interpretation limits

This evidence establishes software behavior only for the recorded runner, head, toolchain, and public health path. It does not establish production security, release readiness, scalability, scientific correctness, or authorization to expose the worker outside a controlled environment.
