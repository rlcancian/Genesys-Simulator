# GUI issue reporting: transport, privacy, and validation

Entry point: DialogUtilityController::onActionAboutReportIssueTriggered().
The DialogReportIssue widget owns the transient form and asynchronous submission. The controller no longer reads the current model, SimulLang editor, console, simulation, reports, crash log or screenshot.

## UI and browser mode

The form contains type, title, and description only. The relay is optional: if no valid HTTPS relay endpoint is configured, the primary action is **Continue on GitHub**. IssueReportBrowserService builds the official https://github.com/rlcancian/Genesys-Simulator/issues/new URL using QUrlQuery with title and body parameters. It includes the selected report type as ordinary body text, not a privileged label query. It does not include automatically collected data.

The encoded URL limit is 1800 bytes; on overflow the body, or if necessary the whole draft, is offered through the clipboard instead of being truncated. Browser handoff is never described as confirmed issue creation. QDesktopServices::openUrl only confirms handoff, not successful loading, authentication, or submission.

## Relay mode

Configuration:

- GENESYS_REPORT_ISSUE_RELAY_URL: optional HTTPS relay URL; absent/invalid means browser mode.
- GENESYS_REPORT_ISSUE_CLIENT_KEY: optional public client identifier, **not authentication**.
- GENESYS_REPORT_ISSUE_TIMEOUT_MS: optional positive integer, default 15,000 ms.

Do not distribute GitHub credentials in the client. Deployment and authorization of a real relay remain out of scope.

IssueReportRelayService::serializePayload preserves the schema version 1 outer JSON structure (report, client, diagnostics). Only category, title, description (report.summary), basic client identity and Qt/application versions are produced by this GUI. The diagnostics member is an empty object. Optional legacy report fields remain available for schema compatibility but are not collected by this form.

Requests are async and use the Qt network manager, reject redirects, and time out. The result is **confirmed** only if the response is HTTP 2xx, valid JSON, and contains a positive issue number and a matching canonical HTTPS GitHub issue URL for this repository. A plain 200, a malformed reply, inconsistent identifiers, and external URLs are failures. After an uncertain timeout, the form remains available and the user is warned to avoid duplicate creation.

## Testing

Build and run the focused test suite from the repository root:

~~~sh
cmake --preset gui-app -DGENESYS_BUILD_GUI_ISSUE_TESTS=ON
cmake --build build/gui-app --target genesys_gui_issue_tests --parallel 2
QT_QPA_PLATFORM=offscreen ctest --test-dir build/gui-app -R '^genesys_gui_issue_tests$' --output-on-failure
cmake --build --preset gui-app --parallel 2
~~~

The tests use QtTest widgets and a fake QNetworkAccessManager; they never create production issues or contact external relay servers. GitHub Actions workflow genesys-gui-report-issue.yml runs this suite and the real main GUI build. An actual relay-to-GitHub end-to-end check requires an independently authorized server and test repository.

Manual impact: this workflow needs incorporation into the LaTeX User and Developer manuals and a regenerated PDF under the documented manual workflow; no PDF regeneration is claimed from GitHub-only code editing.
