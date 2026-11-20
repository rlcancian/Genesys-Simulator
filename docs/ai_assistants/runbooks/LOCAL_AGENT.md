---
document_type: runbook
authority: operational
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# Runbook: Local GenESyS Agent

## 1. Scope

Use this runbook when the agent has a real local checkout and may inspect files, run Git/GitHub CLI, configure/build with CMake and Ninja, execute tests, and use local diagnostics.

The expected primary environment is Ubuntu 24.04.

## 2. Preconditions

Before changing anything:

```bash
git status --short --branch
git remote -v
git branch --show-current
git fetch --prune origin
```

Confirm:

- repository is `rlcancian/Genesys-Simulator`;
- local uncommitted changes are understood and preserved;
- the task base is current `origin/WorkInProgress`;
- no unrelated worktree files will be staged;
- `gh auth status` is valid when PR/issue operations are required.

Do not use `git reset --hard`, `git clean -fdx`, force push, or broad deletion to obtain a clean tree without explicit human authorization.

## 3. Branch setup

Preferred bounded flow:

```bash
git switch WorkInProgress
git pull --ff-only origin WorkInProgress
git switch -c WiPYYYYMMDD/<short-scope>
```

A worktree may be used when another checkout must remain untouched:

```bash
git worktree add ../Genesys-<scope> -b WiPYYYYMMDD/<short-scope> origin/WorkInProgress
```

Record the base commit:

```bash
git rev-parse HEAD
```

## 4. Repository inspection

Use targeted inspection before editing:

```bash
find source -maxdepth 3 -type f | sort
rg -n "<symbol-or-term>" source docs CMakeLists.txt CMakePresets.json .github
cmake --list-presets
```

For ownership work, inspect constructors, destructors, copy/move operations, insertion/adoption paths, callbacks, persistence, and all immediate callers.

For CMake work, inspect target definitions, source selection, compile definitions, link interfaces, consumers, presets, and workflow path filters.

For large files, use local narrow patches rather than replacing the complete file through a remote contents API.

## 5. Canonical build and tests

### Ordinary unit baseline

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel "$(nproc)"
ctest --preset tests-unit --output-on-failure
```

### Kernel-focused baseline

```bash
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel "$(nproc)"
ctest --preset tests-kernel-unit --output-on-failure
```

### Smoke baseline

```bash
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel "$(nproc)"
ctest --preset tests-smoke --output-on-failure
```

### Inventory only

```bash
ctest --preset tests-unit -N
ctest --preset tests-kernel-unit -N
ctest --preset tests-smoke -N
```

Never reuse an old test count as a current count after modifying test registration.

## 6. Application validation

Build the exact affected preset, for example:

```bash
cmake --preset genesys_shell
cmake --build --preset genesys_shell --parallel "$(nproc)"

cmake --preset genesys_worker_app
cmake --build --preset genesys_worker_app --parallel "$(nproc)"

cmake --preset gui-app
cmake --build --preset gui-app --parallel "$(nproc)"
```

Independent GUI presets include:

```text
gui-httpworker
gui-dataanalyser
gui-optimizer
gui-ai-assistant
```

A successful build is not startup validation. A successful startup is not functional workflow or scientific validation.

When using Xvfb, disable TCP listening, verify the window belongs to the application PID, use a bounded timeout, and verify no residual process remains.

## 7. Sanitizers and diagnostics

Use only configurations compatible with the current target and dependencies.

Typical local flags for a focused diagnostic build may include:

```text
-fsanitize=address,undefined
-fno-omit-frame-pointer
```

LeakSanitizer may be included through AddressSanitizer on supported Linux configurations.

Rules:

- start with a focused executable;
- record compiler and sanitizer options;
- capture full exit code and report;
- distinguish reachable allocations, intentional process-lifetime objects, leaks, UAF, double free, and UB;
- do not claim repository-wide cleanliness from one focused run.

Use Valgrind, clang-tidy, cppcheck, or profiling only when they answer the task and the build is compatible.

## 8. Editing and commit discipline

Inspect before staging:

```bash
git diff --stat
git diff -- <intended-paths>
git status --short
```

Stage explicit paths:

```bash
git add <path1> <path2>
```

Use small commits:

```bash
git commit -m "<bounded description>"
```

Do not use `git add -A` in a mixed worktree.

Before push:

```bash
git log --oneline origin/WorkInProgress..HEAD
git diff --check origin/WorkInProgress...HEAD
git diff --stat origin/WorkInProgress...HEAD
```

## 9. Push and PR

```bash
git push -u origin "$(git branch --show-current)"
```

Open a draft PR targeting `WorkInProgress`. Include exact validation commands and results. Do not mark ready until required final-head validation is green.

After merge:

```bash
git fetch --prune origin
git switch WorkInProgress
git pull --ff-only origin WorkInProgress
git branch -d <merged-branch>
```

Delete the remote source branch through GitHub or:

```bash
git push origin --delete <merged-branch>
```

Only delete after confirming the PR merge commit is integrated.

## 10. Local-only high-value tasks

Prefer a local agent for:

- narrow edits in very large files;
- exhaustive repository search;
- `git log --all`, blame, branch comparison, and worktrees;
- exact CMake/Ninja/CTest execution;
- sanitizers, Valgrind, profiling, clang-tidy, and cppcheck;
- package install/uninstall lifecycle;
- application interaction beyond bounded connector workflows;
- generated build graph, link map, symbol, or compile-command analysis.

## 11. Reporting

Record:

- hostname/environment when relevant;
- branch/base/head commits;
- compiler/CMake/Ninja/Qt versions;
- commands executed;
- test inventories and failures;
- sanitizer configuration and exit code;
- files changed;
- unvalidated paths;
- PR and merge information.
