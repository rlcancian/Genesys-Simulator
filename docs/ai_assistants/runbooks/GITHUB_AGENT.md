---
document_type: runbook
authority: operational
owner: project-maintainer
last_reviewed: 2026-07-22
status: active
tracks: 511
---

# Runbook: GitHub-Only GenESyS Agent

## 1. Scope

Use this runbook when the agent can inspect and modify the repository only through GitHub APIs/connectors, pull requests, issues, Actions workflows, and workflow artifacts.

This environment does not provide a local checkout unless a separate local tool explicitly confirms one.

## 2. Capability boundary

A GitHub-only agent may:

- read repository files by branch/commit;
- inspect pull requests, issues, diffs, comments, reviews, and merge state;
- create bounded branches;
- create/update text files through GitHub contents APIs;
- open and manage draft PRs;
- inspect workflow run state;
- download and review workflow artifacts;
- use purpose-built CI workflows to generate executable evidence.

A GitHub-only agent must not claim it locally:

- configured CMake;
- built with Ninja;
- ran CTest;
- launched an executable;
- ran a sanitizer, Valgrind, profiler, or package install;
- inspected uncommitted local files;
- searched branches or Git history beyond connector evidence.

If executable evidence is required, use an existing approved workflow, add a bounded evidence workflow in a dedicated PR, or stop/escalate.

## 3. Startup sequence

1. read repository `/README.md` from current `WorkInProgress`;
2. read `docs/ai_assistants/README.md`;
3. read `../GOVERNANCE.md`, `../STATUS.md`, and the applicable backlog;
4. inspect current open PRs against `WorkInProgress`;
5. inspect related open issues and recent merged PRs;
6. confirm the current base branch content directly;
7. identify whether another branch owns the same backlog task.

Do not use a file fetched from `master` when the task explicitly targets `WorkInProgress`.

## 4. Branch and file operations

Create a bounded branch from current `WorkInProgress`.

Before updating an existing file:

- fetch the file from the exact branch;
- retain its current blob SHA;
- replace it only with complete content derived from that exact version;
- never perform parallel writes to the same path;
- re-fetch after a sequential update if another update is required.

For large files:

- avoid full replacement for a tiny change when a local agent is safer;
- do not reconstruct a file from partial/truncated connector output;
- prefer a new focused test/source file over unsafe editing of a very large historical file when semantics permit;
- otherwise stop and mark the task `local`.

## 5. Evidence through GitHub Actions

An evidence workflow must be bounded and must:

- use the existing project preset when possible;
- record branch/head/merge SHA;
- record toolchain versions;
- capture configure, build, execution, and exit status separately;
- upload diagnostics even when the step fails;
- use timeouts;
- avoid public network exposure;
- avoid real credentials unless a separately approved secure test contract exists;
- preserve ordinary CI.

For GUI startup:

- use a private Xvfb display with TCP disabled;
- locate exactly the intended executable;
- verify a window associated with the application PID;
- verify bounded liveness;
- terminate predictably;
- verify no residual process.

For server/worker startup:

- use an ephemeral port;
- send requests through loopback unless the task explicitly validates private binding;
- record actual listener addresses;
- bound requests and process lifetime;
- verify no residual listener/process;
- do not interpret loopback requests as proof of loopback-only binding.

For sanitizers:

- compile only the focused executable plus dependencies when practical;
- preserve sanitizer stderr and exit code;
- review the artifact, not only the green/red conclusion.

## 6. Artifact review

Before merging a PR that requires artifact review:

1. fetch workflow run and artifact metadata;
2. verify artifact belongs to the final PR head/merge ref;
3. download and inspect relevant files;
4. confirm exit code, executable path, process/window/listener evidence, residual processes, and diagnostics;
5. distinguish harmless environment warnings from task failures;
6. record artifact ID and digest when useful;
7. add a concise PR/issue comment with the conclusion.

A successful workflow conclusion does not replace artifact inspection when the acceptance criteria explicitly require it.

## 7. Red/green workflow

For a defect correction:

1. create a focused test/evidence-only red checkpoint;
2. verify the failure occurs in the intended path;
3. inspect logs/artifacts;
4. apply the smallest production correction;
5. rerun final-head focused and regression workflows;
6. preserve the permanent guard if it is maintainable and scoped.

Do not modify production behavior based only on an assertion failure whose fixture/lifecycle has not been validated.

## 8. Pull-request readiness and merge

Keep the PR draft until:

- final-head workflows are completed;
- required artifacts are reviewed;
- the branch is mergeable against current `WorkInProgress`;
- no open review thread remains;
- scope/non-goals match the final diff;
- status/backlog/changelog updates are included when applicable.

Use the repository-supported merge method. For this repository, merge commits are supported while squash and rebase merge are disabled.

Use expected head SHA when merging to prevent integrating an unreviewed branch update.

After merge:

- verify merge result and SHA;
- close the issue with evidence;
- rely on repository automatic branch deletion when confirmed;
- otherwise report that branch deletion could not be performed with available connector actions.

## 9. Connector-specific caution

GitHub search and connector results may be incomplete, stale, truncated, or substring-based.

Therefore:

- use generated CMake File API or workflow evidence for exact target graphs;
- use direct file reads for current source facts;
- do not infer absence from one code-search result when the claim is critical;
- cross-check PR claims against current branch files;
- do not copy historical branch files wholesale into a diverged current structure;
- use exact paths returned by file/PR listing actions;
- re-fetch PR metadata after base branch advances because mergeability may be recalculated asynchronously.

## 10. Stop conditions

Stop and request local execution or human input when:

- a safe edit requires partial modification of a very large file;
- connector output is truncated and complete content is required;
- exact Git history/blame across branches is necessary;
- the task requires interactive application behavior;
- package lifecycle needs root/local environment control not represented by CI;
- architecture, security, science, release, or product choices are unresolved;
- required workflows cannot safely represent the real target environment.

## 11. Reporting language

Use precise statements such as:

- “confirmed in the current GitHub file”;
- “confirmed by workflow run X and artifact Y”;
- “not locally executed in this environment”;
- “strong indication from connector search; generated graph still required”;
- “historical PR evidence; not confirmed in current `WorkInProgress`”.
