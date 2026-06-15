# Genesys MCP Connector

Python MCP server that exposes GenESyS simulation capabilities to AI assistants
(Claude Desktop, Claude Code, and any MCP-compatible client).

## Prerequisites

- Python 3.10+
- `genesys_web_app` running (see `source/applications/web/README.md`)

## Install

```bash
cd source/applications/mcp
pip install -e .
```

## Run

```bash
# via entry point
genesys-mcp

# or via module
python -m genesys_mcp
```

The server communicates over stdio (default FastMCP transport).
Set `GENESYS_URL` to point to a non-default web app address:

```bash
GENESYS_URL=http://192.168.1.10:8080 genesys-mcp
```

## Claude Desktop integration

Add to `~/.config/claude/claude_desktop_config.json` (Linux) or
`~/Library/Application Support/Claude/claude_desktop_config.json` (macOS):

```json
{
  "mcpServers": {
    "genesys": {
      "command": "genesys-mcp",
      "env": {
        "GENESYS_URL": "http://localhost:8080"
      }
    }
  }
}
```

If `genesys-mcp` is not on PATH (e.g. installed in a venv), use the full path:

```json
{
  "mcpServers": {
    "genesys": {
      "command": "/path/to/venv/bin/genesys-mcp",
      "env": {
        "GENESYS_URL": "http://localhost:8080"
      }
    }
  }
}
```

## Claude Code integration

```bash
claude mcp add genesys -- genesys-mcp
```

Or with a custom URL:

```bash
claude mcp add genesys -e GENESYS_URL=http://localhost:8080 -- genesys-mcp
```

## Workflow

All tools that interact with the simulator require a session token.
Call `genesys_auth` once per conversation and pass the returned token
to every subsequent tool.

```
1. genesys_auth          → { token: "..." }
2. genesys_new_model     (token)
3. genesys_import_model_language  (token, model_text)
4. genesys_configure_simulation   (token, replications=10, length=480.0)
5. genesys_run_simulation         (token)
6. genesys_create_job    (token) → { jobId: "..." }
7. genesys_run_job       (token, jobId)
8. genesys_job_result    (token, jobId)
```

## Available tools (18)

| Tool | Auth required | Description |
|---|---|---|
| `genesys_health` | No | Check web app reachability |
| `genesys_auth` | No | Create session, returns token |
| `genesys_info` | Yes | Simulator version and licence |
| `genesys_new_model` | Yes | Create empty model |
| `genesys_get_model` | Yes | Describe current model |
| `genesys_save_model` | Yes | Save model to workspace file |
| `genesys_load_model` | Yes | Load model from workspace file |
| `genesys_import_model_language` | Yes | Import model from text |
| `genesys_simulation_status` | Yes | Current simulation state |
| `genesys_configure_simulation` | Yes | Set replication/length/warmup params |
| `genesys_run_simulation` | Yes | Run simulation synchronously |
| `genesys_step_simulation` | Yes | Advance one event step |
| `genesys_worker_info` | No | Worker identity metadata |
| `genesys_worker_capabilities` | No | Worker capability flags |
| `genesys_create_job` | Yes | Create job from current model |
| `genesys_job_status` | Yes | Query job state |
| `genesys_run_job` | Yes | Execute job synchronously |
| `genesys_job_result` | Yes | Retrieve job result summary |
