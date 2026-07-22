# GenESyS standalone Optimizer GUI validation evidence — 2026-07-22

## 1. Scope

This document records bounded configure, build, Qt6/XCB startup, X11-window, process-liveness, and external-teardown evidence for the standalone Optimizer GUI.

It does not establish that the optimizer algorithms are implemented, numerically correct, convergent, scientifically validated, or mature at Level 3.

## 2. Integrated change

PR #507 added the focused workflow:

```text
.github/workflows/genesys-optimizer-gui-validation.yml
```

and corrected the trigger filters of both the Optimizer and Data Analyser GUI workflows to include:

```text
source/applications/gui/CMakeLists.txt
```

The umbrella CMake file owns the GUI feature options and `add_subdirectory(...)` routing, so changes limited to it must trigger the relevant standalone validations.

PR #507 was merged into `WorkInProgress` as:

```text
408c62dcd428de38708df4eac11cad287fb84f13
```

Validated branch head:

```text
d1531642aeb5f808fc0f11d43db3542eb49f814c
```

## 3. Executed workflows

| Workflow | Run | Result |
|---|---:|---|
| GenESyS CI | `29912799689` | passed |
| GenESyS Optimizer GUI | `29912799769` | passed |
| GenESyS Data Analyser GUI | `29912799683` | passed |

The Data Analyser workflow executed because its workflow file was changed to close the umbrella-CMake trigger gap.

## 4. Reviewed Optimizer artifact

Artifact:

```text
genesys-optimizer-gui-validation
```

Artifact ID:

```text
8526654357
```

The artifact confirmed:

- the `gui-optimizer` preset configured and built successfully;
- exactly one executable named `genesys-optimizer-gui` was found;
- the executable path was `build/gui-optimizer/source/applications/gui/optimizer/genesys-optimizer-gui`;
- a private Xvfb display was started with TCP disabled;
- Qt used the XCB platform;
- application PID `5950` remained alive through the bounded startup interval;
- X11 window ID `2097158` was associated with that PID;
- the process was externally terminated with SIGTERM;
- the recorded exit code was `143`;
- no residual `genesys-optimizer-gui` process remained.

The application log recorded the normal GenESyS startup banner, version/licence information, activation-code state, and academic-mode limits without a startup crash.

## 5. What is validated

Confirmed for the recorded head and Ubuntu 24.04 runner:

- CMake preset `gui-optimizer`;
- Ninja build of `genesys_optimizer_gui_application`;
- creation of the `genesys-optimizer-gui` executable;
- Qt6/XCB event-loop startup;
- creation of at least one application-owned X11 window;
- bounded process stability;
- deterministic external teardown;
- absence of residual application processes;
- ordinary unit and focused GUI GMDD regression checks.

## 6. What is not validated

Not established by this workflow:

- loading a simulation model;
- selecting controls or responses;
- defining objectives or constraints;
- execution of any optimization algorithm;
- ranking, feasibility, convergence, stopping criteria, or result persistence;
- multiobjective methods, hypervolume, PISA, HypE, SPEA2, IBEA, ZDT, or DTLZ behavior;
- numerical or statistical correctness;
- Level 3/Beta maturity;
- installation or package lifecycle;
- visual correctness or accessibility;
- clean in-application close handling, because the process is terminated externally.

## 7. Interpretation

The standalone Optimizer GUI is now partially validated at the application startup/lifecycle layer. The backend remains a scaffold and must not be described as functionally complete.

The next optimizer work must remain separate from GUI startup validation and must follow the future multiobjective research plan, reference algorithms, deterministic benchmarks, and Level 3 acceptance criteria.

## 8. Remaining independent GUI coverage

Still requiring bounded standalone validation:

- AI Assistant GUI;
- HTTP Worker GUI;
- broader main GUI startup and model-interaction workflow.

These validations must not implicitly change plugin architecture, Qt5 fallback policy, worker security, provider credentials, or optimizer algorithms.
