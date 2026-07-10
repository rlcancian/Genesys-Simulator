# GenESyS Docker Packaging

This repository now exposes two public helper scripts in `packaging/docker/`:

- `exec-genesys.sh`
- `develop-genesys.sh`

For the repository-wide branch policy used by these scripts, see
`docs/ai_assistants/branch_workflow.md`.

## User flow

Run from the repository root:

```bash
bash packaging/docker/exec-genesys.sh
```

The script builds or reuses the runtime image and shows a menu with the
available applications:

- `GenESyS Terminal Shell`
- `GenESyS Qt GUI`
- `HTTP Worker GUI`
- `Data Analyser GUI`
- `Optimizer GUI`
- `AI Assistant GUI`

The runtime image defaults to the `master` branch from
`https://github.com/rlcancian/Genesys-Simulator.git`. Advanced users can
override the repository URL and branch with environment variables or CLI flags.
This is intentional: `master` is the public release line, while the active
Docker work in this checkout is being developed on `WiP20261` and will be
promoted through the normal PR flow.

## Development flow

Run from the repository root:

```bash
bash packaging/docker/develop-genesys.sh
```

This opens Qt Creator in a development container with the local checkout mounted
at `/workspace`.

## GUI forwarding

The helper scripts use X11 forwarding when `DISPLAY` is available. They mount
`/tmp/.X11-unix` and pass `XAUTHORITY` when present. Wayland support is kept
conservative and is disabled by default.

## Configuration

Common defaults live in `packaging/docker/config/genesys-docker.conf`.
The current runtime/development Docker presets and branch defaults were
validated against the `WiP20261` branch in this worktree, with the runtime
branch pinned to `master` by default so user-facing execution follows the
public branch policy.

## Current limitations

- GUI startup still depends on the host desktop session being available.
- The runtime image is intentionally larger because it builds the supported
  applications in advance.
- Qt Creator in a container is functional, but the host still controls the
  display/session forwarding.
