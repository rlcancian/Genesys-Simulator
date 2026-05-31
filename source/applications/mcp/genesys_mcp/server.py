"""Genesys MCP Server — exposes GenESyS simulation capabilities via MCP."""

from __future__ import annotations

import os
from typing import Any

import httpx
from mcp.server.fastmcp import FastMCP

from .client import GenesysClient, GenesysClientError

# ---------------------------------------------------------------------------
# Server setup
# ---------------------------------------------------------------------------

mcp = FastMCP(
    "Genesys Simulator",
    instructions=(
        "Use these tools to create, configure, and run GenESyS discrete-event "
        "simulation models. Always call genesys_auth first to obtain a session "
        "token before using any other tool."
    ),
)

_DEFAULT_URL = os.environ.get("GENESYS_URL", "http://localhost:8080")

# Each MCP tool call creates a short-lived client.  For workflows that need
# a persistent session token across multiple tool calls, the caller must pass
# the token returned by genesys_auth back into subsequent calls.
#
# Rationale: MCP tools are stateless by design; the session token is the
# caller's responsibility to thread through the conversation.


def _client(base_url: str, token: str | None = None) -> GenesysClient:
    c = GenesysClient(base_url)
    c._token = token  # noqa: SLF001
    return c


def _err(e: GenesysClientError) -> dict[str, str]:
    return {"error": str(e)}


def _conn_err(e: httpx.RequestError) -> dict[str, str]:
    return {"error": f"Connection error: {e}"}


# ---------------------------------------------------------------------------
# Auth / health
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_health(base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Check that the Genesys web application is reachable.

    Returns the health status object from GET /health.
    No authentication required.
    """
    try:
        with _client(base_url) as c:
            return c.health()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_auth(base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Create a new Genesys session and return the bearer token.

    Returns {"token": "<bearer-token>", ...}.
    Pass the token to all subsequent tools as the `token` parameter.
    """
    try:
        with _client(base_url) as c:
            return c.create_session()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Simulator info
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_info(token: str, base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Return simulator version, name, and licence information.

    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.simulator_info()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Model management
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_new_model(token: str, base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Create a new empty simulation model in the current session.

    The new model becomes the session's current model.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.new_model()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_get_model(token: str, base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Return a description of the session's current simulation model.

    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.get_model()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_save_model(
    token: str,
    filename: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Save the current model to a file inside the session workspace.

    `filename` must be a basename (no path separators). Only alphanumeric
    characters, dots, hyphens, and underscores are allowed.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.save_model(filename)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_load_model(
    token: str,
    filename: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Load a model from a file inside the session workspace.

    `filename` must be a basename (no path separators).
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.load_model(filename)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_import_model_language(
    token: str,
    model_text: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Import a simulation model from GenESyS model language text.

    `model_text` is the full plain-text GenESyS model specification.
    The imported model becomes the session's current model.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.import_model_language(model_text)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Simulation control
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_simulation_status(
    token: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Return the current simulation status (state, replication, clock, etc.).

    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.simulation_status()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_configure_simulation(
    token: str,
    number_of_replications: int,
    replication_length: float,
    warm_up_period: float = 0.0,
    pause_on_event: bool = False,
    pause_on_replication: bool = False,
    initialize_statistics: bool = True,
    initialize_system: bool = True,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Configure simulation parameters before running.

    Args:
        token: Bearer token from genesys_auth.
        number_of_replications: How many independent replications to run.
        replication_length: Simulated time length of each replication.
        warm_up_period: Simulated time before statistics start being collected.
        pause_on_event: Pause simulation after each event (step-by-step mode).
        pause_on_replication: Pause between replications.
        initialize_statistics: Reset statistics counters at start of each replication.
        initialize_system: Reset system state at start of each replication.
        base_url: Genesys web app base URL.
    """
    try:
        with _client(base_url, token) as c:
            return c.configure_simulation(
                number_of_replications=number_of_replications,
                replication_length=replication_length,
                warm_up_period=warm_up_period,
                pause_on_event=pause_on_event,
                pause_on_replication=pause_on_replication,
                initialize_statistics=initialize_statistics,
                initialize_system=initialize_system,
            )
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_run_simulation(
    token: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Run the current simulation synchronously and return a summary.

    Configure simulation parameters first with genesys_configure_simulation.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.run_simulation()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_step_simulation(
    token: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Advance the simulation by one event step.

    Useful for step-by-step inspection. Configure with pause_on_event=True first.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.step_simulation()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Worker discovery
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_worker_info(base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Return worker identity metadata (id, version, role).

    No authentication required.
    """
    try:
        with _client(base_url) as c:
            return c.worker_info()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_worker_capabilities(base_url: str = _DEFAULT_URL) -> dict[str, Any]:
    """Return the current worker's capability flags.

    Describes what simulation features this worker instance supports.
    No authentication required.
    """
    try:
        with _client(base_url) as c:
            return c.worker_capabilities()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Worker jobs
# ---------------------------------------------------------------------------


@mcp.tool()
def genesys_create_job(
    token: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Create a worker job from the session's current model.

    The job is created in 'queued' state. A snapshot of the model is
    persisted so later execution is decoupled from live model mutations.
    Returns {"jobId": "<id>", "state": "queued", ...}.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.create_job()
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_job_status(
    token: str,
    job_id: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Return the current state of a worker job (queued/running/finished/failed).

    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.get_job(job_id)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_run_job(
    token: str,
    job_id: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Execute a worker job synchronously.

    The job must be in 'queued' state. This call blocks until the simulation
    finishes. Poll genesys_job_status or call genesys_job_result afterwards.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.run_job(job_id)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


@mcp.tool()
def genesys_job_result(
    token: str,
    job_id: str,
    base_url: str = _DEFAULT_URL,
) -> dict[str, Any]:
    """Retrieve the simulation result summary for a finished or failed job.

    Only available after the job reaches a terminal state.
    Requires a valid session token from genesys_auth.
    """
    try:
        with _client(base_url, token) as c:
            return c.get_job_result(job_id)
    except GenesysClientError as e:
        return _err(e)
    except httpx.RequestError as e:
        return _conn_err(e)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def main() -> None:
    mcp.run()


if __name__ == "__main__":
    main()
