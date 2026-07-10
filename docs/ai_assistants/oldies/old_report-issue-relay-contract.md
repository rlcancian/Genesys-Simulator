# Genesys GUI issue-report relay contract

This document describes the client-to-relay contract used by the
`About -> Report Issue` workflow in `GenesysQtGUI`.

## Objective

The desktop GUI must **not** embed a GitHub token with write access.
Instead, the desktop client sends an HTTPS JSON payload to a relay
owned and operated by the project maintainers. The relay validates,
rate-limits, enriches, and forwards the report to GitHub.

## Client-side configuration

The GUI client reads the following environment variables:

- `GENESYS_REPORT_ISSUE_RELAY_URL` **required**
  - HTTPS endpoint of the relay, for example:
    - `https://issues-relay.example.org/api/v1/report-issue`
- `GENESYS_REPORT_ISSUE_CLIENT_KEY` optional
  - Public client identifier used by the relay for non-secret routing, analytics, or version gating.
- `GENESYS_REPORT_ISSUE_TIMEOUT_MS` optional
  - Request timeout in milliseconds.

## HTTP request

- Method: `POST`
- Content-Type: `application/json`
- Accept: `application/json`

### Request body

```json
{
  "schema_version": 1,
  "report": {
    "repository": "rlcancian/Genesys-Simulator",
    "category_key": "bug",
    "category_label": "Bug",
    "title": "Crash when editing queue property",
    "summary": "The GUI closes after pressing Enter in the property editor.",
    "reproduction_steps": "1. Create Queue\n2. Edit property\n3. Press Enter",
    "expected_behavior": "The property should be applied without crashing.",
    "observed_behavior": "The application exits with code 139.",
    "additional_context": "Observed on Ubuntu 24.04 with Qt6 build.",
    "contact_email": "optional@example.org"
  },
  "client": {
    "application_name": "Genesys-Simulator",
    "application_version": "unknown",
    "qt_version": "6.x",
    "operating_system": "Ubuntu 24.04",
    "current_model_name": "MyModel",
    "ui_surface": "GenesysQtGUI",
    "entrypoint": "About/Report Issue",
    "submitted_at_utc": "2026-04-23T12:00:00.000Z"
  },
  "diagnostics": {
    "current_model_simulang": "optional",
    "console_tail": "optional",
    "simulation_tail": "optional",
    "reports_tail": "optional",
    "process_log_tail": "optional",
    "screenshot_png_base64": "optional"
  }
}
```

## Recommended relay behavior

The relay should:

1. require HTTPS;
2. validate payload shape and length;
3. rate-limit by source and payload fingerprint;
4. map `category_key` to server-side GitHub labels;
5. write the GitHub issue using a server-side credential only;
6. redact or reject oversized screenshots/logs;
7. store an audit trail for abuse analysis.

## Recommended relay response

### Success

```json
{
  "issue": {
    "number": 1234,
    "html_url": "https://github.com/rlcancian/Genesys-Simulator/issues/1234"
  }
}
```

### Failure

```json
{
  "message": "Validation failed: title is required."
}
```
