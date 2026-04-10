# Genesys Web Application

First functional implementation of the **Web** application type for GenESyS.

## Current Stage 3 scope
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
