# Genesys Web Application

First functional implementation of the **Web** application type for GenESyS.

## Current Stage 5 scope
- CMake executable: `genesys_webhook` (enabled with `-DGENESYS_BUILD_WEB_APPLICATION=ON`).
- Layered architecture for HTTP transport, API routing, session/token, and simulator service.
- Stateful session management with one `Simulator` per session and per-session workspace directories.
- Endpoints:
  - `GET /health`
  - `POST /api/v1/auth/session`
  - `GET /api/v1/simulator/info` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/models` (requires `Authorization: Bearer <token>`)
  - `GET /api/v1/models/current` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/models/save` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/models/load` (requires `Authorization: Bearer <token>`)
  - `GET /api/v1/simulation/status` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/simulation/config` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/simulation/run` (requires `Authorization: Bearer <token>`)
  - `POST /api/v1/simulation/step` (requires `Authorization: Bearer <token>`)

## Worker cycle Stage 1 (new cycle)
The Web application is now also being evolved as a **worker API** that can be discovered by a future distributed orchestrator application.

This stage adds two public worker discovery endpoints (no Bearer token required):
- `GET /api/v1/worker/info`
- `GET /api/v1/worker/capabilities`

These routes expose worker identity metadata and current implementation capability flags. They are intentionally limited to discovery only and do not include job submission, polling, or background execution in this stage.

## Worker cycle Stage 2
This stage starts worker model-ingress support with one authenticated and session-scoped endpoint:
- `POST /api/v1/worker/models/import-language` (requires `Authorization: Bearer <token>`)

The request body must be plain text GenESyS model language/specification. The worker imports this text into the session simulator as the current model.

This stage still does **not** include multipart/binary upload, distributed job submission, polling, or background orchestration.

## How to run
```bash
cmake -S . -B build/web-debug -G Ninja -DGENESYS_BUILD_WEB_APPLICATION=ON
cmake --build build/web-debug --target genesys_webhook
./build/web-debug/source/applications/web/genesys_webhook --port 8080
```

Optional parameters:
- `--port <N>`: HTTP port (default 8080).
- `--max-requests <N>`: exit after N requests (useful for scripted smoke tests).

## Security notice
Session creation in Stage 1 is provisional and stateful in memory. Generated tokens are opaque random strings and **not** production-grade authentication.

For Stage 3 model persistence:
- Save/load operations are restricted to each session workspace directory.
- API requests accept only a `filename` basename (no paths).
- Filenames must match `[A-Za-z0-9._-]` and cannot include `..`, `/`, or `\`.

For Stage 4 simulation configuration:
- `POST /api/v1/simulation/config` currently uses a minimal contract parser without external JSON libraries.
- All configuration fields are required in the request body: `numberOfReplications`, `replicationLength`, `warmUpPeriod`, `pauseOnEvent`, `pauseOnReplication`, `initializeStatistics`, `initializeSystem`.

For Stage 5 simulation execution:
- `POST /api/v1/simulation/run` and `POST /api/v1/simulation/step` are synchronous wrappers over `ModelSimulation::start()` and `ModelSimulation::step()`.
- This stage intentionally does not introduce asynchronous/background simulation control endpoints.
