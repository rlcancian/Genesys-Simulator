# Genesys Distributed Simulation Layer

Intermediate orchestration layer that runs GenESyS simulations across several machines, using
existing **web worker** instances (`genesys_web_app`) as remote simulation workers. It discovers
workers, partitions the replications, executes them (locally and remotely, with failover) and
aggregates the partial results into a single unified result — as if the whole simulation had run
locally.

This layer is **not** part of the kernel and **not** the web application itself; it is a reusable
library (`genesys_distributed_core`) plus a standalone orchestrator app (`genesys_distributed_app`).
Because the logic lives in a library behind a small facade, it is **reusable by other applications**:
the GenESyS **terminal** app embeds it as a `distribute` shell command (see
[Using the layer from the integrated terminal](#using-the-layer-from-the-integrated-terminal)).

## Architecture

- `WorkerDescriptor` / `WorkerRegistry` — known workers, their capabilities, state and failure history.
- `WorkerDiscoveryService` — validates a static list of endpoints via the worker API.
- `WorkerHttpClient` — minimal outbound HTTP/1.1 client (POSIX sockets, no extra dependency).
- `DistributedScheduler` — partitions N replications into batches (one per target) with distinct seeds.
- `RemoteSimulationExecutor` / `LocalSimulationExecutor` — run a batch on a worker or in-process.
- `DistributedExecutionManager` — runs batches in parallel, with timeout/failure handling and failover.
- `DistributedResultsAggregator` — merges partial results with exact pooled statistics.
- `DistributedSimulationManager` — high-level facade used by applications.

## Build

The layer is gated by a CMake option and is **off by default** (so it never affects other apps):

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON \
    -DGENESYS_BUILD_WEB_APPLICATION=ON
cmake --build build/distributed --target genesys_distributed_app genesys_web_app
```

Or, matching the project's other apps, use the **`distributed-app` preset** — it enables the layer,
the worker web app and the terminal (with the distributed command wired in), and builds all three:

```bash
cmake --preset distributed-app
cmake --build --preset distributed-app -j$(nproc)
```

Binaries:
- orchestrator: `build/distributed/source/applications/distributed/genesys_distributed_app`
- worker:       `build/distributed/source/applications/web/genesys_web_app`
- terminal:     `build/distributed/source/applications/terminal/genesys_terminal_application`

## Running the orchestrator

The binary is **not installed on `PATH`** — run it from its build location. The examples below run
from the project root and use a shell variable for brevity:

```bash
APP=build/distributed/source/applications/distributed/genesys_distributed_app
```

Via command-line arguments (no config file needed):

```bash
$APP --model <file> --replications <N> [--local] \
    [--worker host:port]... [--output <file.json>] \
    [--max-retries <N>] [--base-seed <N>] \
    [--timeout <seconds>] [--discovery-timeout <seconds>]
```

Or via a JSON config. Copy the example and edit it (the `modelFile` path and worker ports must
match your setup); the path is relative to your current directory:

```bash
cp source/applications/distributed/config/workers.example.json workers.json
# edit workers.json, then:
$APP --config workers.json
```

Running `$APP` with no arguments prints the usage. The app prints a human-readable summary to
stdout and, when `--output` (or `outputFile` in the config) is given, writes the aggregated result
as structured JSON.

## Using the layer from the integrated terminal

The same facade is also reachable from the GenESyS **terminal** application through a `distribute`
shell command — demonstrating that the layer is reusable by an existing app, not just the standalone
orchestrator. The command is only compiled when the layer is enabled (it is guarded by
`GENESYS_DISTRIBUTED_ENABLED`, set by the CMake glue when `GENESYS_BUILD_DISTRIBUTED=ON`), so the
default terminal build is unaffected.

Build the terminal with the layer wired in (the `distributed-app` preset does this), then drive it
non-interactively — pass each shell command as a quoted argument and end with `exit`. The examples
use a model that ships with the repo (`models/Smart_HoldSearchRemove.gen`); any multi-line `.gen`
works, including the `model.txt` built in
[Reproducing the distributed speedup](#reproducing-the-distributed-speedup) below.

```bash
APP=build/distributed/source/applications/terminal/genesys_terminal_application
MODEL=models/Smart_HoldSearchRemove.gen

# Local only (no workers needed):
"$APP" "load $MODEL" "distribute --local --replications 200" "exit" < /dev/null

# Distributed across two workers plus local (start the workers first, see below):
"$APP" "load $MODEL" \
    "distribute --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local --replications 3000" \
    "exit" < /dev/null
```

`distribute` accepts the **same flags** as the standalone orchestrator (`--worker`, `--local`,
`--replications`, `--model`, `--max-retries`, `--base-seed`, `--timeout`, `--discovery-timeout`) and
prints the same aggregated summary. When `--model` / `--replications` are omitted it falls back to the
**model already loaded** with `load` and its configured replication count, so the natural flow is
`load … → distribute --worker …`.

Three things matter when invoking the terminal this way:

- **Pass commands as quoted `argv` arguments and finish with `"exit"`.** The shell consumes `argv`
  entries as commands; without a final `exit` it then falls back to reading stdin in raw mode, which
  garbles the calling terminal. (Do **not** name the shell variable `TERM` — that is the reserved
  terminfo variable and setting it to a path breaks your terminal.)
- **Redirect stdin from `/dev/null`.** On start-up the terminal tries to auto-load every plugin and
  interactively offers to install missing system dependencies (libSBML, ngspice, R, Octave). With a
  non-interactive stdin it declines automatically and simply skips those plugins — none are needed by
  ordinary discrete-event models. (The standalone orchestrator and the worker do not prompt: they use
  the silent `autoInsertPlugins()` over the built-in connector.)
- **Timing note.** `time` only measures the orchestrator process; the remote workers are separate
  processes whose CPU it does not count. Numbers vary run-to-run because the local batch and the two
  worker processes contend for the same cores — this is environmental, not a difference between the
  terminal and standalone code paths (both call the identical facade and kernel).

## Reproducing the distributed speedup

The primary use case of this layer is running a **large number of replications** faster by spreading
them across workers. The steps below show a measurable wall-clock reduction.

### 1. Create a model file `model.txt`

The model **must be multi-line** — the `.gen` parser reads one record per line. A single-line model is
parsed as a single `ModelInfo` record (zero components, empty statistics).

```
# Genesys Simulation Model
0   ModelInfo  "DistributedDemo" version="1.0" projectTitle="" description="" analystName=""
0   ModelSimulation "" traceLevel=0 replicationLength=10.000000 numberOfReplications=20000
62  EntityType "Part"
61  Create     "Create_1" entityType="Part" nextId=64 timeBetweenCreations="norm(1.5,0.5)"
64  Delay      "Delay_1" delayExpression="norm(1.0,0.2)" nextId=63
63  Dispose    "Dispose_1" nexts=0
```

This model does real component work (entities flow Create → Delay → Dispose), so it produces real
statistics (`Delay_1.DelayTime`, `Part.TotalTimeInSystem`, `Part.WaitTime`) and counters. Because each
replication now does real work, **far fewer replications** are needed for a measurable run than with a
trivial model (and a worker batch can take several seconds — see the `--timeout` note below).

### 2. Start two workers on free ports

```bash
WEB=build/distributed/source/applications/web/genesys_web_app
pkill -f genesys_web_app           # make sure no stale worker holds the ports
$WEB --port 8101 --max-requests 200 &
$WEB --port 8102 --max-requests 200 &
sleep 2
```

### Demonstrating the required behaviors (Tema 13, section 14)

`demonstrate-requirements.sh` walks through every item the assignment's tests must show
(discovery of multiple workers, remote job submission, replication distribution, aggregation,
timeout/failure handling, failover re-routing, and distributed-vs-local coherence), printing a
PASS/FAIL line for each. It mixes live orchestrator+worker runs with the targeted unit tests.

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON -DGENESYS_BUILD_TESTS=ON
cmake --build build/distributed --target \
    genesys_distributed_app genesys_web_app genesys_kernel_unit_tests
source/applications/distributed/demonstrate-requirements.sh
```

The orchestrator output includes a per-worker report (discovery state, observed latency, failure
count and replications completed per target), so the distribution and failure handling are visible
directly in the summary / JSON.

### Automated benchmark

The helper script runs both versions and prints a filtered comparison (no startup banner). From the
project root:

```bash
source/applications/distributed/run-distributed-demo.sh            # default 20000 replications
source/applications/distributed/run-distributed-demo.sh 50000      # custom replication count
BUILD_DIR=build/distributed WORKER_PORTS="8101 8102 8103" \
    source/applications/distributed/run-distributed-demo.sh        # override build dir / workers
```

It builds nothing (build first, see above), starts/stops the workers itself, and prints `local`,
`distributed` and `speedup`. The manual steps below do the same by hand.

### 3. Measure baseline (local only) vs distributed

```bash
APP=build/distributed/source/applications/distributed/genesys_distributed_app

# Baseline: a single local engine.
time $APP --model model.txt --replications 20000 --local

# Distributed: two remote workers plus the local engine (3 engines in parallel).
time $APP --model model.txt --replications 20000 \
     --worker 127.0.0.1:8101 --worker 127.0.0.1:8102 --local

pkill -f genesys_web_app           # clean up the workers
```

Representative result (4-core machine, model above):

| Execution                         | Wall-clock |
|-----------------------------------|------------|
| Local only (1 engine)             | ~7.5 s     |
| 2 workers + local (3 engines)     | ~3.0 s     |

Both runs complete `20000 / 20000` replications. Increase `--replications` or add more `--worker`
endpoints for a larger, clearer speedup.

### Tips and caveats

- **Two separate timeouts.** Discovery and execution have opposite needs, so they use different
  timeouts:
  - `--discovery-timeout <s>` (default **5s**): connect/response timeout for probing workers, so a
    dead/unreachable worker is detected quickly.
  - `--timeout <s>` (default **300s**): response timeout while a worker runs the job. A batch that
    runs a non-trivial model for many replications can take a while; if it exceeds this, the batch is
    treated as a worker failure (→ failover, or reported `lost`). Raise it for very long jobs.

  The 300s default covers the demo batches out of the box (no `--timeout` needed). Both are also
  settable in the JSON config as `discoveryTimeoutSeconds` / `runTimeoutSeconds`.
- **Use free ports.** A worker that fails to bind is marked unavailable during discovery, and its
  share of the load falls back to the remaining targets (or local) — which hides the speedup.
- The orchestrator runs the initial attempts in parallel; failover reassignment is sequential.
- Failover during execution (a worker dying mid-job) is covered by the unit tests; pointing
  `--worker` at a dead port instead exercises the "ignore unavailable worker" path at discovery.

## Component statistics: how it works and current limits

Imported language models now instantiate their components and produce **real component statistics**
(Cstats and Counters), both locally and on remote workers, and the layer aggregates them correctly.

This relies on registering the built-in component plugins before parsing: the worker
(`genesys_web_app`) and the local executor call `autoInsertPlugins()` when creating their `Simulator`.
The built-in plugin connector maps component type names to compiled-in classes, so this works in the
static build **without loading any `.so` file** (no dynamic plugin system needed).

Two limits remain:

- **Model format must be multi-line** — one record per line, like the `.gen` files under `models/`.
  A model squeezed onto a single line is parsed as a single record (only its `ModelInfo`), yielding
  zero components. (The replication count still works because it comes from `--replications` / the job
  config, not from the parsed model.)
- **Complex `.gen` models still fail to import** (`INVALID_MODEL_SPECIFICATION`) on this branch. This
  is a separate pre-existing limitation of the parser/serializer, unrelated to plugin registration.
  Simple and moderate models run end-to-end with real statistics.

Example (Create→Dispose, multi-line, run on a remote worker) returns non-empty results:
`Create_1.CountNumberOut`, `Dispose_1.CountNumberIn` and the `Part.TotalTimeInSystem` Cstat. The
exactness of the pooled aggregation is additionally proven by `genesys_test_distributed_aggregator`
(pooled moments equal the statistics computed over the combined values).

## Tests

```bash
cmake -S . -B build/distributed -G Ninja \
    -DGENESYS_BUILD_DISTRIBUTED=ON -DGENESYS_BUILD_WEB_APPLICATION=ON -DGENESYS_BUILD_TESTS=ON
cmake --build build/distributed --target genesys_kernel_unit_tests
ctest --test-dir build/distributed -L unit -R distributed --output-on-failure
```
