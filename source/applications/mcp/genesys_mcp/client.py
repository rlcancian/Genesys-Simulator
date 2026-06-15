"""HTTP client wrapping the Genesys Web Application REST API."""

from __future__ import annotations

import httpx
from typing import Any


class GenesysClientError(Exception):
    """Raised when the Genesys REST API returns an error."""
    def __init__(self, status_code: int, message: str) -> None:
        self.status_code = status_code
        super().__init__(f"HTTP {status_code}: {message}")


class GenesysClient:
    """Stateful HTTP client for one Genesys web-app session."""

    def __init__(self, base_url: str = "http://localhost:8080", timeout: float = 30.0) -> None:
        self._base_url = base_url.rstrip("/")
        self._token: str | None = None
        self._http = httpx.Client(timeout=timeout)

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    def _headers(self) -> dict[str, str]:
        h = {"Content-Type": "application/json"}
        if self._token:
            h["Authorization"] = f"Bearer {self._token}"
        return h

    def _get(self, path: str) -> Any:
        r = self._http.get(f"{self._base_url}{path}", headers=self._headers())
        if not r.is_success:
            raise GenesysClientError(r.status_code, r.text)
        return r.json()

    def _post(self, path: str, body: Any = None) -> Any:
        r = self._http.post(
            f"{self._base_url}{path}",
            headers=self._headers(),
            json=body if body is not None else {},
        )
        if not r.is_success:
            raise GenesysClientError(r.status_code, r.text)
        return r.json()

    def _post_text(self, path: str, text: str) -> Any:
        h = self._headers()
        h["Content-Type"] = "text/plain"
        r = self._http.post(f"{self._base_url}{path}", headers=h, content=text.encode())
        if not r.is_success:
            raise GenesysClientError(r.status_code, r.text)
        return r.json()

    # ------------------------------------------------------------------
    # Auth / health
    # ------------------------------------------------------------------

    def health(self) -> dict[str, Any]:
        return self._get("/health")

    def create_session(self) -> dict[str, Any]:
        result = self._post("/api/v1/auth/session")
        self._token = result.get("token")
        return result

    # ------------------------------------------------------------------
    # Simulator info
    # ------------------------------------------------------------------

    def simulator_info(self) -> dict[str, Any]:
        return self._get("/api/v1/simulator/info")

    # ------------------------------------------------------------------
    # Model management
    # ------------------------------------------------------------------

    def new_model(self) -> dict[str, Any]:
        return self._post("/api/v1/models")

    def get_model(self) -> dict[str, Any]:
        return self._get("/api/v1/models/current")

    def save_model(self, filename: str) -> dict[str, Any]:
        return self._post("/api/v1/models/save", {"filename": filename})

    def load_model(self, filename: str) -> dict[str, Any]:
        return self._post("/api/v1/models/load", {"filename": filename})

    # ------------------------------------------------------------------
    # Simulation
    # ------------------------------------------------------------------

    def simulation_status(self) -> dict[str, Any]:
        return self._get("/api/v1/simulation/status")

    def configure_simulation(
        self,
        number_of_replications: int,
        replication_length: float,
        warm_up_period: float,
        pause_on_event: bool = False,
        pause_on_replication: bool = False,
        initialize_statistics: bool = True,
        initialize_system: bool = True,
    ) -> dict[str, Any]:
        return self._post("/api/v1/simulation/config", {
            "numberOfReplications": number_of_replications,
            "replicationLength": replication_length,
            "warmUpPeriod": warm_up_period,
            "pauseOnEvent": pause_on_event,
            "pauseOnReplication": pause_on_replication,
            "initializeStatistics": initialize_statistics,
            "initializeSystem": initialize_system,
        })

    def run_simulation(self) -> dict[str, Any]:
        return self._post("/api/v1/simulation/run")

    def step_simulation(self) -> dict[str, Any]:
        return self._post("/api/v1/simulation/step")

    # ------------------------------------------------------------------
    # Worker discovery
    # ------------------------------------------------------------------

    def worker_info(self) -> dict[str, Any]:
        return self._get("/api/v1/worker/info")

    def worker_capabilities(self) -> dict[str, Any]:
        return self._get("/api/v1/worker/capabilities")

    # ------------------------------------------------------------------
    # Worker — model ingress
    # ------------------------------------------------------------------

    def import_model_language(self, model_text: str) -> dict[str, Any]:
        return self._post_text("/api/v1/worker/models/import-language", model_text)

    # ------------------------------------------------------------------
    # Worker — jobs
    # ------------------------------------------------------------------

    def create_job(self) -> dict[str, Any]:
        return self._post("/api/v1/worker/jobs")

    def get_job(self, job_id: str) -> dict[str, Any]:
        return self._get(f"/api/v1/worker/jobs/{job_id}")

    def run_job(self, job_id: str) -> dict[str, Any]:
        return self._post(f"/api/v1/worker/jobs/{job_id}/run")

    def get_job_result(self, job_id: str) -> dict[str, Any]:
        return self._get(f"/api/v1/worker/jobs/{job_id}/result")

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------

    def close(self) -> None:
        self._http.close()

    def __enter__(self) -> GenesysClient:
        return self

    def __exit__(self, *_: object) -> None:
        self.close()
