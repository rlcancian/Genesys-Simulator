# GenESyS Data Analyser GUI validation evidence — 2026-07-22

## 1. Scope

This document records bounded standalone configure/build/startup evidence for the independent Qt6 Data Analyser application.

It validates only:

- the existing `gui-dataanalyser` CMake preset;
- Ninja build completion;
- discovery of exactly one `genesys-dataanalyser-gui` executable;
- Qt6/XCB startup under a private Xvfb display;
- creation of an X11 window associated with the application PID;
- process stability during a bounded startup interval;
- controlled SIGTERM teardown with no residual application process.

It does not establish Data Analyser feature maturity or scientific correctness.

## 2. Integrated pull request

PR #503 added:

```text
.github/workflows/genesys-dataanalyser-gui-validation.yml
```

It was merged into `WorkInProgress` as:

```text
b3c7f462666c955d2c6d6429c1d5636290ef25f1
```

The final validated branch head was:

```text
0a6885298f21f997d23ec6a78ae43f134597886c
```

The PR merge ref executed as:

```text
f8b72f02801026804b6766820b4e21aa9bb75db8
```

## 3. Focused workflow evidence

Workflow:

```text
GenESyS Data Analyser GUI Validation
```

Final run:

```text
29911036076
```

Artifact:

```text
genesys-dataanalyser-gui-validation
ID: 8525948784
```

All workflow steps passed:

1. checkout;
2. Ubuntu 24.04 Qt6/X11 dependency installation;
3. toolchain capture;
4. `gui-dataanalyser` preset configuration;
5. Ninja build;
6. exact executable discovery;
7. Xvfb startup with TCP disabled;
8. Qt6/XCB application startup;
9. X11 window discovery;
10. bounded process-liveness check;
11. controlled termination;
12. diagnostic and artifact publication.

## 4. Runtime evidence

The artifact records:

- executable:
  `build/gui-dataanalyser/source/applications/gui/dataanalyser/genesys-dataanalyser-gui`;
- X11 window ID: `2097158`;
- process state during the bounded interval: `Sl`;
- process remained alive for at least the required startup interval;
- application exit code after SIGTERM: `143`;
- residual application-process list: empty.

The application log shows normal GenESyS startup, academic-license information, activation-code absence, and configured execution limits. No startup crash was recorded.

## 5. Ordinary validation

Ordinary CI run:

```text
29911036020
```

Passed:

- `tests-unit` configure;
- `tests-unit` build;
- CTest;
- focused GUI GMDD diagnostics.

## 6. Interpretation

Confirmed:

- the standalone Data Analyser preset is operational on the recorded Ubuntu 24.04/Qt6 toolchain;
- the application links and starts its Qt event loop;
- the process creates a real X11 window;
- process teardown through SIGTERM is bounded and leaves no residual application process.

Not confirmed:

- menu/action correctness;
- user interaction;
- data import;
- distribution fitting;
- statistical inference;
- chart generation;
- export;
- persistence;
- visual correctness;
- numerical/statistical correctness;
- package/install layout;
- graceful application-owned close semantics.

Exit code `143` demonstrates external controlled process termination, not a user-driven Qt close workflow.

## 7. Maturity boundary

This evidence raises the standalone application from preset-only inventory to partial startup validation.

It does not justify Level 3 maturity for the Data Analyser. Level 3 still requires reference-backed functional workflows, error handling, data fixtures, numerical/statistical oracles, and reproducible end-to-end tests.

## 8. Next bounded work

Continue independent Qt6 application startup validation using the same constraints:

1. one existing preset per PR;
2. no application behavior changes;
3. private Xvfb display;
4. exact executable discovery;
5. PID-associated window evidence;
6. bounded startup and teardown;
7. ordinary CI preservation;
8. explicit separation from scientific/functional correctness.

Issue #504 tracks the next candidate: the standalone Optimizer GUI.
