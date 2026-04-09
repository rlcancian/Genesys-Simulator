# Genesys Web Application

First functional implementation of the **Web** application type for GenESyS.

## Current Stage 1 scope
- CMake executable: `genesys_webhook` (enabled with `-DGENESYS_BUILD_WEB_APPLICATION=ON`).
- Layered architecture for HTTP transport, API routing, session/token, and simulator service.
- Endpoints:
  - `GET /health`
  - `POST /api/v1/auth/session`
  - `GET /api/v1/simulator/info` (requires `Authorization: Bearer <token>`)

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
